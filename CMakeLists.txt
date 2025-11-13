# HALDXGame の CMake プロジェクト
cmake_minimum_required(VERSION 3.20)
# MSBuild で UTF-8 出力を有効にする
set(CMAKE_VS_GLOBALS "${CMAKE_VS_GLOBALS};Utf8Output=true")
project(HALDXGame)

if (WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
endif ()

# C++17 標準
set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(UNICODE _UNICODE)
#add_definitions(-D_SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING)
if (WIN7_SYSTEM_SUPPORT MATCHES ON)
    add_compile_definitions(_WIN32_WINNT=0x601)
endif ()

# 编译器の設定
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# MSVC デバッグ情報の設定
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif ()

# CMake のバージョンを指定
set(BUILD_DIRECTXTK_WERROR OFF CACHE BOOL "" FORCE)
set(USE_DIRECTX_MATH TRUE CACHE BOOL "" FORCE)

file(GLOB HLSL_FILES shaders/*.hlsl shaders/*.hlsli)
foreach (HLSL_FILE ${HLSL_FILES})
    get_filename_component(HLSL_FDIR ${HLSL_FILE} DIRECTORY)
    get_filename_component(HLSL_FBASENAME_WE ${HLSL_FILE} NAME_WE)
    string(CONCAT HLSL_FNAME_WE ${HLSL_FDIR} / ${HLSL_FBASENAME_WE})

    string(LENGTH ${HLSL_FBASENAME_WE} LEN_FNAME_WE)
    math(EXPR LEN_FNAME_WE "${LEN_FNAME_WE}-2")
    string(SUBSTRING ${HLSL_FBASENAME_WE} ${LEN_FNAME_WE} 2 ENTRY_POINT)
    string(TOLOWER ${ENTRY_POINT} SHADER_TYPE)

    if ("${SHADER_TYPE}" STREQUAL "vs")
        set(SHADER_TYPE "Vertex")
    elseif ("${SHADER_TYPE}" STREQUAL "hs")
        set(SHADER_TYPE "Hull")
    elseif ("${SHADER_TYPE}" STREQUAL "ds")
        set(SHADER_TYPE "Domain")
    elseif ("${SHADER_TYPE}" STREQUAL "gs")
        set(SHADER_TYPE "Geometry")
    elseif ("${SHADER_TYPE}" STREQUAL "ps")
        set(SHADER_TYPE "Pixel")
    elseif ("${SHADER_TYPE}" STREQUAL "cs")
        set(SHADER_TYPE "Compute")
    endif ()
    set_source_files_properties(${HLSL_FILE} PROPERTIES
            VS_SHADER_OBJECT_FILE_NAME shaders/${HLSL_FBASENAME_WE}.cso
            VS_SHADER_TYPE ${SHADER_TYPE}
            VS_SHADER_MODEL 5.0
            VS_SHADER_ENTRYPOINT ${ENTRY_POINT}
            VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:true>
            VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>)
endforeach ()

# 每次构建时都复制 assets 文件夹
set(ASSETS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/assets)
set(ASSETS_BINARY_DIR ${CMAKE_BINARY_DIR}/assets)

add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SOURCE_DIR} ${ASSETS_BINARY_DIR}
        COMMENT "asset ファイルを build ディレクトリにコピーする"
)

# third_party ディレクトリの追加
add_subdirectory(third_party/DirectXTK DirectXTK)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "Use dynamic CRT for JoltPhysics" FORCE)
set(PHYSICS_REPO_ROOT ${CMAKE_SOURCE_DIR}/third_party/JoltPhysics)
add_subdirectory(third_party/JoltPhysics/Build JoltBuild)

#include(third_party/JoltPhysics/Jolt/Jolt.cmake)
#include(third_party/JoltPhysics/JoltViewer/JoltViewer.cmake)
#include(third_party/JoltPhysics/PerformanceTest/PerformanceTest.cmake)
#include(third_party/JoltPhysics/TestFramework/TestFramework.cmake)
#include(third_party/JoltPhysics/Samples/Samples.cmake)
#include(third_party/JoltPhysics/UnitTests/UnitTests.cmake)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build Assimp tests" FORCE)
# 不安装、不打工具/样例/文档/测试，不把警告当错误
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)      # 你已设置
set(ASSIMP_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
# 建议静态库，避免多 DLL（按需改成 ON）
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# Windows 下建议启用内置 zlib，少外部依赖；非 Win 可用系统 zlib
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
# 如不需要 glTF-Draco 压缩，务必关闭
set(ASSIMP_BUILD_DRACO OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/assimp assimp)
add_subdirectory(third_party/ImGui ImGui)
add_subdirectory(third_party/enet enet)

#モジュールの追加
add_subdirectory(src/modules/GameKit)
add_subdirectory(src/modules/DX DX)

file(GLOB_RECURSE BASE_CPP_FILES src/base/*.cpp)
file(GLOB_RECURSE BASE_HEADER_FILES src/base/*.h)
file(GLOB_RECURSE GAMEPLAY_CPP_FILES src/gameplay/*.cpp)
file(GLOB_RECURSE GAMEPLAY_HEADER_FILES src/gameplay/*.h)
source_group("base" FILES ${BASE_CPP_FILES} ${BASE_HEADER_FILES})
source_group("gameplay" FILES ${GAMEPLAY_CPP_FILES} ${GAMEPLAY_HEADER_FILES})
source_group("shaders" FILES ${HLSL_FILES})

# HALDXGame のソースコード
add_executable(HALDXGame
        src/Main.cpp
        ${HLSL_FILES}
        ${BASE_CPP_FILES}
        ${GAMEPLAY_CPP_FILES}
)
# UTF-8 文字コードのサポート
target_compile_options(HALDXGame PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# HALDXGame のインクルードディレクトリ
target_link_libraries(HALDXGame PRIVATE DirectXTK)
target_link_libraries(HALDXGame PRIVATE Jolt)
target_link_libraries(HALDXGame PRIVATE assimp)

############## enet の設定 ##############
# 强制预包含 winsock_first.h —— 统一 winsock2 包含顺序
set(WINSOCK_FORCE_INCLUDE "${CMAKE_SOURCE_DIR}/src/base/net/winsock_first.h")

# 建一个可复用的接口库，把“先包含 winsock2”的规则挂上去
add_library(winsock_first_iface INTERFACE)
# 这些宏重复定义也没事（有 include guard），但能保证第三方没定义时仍生效
target_compile_definitions(winsock_first_iface INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX)

# MSVC 用 /FI，其它编译器用 -include（为以后做兼容）
target_compile_options(winsock_first_iface INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:/FI${WINSOCK_FORCE_INCLUDE}>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-include ${WINSOCK_FORCE_INCLUDE}>
)

# 链接 WinSock2（INTERFACE：谁连它，谁最终链接 Ws2_32）
target_link_libraries(winsock_first_iface INTERFACE Ws2_32)

############### enet End ###############

# HALDXGame のenet ライブラリ
target_link_libraries(HALDXGame
        PRIVATE
        enet           # ENet ライブラリ
        ws2_32         # ENet のソケット関連（winsock2.h）
        winmm          # ENet の時間関連（timeGetTime()）
)

# 消除弃用 API 警告
target_compile_definitions(enet PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS)

target_link_libraries(HALDXGame PRIVATE GameKit)
target_link_libraries(HALDXGame PRIVATE DX)

target_include_directories(HALDXGame PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party/DirectXTK/Inc
        ${CMAKE_SOURCE_DIR}/third_party/JoltPhysics
        ${CMAKE_SOURCE_DIR}/third_party/enet/include
)

file(GLOB_RECURSE ALL_INCLUDE_DIRS LIST_DIRECTORIES true "${PROJECT_SOURCE_DIR}/src/*")
foreach (dir ${ALL_INCLUDE_DIRS})
    if (IS_DIRECTORY ${dir})
        target_include_directories(HALDXGame PRIVATE ${dir})
    endif ()
endforeach ()

add_dependencies(HALDXGame copy_assets)
set_target_properties(HALDXGame PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")