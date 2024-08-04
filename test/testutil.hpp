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

#include "../../lib/ghc/fs_std.hpp"

#include <algorithm>
#include <deque>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <random>
#include <sndfile.h>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

// Disables `min` and `max` macro definition in `minwindef.h`.
#undef min
#undef max

#include "../../lib/json.hpp"

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
      std::cerr << "Error " << path << ": sf_open failed." << std::endl;
      isReady_ = SndFileResult::failure;
      return;
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
      return;
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

class TesterCommon {
protected:
  std::mutex mtx;
  std::shared_ptr<PresetQueue> queue;
  std::vector<std::thread> threads;

  std::string ref_dir;
  std::vector<std::string> test_dir;

public:
  bool isFinished = false;

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
};
