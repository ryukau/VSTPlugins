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
    "../License/README.md"
    "License")
endfunction()
