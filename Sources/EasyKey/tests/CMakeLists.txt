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

if(WIN32)
    add_executable(UpdateSignatureTests UpdateSignatureTests.cpp
        ../win32/EasyKey/EasyKey/UpdateSignature.cpp)
    target_compile_options(UpdateSignatureTests PRIVATE /utf-8)
    target_compile_definitions(UpdateSignatureTests PRIVATE
        EASYKEY_PUBLIC_CERT=L"${CMAKE_CURRENT_SOURCE_DIR}/../../../docs/signing/EasyKey-codesign.cer")
    target_link_libraries(UpdateSignatureTests PRIVATE wintrust crypt32)
    # signtool carries an embedded RFC 3161 Authenticode signature; Windows
    # system executables may instead be catalog-signed and are unsuitable here.
    file(GLOB SIGNED_SDK_TOOLS "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/bin/*/x64/signtool.exe")
    if(NOT SIGNED_SDK_TOOLS)
        message(FATAL_ERROR "Windows SDK signtool.exe is required for signature integration tests")
    endif()
    list(SORT SIGNED_SDK_TOOLS ORDER DESCENDING)
    list(GET SIGNED_SDK_TOOLS 0 SIGNED_SDK_TOOL)
    add_test(NAME UpdateSignatureTests COMMAND UpdateSignatureTests "${SIGNED_SDK_TOOL}")

    add_executable(UpdateCleanupTests UpdateCleanupTests.cpp
        ../win32/EasyKey/EasyKey/UpdateCleanup.cpp)
    target_compile_options(UpdateCleanupTests PRIVATE /utf-8)
    add_test(NAME UpdateCleanupTests COMMAND UpdateCleanupTests)
endif()
