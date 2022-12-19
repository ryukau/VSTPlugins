function(add_fftw3)
  add_library(fftw3 STATIC IMPORTED)
  if(MSVC)
    get_filename_component(fftw3_path ../lib/fftw3/windows/fftw3f.lib ABSOLUTE)
  elseif(UNIX)
    if(APPLE)
      get_filename_component(fftw3_path ../lib/fftw3/macOS/libfftw3f.a ABSOLUTE)
    else()
      if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        get_filename_component(fftw3_path ../lib/fftw3/linux/x86_64/libfftw3f.a ABSOLUTE)
      elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        get_filename_component(fftw3_path ../lib/fftw3/linux/aarch64/libfftw3f.a ABSOLUTE)
      else()
        message(FATAL_ERROR
          "FFTW3 static library for ${CMAKE_SYSTEM_PROCESSOR} is not available in this reposirotry.")
      endif()
    endif()
  endif()
  set_property(TARGET fftw3 PROPERTY IMPORTED_LOCATION ${fftw3_path})
endfunction()

function(add_common_resources target)
  # Custom font.
  smtg_target_add_plugin_resource(${target}
    "../common/resource/Fonts/Tinos-BoldItalic.ttf"
    "Fonts")
  smtg_target_add_plugin_resource(${target}
    "../common/resource/Fonts/LICENSE.txt"
    "Fonts")

  # Licenses.
  smtg_target_add_plugin_resource(${target}
    "../License/COPYING"
    "License")
  smtg_target_add_plugin_resource(${target}
    "../License/README.md"
    "License")
  smtg_target_add_plugin_resource(${target}
    "../License/Fonts/Tinos/LICENSE.txt"
    "License/Fonts/Tinos")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/AudioFFT/COPYING.txt"
    "License/lib/AudioFFT")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/fftw3/AUTHORS"
    "License/lib/fftw3")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/fftw3/COPYING"
    "License/lib/fftw3")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/fftw3/COPYRIGHT"
    "License/lib/fftw3")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/ghc/LICENSE"
    "License/lib/ghc")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/json/LICENSE.MIT"
    "License/lib/json")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/juce/LICENSE_juce_FastMathApproximations"
    "License/lib/juce")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/pcg-cpp/LICENSE-MIT.txt"
    "License/lib/pcg-cpp")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/pocketfft/LICENSE.md"
    "License/lib/pocketfft")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/sse2neon/LICENSE"
    "License/lib/sse2neon")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/sse2neon/README.md"
    "License/lib/sse2neon")
  smtg_target_add_plugin_resource(${target}
    "../License/lib/vcl/LICENSE"
    "License/lib/vcl")
  smtg_target_add_plugin_resource(${target}
    "../License/VST3_SDK/LICENSE.txt"
    "License/VST3_SDK")
  smtg_target_add_plugin_resource(${target}
    "../License/VST3_SDK/base/LICENSE.txt"
    "License/VST3_SDK/base")
  smtg_target_add_plugin_resource(${target}
    "../License/VST3_SDK/cmake/LICENSE.txt"
    "License/VST3_SDK/cmake")
  smtg_target_add_plugin_resource(${target}
    "../License/VST3_SDK/pluginterfaces/LICENSE.txt"
    "License/VST3_SDK/pluginterfaces")
  smtg_target_add_plugin_resource(${target}
    "../License/VST3_SDK/public.sdk/LICENSE.txt"
    "License/VST3_SDK/public.sdk")
  smtg_target_add_plugin_resource(${target}
    "../License/VST3_SDK/vstgui4/LICENSE"
    "License/VST3_SDK/vstgui4")
endfunction()
