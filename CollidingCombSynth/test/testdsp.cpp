#include "../../lib/json.hpp"
#include "../../test/testutil.hpp"
#include "../source/dsp/dspcore.hpp"

#include <chrono>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

// CMake provides this macro, but just in case.
#ifndef UHHYOU_PLUGIN_NAME
#define UHHYOU_PLUGIN_NAME "CollidingCombSynth"
#endif

#define OUT_DIR_PATH "snd/" UHHYOU_PLUGIN_NAME
#define REFERENCE_DIR_PATH OUT_DIR_PATH "/reference/"
#define OUT_DIR_FIRST_PATH OUT_DIR_PATH "/run1_init/"
#define OUT_DIR_SECOND_PATH OUT_DIR_PATH "/run2_reset/"

std::mutex mtx;

bool checkInstrset()
{
  auto iset = instrset_detect();

#ifdef __linux__
  if (iset >= 10) {
    std::cout << "AVX512 is selected.\n";
    return true;
  } else
#endif
    if (iset >= 8) {
    std::cout << "AVX2 is selected.\n";
    return true;
  } else if (iset >= 7) {
    std::cout << "AVX is selected.\n";
    return true;
  }
  std::cerr << "\nError: Instruction set AVX or later not supported on this computer";
  return false;
}

std::unique_ptr<DSPInterface> setupDSP()
{
  auto iset = instrset_detect();

#ifdef __linux__
  if (iset >= 10) {
    return std::make_unique<DSPCore_AVX512>();
  } else
#endif
    if (iset >= 8) {
    return std::make_unique<DSPCore_AVX2>();
  } else if (iset >= 7) {
    return std::make_unique<DSPCore_AVX>();
  }
  return nullptr;
}

inline void render(
  const size_t nFrame,
  Sequencer &sequencer,
  std::vector<std::vector<float>> &wav,
  std::unique_ptr<DSPInterface> &dsp)
{
  sequencer.rewind();

  size_t currentFrame = 0;
  size_t nextFrame = 0;
  NoteEvent note;
  while (true) {
    do {
      bool success = sequencer.process(currentFrame, note);
      if (success) {
        switch (note.type) {
          case NoteEventType::noteOn:
            dsp->noteOn(note.id, note.pitch, note.tuning, note.velocity);
            break;
          case NoteEventType::noteOff:
            dsp->noteOff(note.id);
            break;
        }
      }
      nextFrame = sequencer.nextFrame();
    } while (nextFrame == currentFrame);
    if (nextFrame >= nFrame) break;

    dsp->process(
      nextFrame - currentFrame, wav[0].data() + currentFrame,
      wav[1].data() + currentFrame);
    currentFrame = nextFrame;
  }
  dsp->process(
    nFrame - currentFrame, wav[0].data() + currentFrame, wav[1].data() + currentFrame);
}

SoundFile readReferenceWave(std::string path)
{
  std::unique_lock<std::mutex> lk{mtx};
  SoundFile snd(path);
  return snd;
}

SndFileResult
writeWaveWithLock(std::string path, std::vector<std::vector<float>> &wav, int sampleRate)
{
  std::unique_lock<std::mutex> lk{mtx};
  return writeWave(path, wav, sampleRate);
}

template<typename T> bool almostEqual(T a, T b)
{
  auto diff = std::fabs(a - b);
  return diff
    <= 8 * std::numeric_limits<T>::epsilon() * std::max(std::fabs(a), std::fabs(b))
    || diff < std::numeric_limits<T>::min();
}

void testAlmostEqual(
  const std::string &name,
  std::stringstream &error_stream,
  std::vector<std::vector<float>> &wav,
  SoundFile &ref)
{
  if (!ref.isReady()) return;

  for (size_t ch = 0; ch < wav.size(); ++ch) {
    for (size_t fr = 0; fr < wav[ch].size(); ++fr) {
      if (!std::isfinite(wav[ch][fr])) {
        std::unique_lock<std::mutex> lk{mtx};
        error_stream << "Error " << name << ": Non-finite value " << wav[ch][fr]
                     << " at channel " << ch << ", frame " << fr << "\n";
        return;
      }

      if (!almostEqual(wav[ch][fr], ref.data_[ch][fr])) {
        std::unique_lock<std::mutex> lk{mtx};
        error_stream << "Error " << name << ": actual " << wav[ch][fr] << " and expected "
                     << ref.data_[ch][fr] << " are not almost equal at channel " << ch
                     << ", frame " << fr << "\n";
        return;
      }
    }
  }
}

void testSequence(std::shared_ptr<PresetQueue> queue)
{
  constexpr float sampleRate = 48000;
  constexpr size_t nFrame = size_t(5 * sampleRate);
  constexpr float tempo = 120.0f;

  std::vector<std::vector<float>> wav(2);
  for (auto &channel : wav) channel.resize(nFrame);

  Sequencer sequencer;
  sequencer.setupSequence(sampleRate, tempo);

  auto iter = queue->next();
  while (iter != queue->end()) {
    const auto &preset = iter.value();

    auto dsp = setupDSP();
    if (!dsp) {
      std::unique_lock<std::mutex> lk{mtx};
      std::cerr << "Error: AVX or later instruction set is not supported.\n";
      return;
    }
    dsp->setup(sampleRate);

    size_t index = 0;
    for (const auto &parameter : preset["parameter"]) {
      if (parameter["type"] == "I")
        dsp->param.value[index]->setFromInt(parameter["value"]);
      else if (parameter["type"] == "d")
        dsp->param.value[index]->setFromNormalized(parameter["value"]);
      ++index;
    }
    dsp->setParameters(tempo);
    dsp->reset();

    std::string filename = preset["name"].get<std::string>() + ".wav";
    std::stringstream error_stream;
    SoundFile ref = readReferenceWave(std::string(REFERENCE_DIR_PATH) + filename);

    render(nFrame, sequencer, wav, dsp);
    testAlmostEqual(filename + " init", error_stream, wav, ref);
    writeWaveWithLock(std::string(OUT_DIR_FIRST_PATH) + filename, wav, int(sampleRate));

    dsp->reset();

    render(nFrame, sequencer, wav, dsp);
    testAlmostEqual(filename + " reset", error_stream, wav, ref);
    writeWaveWithLock(std::string(OUT_DIR_SECOND_PATH) + filename, wav, int(sampleRate));

    // Post processing.
    std::string error_str = error_stream.str();
    {
      std::unique_lock<std::mutex> lk{mtx};
      if (error_str.size() > 0) std::cerr << error_str;
    }

    for (auto &channel : wav) std::fill(channel.begin(), channel.end(), 0.0f);

    iter = queue->next();
  }
}

int main()
{
  auto isAvailable = checkInstrset();
  if (!isAvailable) return EXIT_FAILURE;

  fs::create_directories(OUT_DIR_FIRST_PATH);
  fs::create_directories(OUT_DIR_SECOND_PATH);

  auto queue = std::make_shared<PresetQueue>(UHHYOU_PLUGIN_NAME);

  const auto n_cpu = std::thread::hardware_concurrency();
  std::vector<std::thread> threads;

  for (size_t i = 0; i < n_cpu; ++i) {
    std::thread th(testSequence, queue);
    threads.push_back(std::move(th));
  }

  for (auto &th : threads) th.join();

  return 0;
}
