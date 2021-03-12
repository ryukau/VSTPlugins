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

#include "../../lib/ghc/fs_std.hpp"
#include "../../lib/json.hpp"

#include <algorithm>
#include <deque>
#include <iostream>
#include <mutex>
#include <sndfile.h>
#include <string>

enum class SndFileResult { success, failure };

inline SndFileResult
writeWave(std::string filename, std::vector<float> &data, const size_t &samplerate)
{
  SF_INFO sfinfo;
  memset(&sfinfo, 0, sizeof(sfinfo));
  sfinfo.samplerate = int(samplerate);
  sfinfo.frames = data.size();
  sfinfo.channels = 1;
  sfinfo.format = (SF_FORMAT_WAV | SF_FORMAT_FLOAT);

  SNDFILE *file = sf_open(filename.c_str(), SFM_WRITE, &sfinfo);
  if (!file) {
    std::cerr << "Error " << filename << ": sf_open failed." << std::endl;
    return SndFileResult::failure;
  }

  size_t length = sfinfo.channels * data.size();
  if (sf_write_float(file, &data[0], length) != length)
    std::cerr << sf_strerror(file) << std::endl;

  if (sf_close(file) != 0) {
    std::cerr << "Error " << filename << ": sf_close failed." << std::endl;
    return SndFileResult::failure;
  }
  return SndFileResult::success;
}

inline SndFileResult writeWave(
  std::string filename, std::vector<std::vector<float>> &data, const int samplerate)
{
  SF_INFO sfinfo;
  memset(&sfinfo, 0, sizeof(sfinfo));
  sfinfo.samplerate = samplerate;
  sfinfo.frames = sf_count_t(data[0].size());
  sfinfo.channels = int(data.size());
  sfinfo.format = (SF_FORMAT_WAV | SF_FORMAT_FLOAT);

  SNDFILE *file = sf_open(filename.c_str(), SFM_WRITE, &sfinfo);
  if (!file) {
    std::cerr << "Error " << filename << ": sf_open failed." << std::endl;
    return SndFileResult::failure;
  }

  size_t length = data.size() * data[0].size();

  // Transpose buffer [ch0, ch1] to [frame0, frame1, ...].
  std::vector<float> buffer;
  buffer.reserve(length);
  for (size_t i = 0; i < data[0].size(); ++i) {
    for (size_t j = 0; j < data.size(); ++j)
      buffer.push_back(i < data[j].size() ? data[j][i] : 0.0f);
  }

  if (sf_write_float(file, &buffer[0], length) != length)
    std::cerr << sf_strerror(file) << std::endl;

  if (sf_close(file) != 0) {
    std::cerr << "Error " << filename << ": sf_close failed." << std::endl;
    return SndFileResult::failure;
  }
  return SndFileResult::success;
}

class SoundFile {
private:
  SndFileResult isReady_ = SndFileResult::failure;

public:
  int samplerate_ = 0;
  size_t channels_ = 0;
  size_t frames_ = 0;
  std::vector<std::vector<float>> data_;

  SoundFile(std::string path)
  {
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));

    SNDFILE *file = sf_open(path.c_str(), SFM_READ, &sfinfo);
    if (!file) {
      std::cerr << "Error: sf_open failed." << std::endl;
      isReady_ = SndFileResult::failure;
    }

    samplerate_ = sfinfo.samplerate;
    channels_ = sfinfo.channels;
    frames_ = sfinfo.frames;

    size_t items = channels_ * frames_;
    std::vector<float> raw(items);
    sf_read_float(file, &raw[0], items);

    data_.resize(channels_);
    for (size_t ch = 0; ch < channels_; ++ch) {
      data_[ch].resize(frames_);
      for (size_t fr = 0; fr < frames_; ++fr) data_[ch][fr] = raw[channels_ * fr + ch];
    }

    if (sf_close(file) != 0) {
      std::cerr << "Error: sf_close failed." << std::endl;
      isReady_ = SndFileResult::failure;
    }

    isReady_ = SndFileResult::success;
  }

  bool isReady() { return isReady_ == SndFileResult::success; }
};

inline nlohmann::json loadPresetJson(std::string plugin_name)
{
  nlohmann::json data;

  auto preset_json_name = plugin_name + ".preset.json";
  auto preset_path = fs::path("../../presets/json") / fs::path(preset_json_name);

  if (!fs::is_regular_file(preset_path)) {
    std::cerr << preset_path << " is not regular file or doesn't exist.\n";
    return data;
  }

  std::ifstream ifs(preset_path);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open " << preset_path << "\n";
    return data;
  }

  ifs >> data;
  return data;
}

struct PresetQueue {
  using json = nlohmann::json;

  std::mutex mtx;
  json data;
  json::iterator iter;

  PresetQueue(std::string plugin_name)
    : data(loadPresetJson(plugin_name)), iter(data.begin())
  {
  }

  json::iterator end() { return data.end(); }

  json::iterator next()
  {
    std::unique_lock<std::mutex> lk{mtx};
    auto preset = iter;
    if (iter != data.end()) {
      std::cout << "Processing preset: " << preset.value()["name"] << "\n";
      ++iter;
    }
    return preset;
  }
};

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
