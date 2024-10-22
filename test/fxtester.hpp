// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "testutil.hpp"

#include <chrono>
#include <random>
#include <thread>

#ifndef HAS_SIDECHAIN
  #define HAS_SIDECHAIN 0
#endif

constexpr uint64_t hasSidechain = HAS_SIDECHAIN;

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

template<typename DSP_CLASS> class FxTester : public TesterCommon {
public:
  FxTester(
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
      std::thread th(&FxTester<DSP_CLASS>::testSequence, this, queue);
      threads.push_back(std::move(th));
    }
    for (auto &th : threads) th.join();

    isFinished = true;
  }

  std::unique_ptr<DSP_CLASS> setupDSP() { return std::make_unique<DSP_CLASS>(); }

  void render(
    const size_t nFrame,
    const std::vector<std::vector<float>> &in,
    std::vector<std::vector<float>> &wav,
    std::unique_ptr<DSP_CLASS> &dsp)
  {
    if constexpr (hasSidechain) {
      dsp->process(
        nFrame, in[0].data(), in[1].data(), in[0].data(), in[1].data(), wav[0].data(),
        wav[1].data());
    } else {
      dsp->process(nFrame, in[0].data(), in[1].data() wav[0].data(), wav[1].data());
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
        std::cerr << "Error: setupDSP failed.\n";
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

      if (!std::filesystem::exists(ref_dir + filename)) {
        render(nFrame, sequencer, wav, dsp);
        writeWaveWithLock(ref_dir + filename, wav, int(sampleRate));

        for (auto &wv : wav) std::fill(wv.begin(), wv.end(), 0.0f);
        dsp->reset();
      }
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

#ifdef __linux__
template<typename DSP_IF, typename DSP_AVX512, typename DSP_AVX2, typename DSP_AVX>
#else
template<typename DSP_IF, typename DSP_AVX2, typename DSP_AVX>
#endif
class FxTesterSimdRuntimeDispatch : public TesterCommon {
public:
  FxTesterSimdRuntimeDispatch(
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
        &FxTesterSimdRuntimeDispatch<
          DSPInterface, DSP_AVX512, DSP_AVX2, DSP_AVX>::testSequence,
        this, queue);
#else
      std::thread th(
        &FxTesterSimdRuntimeDispatch<DSPInterface, DSP_AVX2, DSP_AVX>::testSequence, this,
        queue);
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
      if (iset >= 8)
    {
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
    if constexpr (hasSidechain) {
      dsp->process(
        nFrame, in[0].data(), in[1].data(), in[0].data(), in[1].data(), wav[0].data(),
        wav[1].data());
    } else {
      dsp->process(nFrame, in[0].data(), in[1].data() wav[0].data(), wav[1].data());
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
        std::cerr << "Error: setupDSP failed.\n";
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

      if (!std::filesystem::exists(ref_dir + filename)) {
        render(nFrame, sequencer, wav, dsp);
        writeWaveWithLock(ref_dir + filename, wav, int(sampleRate));

        for (auto &wv : wav) std::fill(wv.begin(), wv.end(), 0.0f);
        dsp->reset();
      }
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
