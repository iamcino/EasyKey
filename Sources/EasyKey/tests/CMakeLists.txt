cmake_minimum_required(VERSION 3.16)
project(EasyKeyEngineTests CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../engine)
set(APP_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../app-core)

add_executable(EngineTests
    EngineTests.cpp
    ${APP_CORE_DIR}/VersionInfo.cpp
    ${APP_CORE_DIR}/GameModeState.cpp
    ${APP_CORE_DIR}/PasswordState.cpp
    ${APP_CORE_DIR}/AppProfileState.cpp
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

#State characterization for plan v2.2 Part B: includes Engine.cpp in its
#own translation unit (probe-style) to assert engine session state at
#event boundaries; Windows-only cells, a no-op elsewhere. Engine.cpp is
#deliberately not a separate source here -- the driver includes it.
add_executable(EngineStateTests
    EngineStateTests.cpp
    ${APP_CORE_DIR}/VersionInfo.cpp
    ${APP_CORE_DIR}/GameModeState.cpp
    ${APP_CORE_DIR}/PasswordState.cpp
    ${APP_CORE_DIR}/AppProfileState.cpp
    ${ENGINE_DIR}/ConvertTool.cpp
    ${ENGINE_DIR}/Macro.cpp
    ${ENGINE_DIR}/MixedWords.cpp
    ${ENGINE_DIR}/SmartSwitchKey.cpp
    ${ENGINE_DIR}/Vietnamese.cpp
)
if(MSVC)
    target_compile_options(EngineStateTests PRIVATE /utf-8)
    target_compile_definitions(EngineStateTests PRIVATE _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(EngineStateTests PRIVATE LINUX)
endif()

#Bug D: the engine/hook packet contract (plain Backspace, modified word break,
#mouse click) and the hook's decision whether a packet may be injected
add_executable(PacketDeliveryTests
    PacketDeliveryTests.cpp
    ${ENGINE_DIR}/ConvertTool.cpp
    ${ENGINE_DIR}/Engine.cpp
    ${ENGINE_DIR}/Macro.cpp
    ${ENGINE_DIR}/MixedWords.cpp
    ${ENGINE_DIR}/SmartSwitchKey.cpp
    ${ENGINE_DIR}/Vietnamese.cpp
)
if(MSVC)
    target_compile_options(PacketDeliveryTests PRIVATE /utf-8)
    target_compile_definitions(PacketDeliveryTests PRIVATE _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(PacketDeliveryTests PRIVATE LINUX)
endif()

enable_testing()
add_test(NAME EngineTests COMMAND EngineTests)
add_test(NAME EngineStateTests COMMAND EngineStateTests)
add_test(NAME PacketDeliveryTests COMMAND PacketDeliveryTests)

#Windows-only: deleting EasyKey.old.exe after an update (locked, read-only, directory),
#and the updater's Authenticode check against the pinned EasyKey certificate
if(WIN32)
    add_executable(UpdateSignatureTests UpdateSignatureTests.cpp
        ../win32/EasyKey/EasyKey/UpdateSignature.cpp)
    target_compile_options(UpdateSignatureTests PRIVATE /utf-8)
    target_compile_definitions(UpdateSignatureTests PRIVATE
        EASYKEY_PUBLIC_CERT=L"${CMAKE_CURRENT_SOURCE_DIR}/../../../docs/signing/EasyKey-codesign.cer")
    target_link_libraries(UpdateSignatureTests PRIVATE wintrust crypt32)
    #signtool carries an embedded RFC 3161 Authenticode signature; Windows system
    #executables may be catalog-signed instead and do not fit here
    file(GLOB SIGNED_SDK_TOOLS "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/bin/*/x64/signtool.exe")
    if(NOT SIGNED_SDK_TOOLS)
        message(FATAL_ERROR "Windows SDK signtool.exe is required for signature tests")
    endif()
    list(SORT SIGNED_SDK_TOOLS ORDER DESCENDING)
    list(GET SIGNED_SDK_TOOLS 0 SIGNED_SDK_TOOL)
    add_test(NAME UpdateSignatureTests COMMAND UpdateSignatureTests "${SIGNED_SDK_TOOL}")

    add_executable(UpdateCleanupTests UpdateCleanupTests.cpp
        ../win32/EasyKey/EasyKey/UpdateCleanup.cpp)
    target_compile_options(UpdateCleanupTests PRIVATE /utf-8)
    add_test(NAME UpdateCleanupTests COMMAND UpdateCleanupTests)

    add_executable(HttpBodyTests HttpBodyTests.cpp
        ../win32/EasyKey/EasyKey/HttpBody.cpp)
    target_compile_options(HttpBodyTests PRIVATE /utf-8)
    add_test(NAME HttpBodyTests COMMAND HttpBodyTests)
endif()
