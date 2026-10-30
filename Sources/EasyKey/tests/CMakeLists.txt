cmake_minimum_required(VERSION 3.16)
project(EasyKeyEngineTests CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../engine)

add_executable(EngineTests
    EngineTests.cpp
    ${ENGINE_DIR}/ConvertTool.cpp
    ${ENGINE_DIR}/Engine.cpp
    ${ENGINE_DIR}/Macro.cpp
    ${ENGINE_DIR}/MixedWords.cpp
    ${ENGINE_DIR}/SmartSwitchKey.cpp
    ${ENGINE_DIR}/Vietnamese.cpp
)

if(MSVC)
    target_compile_options(EngineTests PRIVATE /utf-8)
    target_compile_definitions(EngineTests PRIVATE _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(EngineTests PRIVATE LINUX)
endif()

enable_testing()
add_test(NAME EngineTests COMMAND EngineTests)
