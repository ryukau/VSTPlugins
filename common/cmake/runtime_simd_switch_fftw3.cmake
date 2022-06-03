function(get_plugin_name NAME_VAR)
  get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  set(${NAME_VAR} ${PLUGIN_NAME} PARENT_SCOPE)
endfunction()

#
# This function returns following variables using `set(... PARENT_SCOPE)`.
# - dspcore_avx
# - dspcore_avx2
# - dspcore_avx512 (Linux only)
#
# `dspcore_avx512` must be empty string on Windows and macOS.
#
function(build_dspcore target)
  set(dspcore_avx "${target}_dspcore_avx")
  set(dspcore_avx ${dspcore_avx} PARENT_SCOPE)
  add_library(${dspcore_avx} OBJECT source/dsp/dspcore.cpp)

  set(dspcore_avx2 "${target}_dspcore_avx2")
  set(dspcore_avx2 ${dspcore_avx2} PARENT_SCOPE)
  add_library(${dspcore_avx2} OBJECT source/dsp/dspcore.cpp)

  if(UNIX AND NOT APPLE) # Linux.
    # I couldn't build FFTW3 with AVX512 on Windows and macOS.
    set(dspcore_avx512 "${target}_dspcore_avx512")
    set(dspcore_avx512 ${dspcore_avx512} PARENT_SCOPE)
    add_library(${dspcore_avx512} OBJECT source/dsp/dspcore.cpp)
  endif()

  if(MSVC)
    set(dsp_msvc_flag
      $<$<CONFIG:Debug>:/W4 /wd4127 /wd4324 /wd4458>
      $<$<CONFIG:Release>:/O2 /fp:fast /W4 /wd4127 /wd4324 /wd4458>)
    target_compile_options(${dspcore_avx} PRIVATE ${dsp_msvc_flag} /arch:AVX)
    target_compile_options(${dspcore_avx2} PRIVATE ${dsp_msvc_flag} /arch:AVX2)
  elseif(UNIX)
    if(APPLE)
      set(dsp_apple_clang_flag
        $<$<CONFIG:Debug>:-g -fPIC -fno-aligned-allocation -Wall>
        $<$<CONFIG:Release>:-O3 -fPIC -fno-aligned-allocation -Wall>)
      target_compile_options(${dspcore_avx} PRIVATE ${dsp_apple_clang_flag} -mavx)
      target_compile_options(${dspcore_avx2} PRIVATE ${dsp_apple_clang_flag} -mavx2 -mfma)
    else()
      set(dsp_linux_gcc_flag
        $<$<CONFIG:Debug>:-g -fPIC -Wall>
        $<$<CONFIG:Release>:-O3 -fPIC -Wall>)
      target_compile_options(${dspcore_avx} PRIVATE ${dsp_linux_gcc_flag} -mavx)
      target_compile_options(${dspcore_avx2} PRIVATE ${dsp_linux_gcc_flag} -mavx2 -mfma)
      target_compile_options(${dspcore_avx512} PRIVATE ${dsp_linux_gcc_flag} -mavx512f -mfma -mavx512vl -mavx512bw -mavx512dq)
    endif()
  endif()
endfunction()

function(add_fftw3)
  add_library(fftw3 STATIC IMPORTED)
  if(MSVC)
    get_filename_component(fftw3_path ../lib/fftw3/windows/fftw3f.lib ABSOLUTE)
  elseif(UNIX)
    if(APPLE)
      get_filename_component(fftw3_path ../lib/fftw3/macOS/libfftw3f.a ABSOLUTE)
    else()
      get_filename_component(fftw3_path ../lib/fftw3/linux/libfftw3f.a ABSOLUTE)
    endif()
  endif()
  set_property(TARGET fftw3 PROPERTY IMPORTED_LOCATION ${fftw3_path})
endfunction()

function(build_test)
  find_package(SndFile CONFIG REQUIRED)

  get_plugin_name(PLUGIN_NAME)
  set(target "testdsp_${PLUGIN_NAME}")
  add_compile_definitions(TEST_DSP)

  build_dspcore(${target})

  add_executable(${target} test/testdsp.cpp)
  target_compile_definitions(${target} PRIVATE
    UHHYOU_PLUGIN_NAME="${PLUGIN_NAME}")

  set(src "${target}_source")
  add_fftw3()
  add_library(${src}
    ../lib/vcl/instrset_detect.cpp
    source/parameter.cpp)
  target_link_libraries(${target} PRIVATE
    SndFile::sndfile
    ${dspcore_avx}
    ${dspcore_avx2}
    ${dspcore_avx512}
    fftw3
    ${src})
endfunction()

function(build_vst3 plug_sources)
  get_plugin_name(PLUGIN_NAME)
  set(target ${PLUGIN_NAME})

  build_dspcore(${target})

  smtg_add_vst3plugin(${target} ${plug_sources})
  if(APPLE)
    target_compile_options(${target} PRIVATE -fno-aligned-allocation)
  endif()
  set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
  target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
  include_directories(../common)
  add_fftw3()
  target_link_libraries(${target} PRIVATE
    ${dspcore_avx}
    ${dspcore_avx2}
    ${dspcore_avx512}
    UhhyouCommon
    fftw3
    base
    sdk
    vstgui_support)

  file(GLOB  snapshots "resource/*_snapshot.png")
  list(LENGTH snapshots length)
  if(length GREATER 0)
    list(GET snapshots 0 snapshot_path)
    if(length GREATER 1)
      message(WARNING "Several snapshots found. Using ${snapshot_path}")
    endif()
    smtg_target_add_plugin_resource(${target} "${snapshot_path}" "Snapshots")
  endif()

  smtg_target_add_plugin_resource(${target} "../common/resource/Fonts/Tinos-BoldItalic.ttf" "Fonts")
  smtg_target_add_plugin_resource(${target} "../common/resource/Fonts/LICENSE.txt" "Fonts")

  if(SMTG_MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
  elseif(SMTG_WIN)
    target_sources(${target} PRIVATE resource/plug.rc)
  endif()
endfunction()
