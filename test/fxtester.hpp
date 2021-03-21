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

template<typename Sample>
inline std::vector<std::vector<Sample>> generateTestNoise(size_t nFrame)
{
  std::minstd_rand rng{unsigned(26935804702)};

  // Standard deviation (sigma) is set to 0.25 / 3.
  // 99.7% of output falls under + or -3 * sigma.
  std::normal_distribution<Sample> dist{Sample(0), Sample(0.08333333333333333)};

  std::vector<std::vector<Sample>> data(2);
  for (auto &dt : data) dt.resize(nFrame);

  for (size_t i = 0; i < nFrame; ++i) {
    data[0][i] = std::clamp<Sample>(dist(rng), Sample(-0.25), Sample(0.25f));
    data[1][i] = std::clamp<Sample>(dist(rng), Sample(-0.25), Sample(0.25f));
  }
  return data;
}

#ifdef __linux__
template<typename DSP_IF, typename DSP_AVX512, typename DSP_AVX2, typename DSP_AVX>
#else
template<typename DSP_IF, typename DSP_AVX2, typename DSP_AVX>
#endif
class FxTester {
private:
  std::mutex mtx;
  std::shared_ptr<PresetQueue> queue;
  std::vector<std::thread> threads;

  std::string ref_dir;
  std::vector<std::string> test_dir;

public:
  bool isFinished = false;

  FxTester(std::string plugin_name, std::string out_dir)
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
        &FxTester<DSPInterface, DSP_AVX512, DSP_AVX2, DSP_AVX>::testSequence, this,
        queue);
#else
      std::thread th(
        &FxTester<DSPInterface, DSP_AVX2, DSP_AVX>::testSequence, this, queue);
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
    const std::vector<std::vector<float>> &in,
    std::vector<std::vector<float>> &wav,
    std::unique_ptr<DSP_IF> &dsp)
  {
    dsp->process(nFrame, in[0].data(), in[1].data(), wav[0].data(), wav[1].data());
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
    const std::vector<std::vector<float>> &wav,
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

    const auto input = generateTestNoise<float>(nFrame);

    std::vector<std::vector<float>> wav(2);
    for (auto &channel : wav) channel.resize(nFrame);

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

      render(nFrame, input, wav, dsp);
      testAlmostEqual(filename + " init", error_stream, wav, ref);
      writeWaveWithLock(test_dir[0] + filename, wav, int(sampleRate));

      dsp->reset();

      render(nFrame, input, wav, dsp);
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
