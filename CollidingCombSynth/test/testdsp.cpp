#include <iostream>
#include <memory>

#include "../source/dsp/dspcore.hpp"

std::unique_ptr<DSPInterface> setupDSP()
{
  auto iset = instrset_detect();

#ifdef __linux__
  if (iset >= 10) {
    std::cout << "AVX512 is selected.\n";
    return std::make_unique<DSPCore_AVX512>();
  } else
#endif
    if (iset >= 8) {
    std::cout << "AVX2 is selected.\n";
    return std::make_unique<DSPCore_AVX2>();
  } else if (iset >= 7) {
    std::cout << "AVX is selected.\n";
    return std::make_unique<DSPCore_AVX>();
  }
  std::cerr << "\nError: Instruction set AVX or later not supported on this computer";
  exit(EXIT_FAILURE);
}

int main()
{
#ifdef TEST_DSP
  std::cout << "TEST_DSP is set.\n";
#endif

  auto dsp = setupDSP();

  return 0;
}
