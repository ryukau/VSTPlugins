// (c) 2021 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#include "testutil.hpp"

enum class NoteEventType { noteOn, noteOff };

struct NoteEvent {
  int32_t id;
  int16_t pitch;
  float tuning;
  float velocity;
  size_t frame;
  NoteEventType type;
};

struct Sequencer {
  size_t eventIndex_ = 0;
  std::vector<NoteEvent> events_;

  void rewind() { eventIndex_ = 0; }

  void addNote(
    float sampleRate,
    float noteOnTime,
    float noteOffTime,
    int32_t id,
    int16_t pitch,
    float tuning,
    float velocity)
  {
    NoteEvent noteOn;
    noteOn.id = id;
    noteOn.pitch = pitch;
    noteOn.tuning = tuning;
    noteOn.velocity = velocity;
    noteOn.frame = size_t(sampleRate * noteOnTime);
    noteOn.type = NoteEventType::noteOn;
    events_.push_back(noteOn);

    NoteEvent noteOff = noteOn;
    noteOff.frame = size_t(sampleRate * noteOffTime);
    noteOff.type = NoteEventType::noteOff;
    events_.push_back(noteOff);
  }

  void sort()
  {
    std::sort(events_.begin(), events_.end(), [](const auto &lhs, const auto &rhs) {
      return lhs.frame < rhs.frame;
    });
  }

  void setupSequence(float sampleRate, float tempo)
  {
    if (tempo <= 0.0f) {
      std::cerr << "Error at Sequencer.setupSequence: Negative tempo.\n";
      return;
    }

    float secondsPerNote = 60.0f / tempo;
    std::vector<float> time(8);
    for (size_t i = 0; i < time.size(); ++i) {
      time[i] = float(i) * secondsPerNote;
    }

    addNote(sampleRate, time[0], time[0] + 3.0f * secondsPerNote, 0, 40, 0.0f, 0.5f);
    addNote(sampleRate, time[1], time[1] + 2.0f * secondsPerNote, 1, 47, 0.3f, 0.75f);
    addNote(sampleRate, time[2], time[2] + 0.5f * secondsPerNote, 2, 54, 0.6f, 1.0f);

    addNote(sampleRate, time[4], time[4] + 3.0f * secondsPerNote, 3, 43, 0.6f, 1.0f);
    addNote(sampleRate, time[5], time[5] + 2.0f * secondsPerNote, 4, 50, 0.3f, 0.75f);
    addNote(sampleRate, time[6], time[6] + 0.5f * secondsPerNote, 5, 57, 0.0f, 0.5f);

    sort();
  }

  size_t nextFrame()
  {
    if (eventIndex_ >= events_.size()) return UINT_MAX;
    return events_[eventIndex_].frame;
  }

  bool process(size_t frame, NoteEvent &note)
  {
    if (eventIndex_ >= events_.size()) return false;
    if (frame != events_[eventIndex_].frame) return false;
    note = events_[eventIndex_];
    ++eventIndex_;
    return true;
  }
};

#ifdef __linux__
template<typename DSP_IF, typename DSP_AVX512, typename DSP_AVX2, typename DSP_AVX>
#else
template<typename DSP_IF, typename DSP_AVX2, typename DSP_AVX>
#endif
class SynthTester {
private:
  std::mutex mtx;
  std::shared_ptr<PresetQueue> queue;
  std::vector<std::thread> threads;

  std::string ref_dir;
  std::vector<std::string> test_dir;

public:
  bool isFinished = false;

  SynthTester(std::string plugin_name, std::string out_dir)
    : ref_dir(out_dir + "/reference/")
  {
    if (!checkInstrset()) return;

    test_dir.resize(2);
    test_dir[0] = out_dir + "/run1_init/";
    test_dir[1] = out_dir + "/run2_reset/";

    fs::create_directories(test_dir[0]);
    fs::create_directories(test_dir[1]);

    queue = std::make_shared<PresetQueue>(plugin_name);

    const auto n_cpu = std::thread::hardware_concurrency();
    for (size_t i = 0; i < n_cpu; ++i) {
#ifdef __linux__
      std::thread th(
        &SynthTester<DSPInterface, DSP_AVX512, DSP_AVX2, DSP_AVX>::testSequence, this,
        queue);
#else
      std::thread th(
        &SynthTester<DSPInterface, DSP_AVX2, DSP_AVX>::testSequence, this, queue);
#endif
      threads.push_back(std::move(th));
    }
    for (auto &th : threads) th.join();

    isFinished = true;
  }

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

  std::unique_ptr<DSP_IF> setupDSP()
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

  void render(
    const size_t nFrame,
    Sequencer &sequencer,
    std::vector<std::vector<float>> &wav,
    std::unique_ptr<DSP_IF> &dsp)
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

  SndFileResult writeWaveWithLock(
    std::string path, std::vector<std::vector<float>> &wav, int sampleRate)
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
          error_stream << "Error " << name << ": actual " << wav[ch][fr]
                       << " and expected " << ref.data_[ch][fr]
                       << " are not almost equal at channel " << ch << ", frame " << fr
                       << "\n";
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
      SoundFile ref = readReferenceWave(ref_dir + filename);

      render(nFrame, sequencer, wav, dsp);
      testAlmostEqual(filename + " init", error_stream, wav, ref);
      writeWaveWithLock(test_dir[0] + filename, wav, int(sampleRate));

      dsp->reset();

      render(nFrame, sequencer, wav, dsp);
      testAlmostEqual(filename + " reset", error_stream, wav, ref);
      writeWaveWithLock(test_dir[1] + filename, wav, int(sampleRate));

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
};
