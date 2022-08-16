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

#pragma once

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

template<typename DSP_CLASS> class SynthTester : public TesterCommon {
public:
  // n_thread is used to disable multithreading. Useful for plugin using FFTW3.
  SynthTester(
    std::string plugin_name,
    std::string out_dir,
    int n_thread = std::thread::hardware_concurrency())
  {
    ref_dir = out_dir + "/reference/";
    test_dir.resize(2);
    test_dir[0] = out_dir + "/run1_init/";
    test_dir[1] = out_dir + "/run2_reset/";

    fs::create_directories(test_dir[0]);
    fs::create_directories(test_dir[1]);

    queue = std::make_shared<PresetQueue>(plugin_name);

    for (size_t i = 0; i < n_thread; ++i) {
      std::thread th(&SynthTester<DSP_CLASS>::testSequence, this, queue);
      threads.push_back(std::move(th));
    }
    for (auto &th : threads) th.join();

    isFinished = true;
  }

  std::unique_ptr<DSP_CLASS> setupDSP() { return std::make_unique<DSP_CLASS>(); }

  void processDsp(
    size_t length,
    size_t currentFrame,
    std::vector<std::vector<float>> &wav,
    std::unique_ptr<DSP_CLASS> &dsp)
  {
#ifdef HAS_INPUT
    dsp->process(
      length, wav[0].data() + currentFrame, wav[1].data() + currentFrame,
      wav[0].data() + currentFrame, wav[1].data() + currentFrame);
#else
    dsp->process(length, wav[0].data() + currentFrame, wav[1].data() + currentFrame);
#endif
  }

  void render(
    const size_t nFrame,
    Sequencer &sequencer,
    std::vector<std::vector<float>> &wav,
    std::unique_ptr<DSP_CLASS> &dsp)
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
            case NoteEventType::noteOn: {
              dsp->noteOn(note.id, note.pitch, note.tuning, note.velocity);

              // // UltraSynth requires this. Maybe refactor at some point.
              // DSP_CLASS::NoteInfo info;
              // info.isNoteOn = true;
              // info.frame = uint32_t(currentFrame);
              // info.id = note.id;
              // info.noteNumber = note.pitch + note.tuning;
              // info.velocity = note.velocity;
              // dsp->noteOn(info);
            } break;
            case NoteEventType::noteOff:
              dsp->noteOff(note.id);
              break;
          }
        }
        nextFrame = sequencer.nextFrame();
      } while (nextFrame == currentFrame);
      if (nextFrame >= nFrame) break;

      processDsp(nextFrame - currentFrame, currentFrame, wav, dsp);
      currentFrame = nextFrame;
    }
    processDsp(nFrame - currentFrame, currentFrame, wav, dsp);
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
      SET_PARAMETERS;
      dsp->reset();

      std::string filename = preset["name"].get<std::string>() + ".wav";
      std::stringstream error_stream;
      SoundFile ref = readReferenceWave(ref_dir + filename);

      render(nFrame, sequencer, wav, dsp);
      testAlmostEqual(filename + " init", error_stream, wav, ref);
      writeWaveWithLock(test_dir[0] + filename, wav, int(sampleRate));

      for (auto &wv : wav) std::fill(wv.begin(), wv.end(), 0.0f);
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

#ifdef __linux__
template<typename DSP_IF, typename DSP_AVX512, typename DSP_AVX2, typename DSP_AVX>
#else
template<typename DSP_IF, typename DSP_AVX2, typename DSP_AVX>
#endif
class SynthTesterSimdRuntimeDispatch : public TesterCommon {
public:
  // n_thread is used to disable multithreading. Useful for plugin using FFTW3.
  SynthTesterSimdRuntimeDispatch(
    std::string plugin_name,
    std::string out_dir,
    int n_thread = std::thread::hardware_concurrency())
  {
    if (!checkInstrset()) return;

    ref_dir = out_dir + "/reference/";
    test_dir.resize(2);
    test_dir[0] = out_dir + "/run1_init/";
    test_dir[1] = out_dir + "/run2_reset/";

    fs::create_directories(test_dir[0]);
    fs::create_directories(test_dir[1]);

    queue = std::make_shared<PresetQueue>(plugin_name);

    for (size_t i = 0; i < n_thread; ++i) {
#ifdef __linux__
      std::thread th(
        &SynthTesterSimdRuntimeDispatch<
          DSPInterface, DSP_AVX512, DSP_AVX2, DSP_AVX>::testSequence,
        this, queue);
#else
      std::thread th(
        &SynthTesterSimdRuntimeDispatch<DSPInterface, DSP_AVX2, DSP_AVX>::testSequence,
        this, queue);
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
      if (iset >= 8)
    {
      std::cout << "AVX2 is selected.\n";
      return true;
    } else if (iset >= 7) {
      std::cout << "AVX is selected.\n";
      return true;
    }
    std::cerr
      << "Error: Instruction set AVX or later is not supported on this computer\n";
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
      if (iset >= 8)
    {
      return std::make_unique<DSPCore_AVX2>();
    } else if (iset >= 7) {
      return std::make_unique<DSPCore_AVX>();
    }
    return nullptr;
  }

  void processDsp(
    size_t length,
    size_t currentFrame,
    std::vector<std::vector<float>> &wav,
    std::unique_ptr<DSP_IF> &dsp)
  {
#ifdef HAS_INPUT
    dsp->process(
      length, wav[0].data() + currentFrame, wav[1].data() + currentFrame,
      wav[0].data() + currentFrame, wav[1].data() + currentFrame);
#else
    dsp->process(length, wav[0].data() + currentFrame, wav[1].data() + currentFrame);
#endif
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

      processDsp(nextFrame - currentFrame, currentFrame, wav, dsp);
      currentFrame = nextFrame;
    }
    processDsp(nFrame - currentFrame, currentFrame, wav, dsp);
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
      SET_PARAMETERS;
      dsp->reset();

      std::string filename = preset["name"].get<std::string>() + ".wav";
      std::stringstream error_stream;
      SoundFile ref = readReferenceWave(ref_dir + filename);

      render(nFrame, sequencer, wav, dsp);
      testAlmostEqual(filename + " init", error_stream, wav, ref);
      writeWaveWithLock(test_dir[0] + filename, wav, int(sampleRate));

      for (auto &wv : wav) std::fill(wv.begin(), wv.end(), 0.0f);
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
