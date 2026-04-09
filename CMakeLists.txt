cmake_minimum_required(VERSION 3.20)

project(Heizungssteuerung LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

option(HEIZUNG_USE_ARDUINO_SDK "Use real Arduino core and library headers" OFF)
option(HEIZUNG_ENABLE_TESTS "Build host-side Unity tests" OFF)

set(ARDUINO_CORE_DIR "" CACHE PATH "Path to Arduino core directory (e.g. .../cores/arduino)")
set(ARDUINO_VARIANT_DIR "" CACHE PATH "Path to Arduino variant directory (e.g. .../variants/standard)")
set(ARDUINO_WIRE_DIR "" CACHE PATH "Path to Wire library src directory")
set(ARDUINO_LIB_ENCODER_DIR "" CACHE PATH "Path to Encoder library directory")
set(ARDUINO_LIB_DALLAS_DIR "" CACHE PATH "Path to DallasTemperature library directory")
set(ARDUINO_LIB_ONEWIRE_DIR "" CACHE PATH "Path to OneWire library directory")
set(ARDUINO_LIB_LCD_I2C_DIR "" CACHE PATH "Path to LiquidCrystal_I2C library directory")
set(UNITY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unity" CACHE PATH "Path to Unity framework root containing src/unity.c and src/unity.h")

add_library(heizungssteuerung OBJECT)

target_sources(
  heizungssteuerung
  PRIVATE
    modularmain/modularmain.ino
    modularmain/controller.h
    modularmain/devicegroups.h
    modularmain/hardwaredrivers.h
    modularmain/types.h
    cmake/arduino_stubs/Wire.cpp
)

set_source_files_properties(modularmain/modularmain.ino PROPERTIES LANGUAGE CXX)

target_include_directories(heizungssteuerung PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modularmain)
target_compile_definitions(heizungssteuerung PRIVATE ARDUINO=10607 ARDUINO_AVR_UNO ARDUINO_ARCH_AVR F_CPU=16000000L)
target_compile_options(heizungssteuerung PRIVATE -fpermissive)

set(HEIZUNG_COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/modularmain)

if(HEIZUNG_USE_ARDUINO_SDK)
  foreach(path_var
          ARDUINO_CORE_DIR
          ARDUINO_VARIANT_DIR
          ARDUINO_WIRE_DIR
          ARDUINO_LIB_ENCODER_DIR
          ARDUINO_LIB_DALLAS_DIR
          ARDUINO_LIB_ONEWIRE_DIR
          ARDUINO_LIB_LCD_I2C_DIR)
    if(NOT EXISTS "${${path_var}}")
      message(FATAL_ERROR "${path_var} is not set to an existing path.")
    endif()
  endforeach()

  target_include_directories(
    heizungssteuerung
    PRIVATE
      ${ARDUINO_CORE_DIR}
      ${ARDUINO_VARIANT_DIR}
      ${ARDUINO_WIRE_DIR}
      ${ARDUINO_LIB_ENCODER_DIR}
      ${ARDUINO_LIB_DALLAS_DIR}
      ${ARDUINO_LIB_ONEWIRE_DIR}
      ${ARDUINO_LIB_LCD_I2C_DIR}
  )
  list(APPEND HEIZUNG_COMMON_INCLUDE_DIRS
       ${ARDUINO_CORE_DIR}
       ${ARDUINO_VARIANT_DIR}
       ${ARDUINO_WIRE_DIR}
       ${ARDUINO_LIB_ENCODER_DIR}
       ${ARDUINO_LIB_DALLAS_DIR}
       ${ARDUINO_LIB_ONEWIRE_DIR}
       ${ARDUINO_LIB_LCD_I2C_DIR})
else()
  target_include_directories(heizungssteuerung PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arduino_stubs)
  list(APPEND HEIZUNG_COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arduino_stubs)
endif()

if(HEIZUNG_ENABLE_TESTS)
  if(NOT EXISTS "${UNITY_ROOT}/src/unity.c")
    message(FATAL_ERROR "Unity source not found: ${UNITY_ROOT}/src/unity.c")
  endif()
  if(NOT EXISTS "${UNITY_ROOT}/src/unity.h")
    message(FATAL_ERROR "Unity header not found: ${UNITY_ROOT}/src/unity.h")
  endif()

  enable_testing()

  add_executable(
    test_unity_entry
    tests/test_unity_entry.cpp
    ${UNITY_ROOT}/src/unity.c
  )

  target_include_directories(
    test_unity_entry
    PRIVATE
      ${HEIZUNG_COMMON_INCLUDE_DIRS}
      ${UNITY_ROOT}/src
      ${CMAKE_CURRENT_SOURCE_DIR}/tests
  )

  target_compile_definitions(test_unity_entry PRIVATE ARDUINO=10607 ARDUINO_AVR_UNO ARDUINO_ARCH_AVR F_CPU=16000000L)
  target_compile_options(test_unity_entry PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>)

  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cases.def" HEIZUNG_TEST_CASES_DEF)
  string(REGEX MATCHALL "TEST_CASE\\(([A-Za-z0-9_]+)\\)" HEIZUNG_TEST_CASES_MATCHES "${HEIZUNG_TEST_CASES_DEF}")
  set(HEIZUNG_TEST_NAMES "")
  foreach(match IN LISTS HEIZUNG_TEST_CASES_MATCHES)
    string(REGEX REPLACE "TEST_CASE\\(([A-Za-z0-9_]+)\\)" "\\1" test_name "${match}")
    list(APPEND HEIZUNG_TEST_NAMES "${test_name}")
  endforeach()

  if(HEIZUNG_TEST_NAMES STREQUAL "")
    message(FATAL_ERROR "No test cases found in tests/test_cases.def")
  endif()

  foreach(test_name IN LISTS HEIZUNG_TEST_NAMES)
    add_test(NAME "unity.${test_name}" COMMAND test_unity_entry "${test_name}")
  endforeach()
endif()
