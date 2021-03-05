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

  set(dspcore_avx2 "${target}_dspcore_avx2")
  set(dspcore_avx2 ${dspcore_avx2} PARENT_SCOPE)

  message("target: ${target}")
  message("avx: ${dspcore_avx}")
  message("${target}_dspcore_avx")
  add_library(${dspcore_avx} OBJECT source/dsp/dspcore.cpp)
  add_library(${dspcore_avx2} OBJECT source/dsp/dspcore.cpp)

  if(UNIX AND NOT APPLE) # Linux.
    # I couldn't build FFTW3 with AVX512 on Windows and macOS.
    set(dspcore_avx512 "${target}_dspcore_avx512")
    set(dspcore_avx512 ${dspcore_avx512} PARENT_SCOPE)
    add_library(${dspcore_avx512} OBJECT source/dsp/dspcore.cpp)
  endif()

  if(MSVC)
    set(dsp_msvc_flag
      $<$<CONFIG:Debug>:>
      $<$<CONFIG:Release>:/O2 /fp:fast>)
    target_compile_options(${dspcore_avx} PRIVATE ${dsp_msvc_flag} /arch:AVX)
    target_compile_options(${dspcore_avx2} PRIVATE ${dsp_msvc_flag} /arch:AVX2)
    # target_compile_options(${dspcore_avx512} PRIVATE ${dsp_msvc_flag} /arch:AVX512)
  elseif(UNIX)
    if(APPLE)
      set(dsp_apple_clang_flag
        $<$<CONFIG:Debug>:-g -fPIC -fno-aligned-allocation>
        $<$<CONFIG:Release>:-O3 -fPIC -fno-aligned-allocation>)
      target_compile_options(${dspcore_avx} PRIVATE ${dsp_apple_clang_flag} -mavx)
      target_compile_options(${dspcore_avx2} PRIVATE ${dsp_apple_clang_flag} -mavx2 -mfma)
      # target_compile_options(${dspcore_avx512} PRIVATE ${dsp_apple_clang_flag} -mavx512f -mfma -mavx512vl -mavx512bw -mavx512dq)
    else()
      set(dsp_linux_gcc_flag
        $<$<CONFIG:Debug>:-g -fPIC>
        $<$<CONFIG:Release>:-O3 -fPIC>)
      target_compile_options(${dspcore_avx} PRIVATE ${dsp_linux_gcc_flag} -mavx)
      target_compile_options(${dspcore_avx2} PRIVATE ${dsp_linux_gcc_flag} -mavx2 -mfma)
      target_compile_options(${dspcore_avx512} PRIVATE ${dsp_linux_gcc_flag} -mavx512f -mfma -mavx512vl -mavx512bw -mavx512dq)
    endif()
  endif()
endfunction()

function(build_test)
  get_plugin_name(PLUGIN_NAME)
  set(target "testdsp_${PLUGIN_NAME}")
  message("add build: ${target}")

  add_compile_definitions(TEST_DSP)

  build_dspcore(${target})
  add_executable(${target} test/testdsp.cpp)
  add_library(src
  ../lib/vcl/instrset_detect.cpp)
  target_link_libraries(${target} PRIVATE
    ${dspcore_avx}
    ${dspcore_avx2}
    ${dspcore_avx512}
    src)
endfunction()

function(build_vst3)
  get_plugin_name(PLUGIN_NAME)
  set(target ${PLUGIN_NAME})

  build_dspcore(${target})

  # VST 3 settings.
  set(plug_sources
    ../lib/vcl/instrset_detect.cpp
    source/gui/splashdraw.cpp
    source/plugprocessor.cpp
    source/editor.cpp
    source/plugfactory.cpp)

  smtg_add_vst3plugin(${target} ${plug_sources})
  if(APPLE)
    target_compile_options(${target} PRIVATE -fno-aligned-allocation)
  endif()
  set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
  target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
  include_directories(../common)
  target_link_libraries(${target} PRIVATE
    ${dspcore_avx}
    ${dspcore_avx2}
    ${dspcore_avx512}
    UhhyouCommon
    base
    sdk
    vstgui_support)

  smtg_add_vst3_snapshot(${target} "resource/10435C85B1B54FD9ACE981C7E46B02FF_snapshot.png")

  if(SMTG_MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
  elseif(SMTG_WIN)
    target_sources(${target} PRIVATE resource/plug.rc)
  endif()
endfunction()

# Starts from here.
cmake_minimum_required(VERSION 3.12)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(TEST_PLUGIN)
  build_test()
else()
  build_vst3()
endif()
