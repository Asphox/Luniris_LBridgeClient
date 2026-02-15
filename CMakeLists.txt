cmake_minimum_required(VERSION 3.16)
project(luniris_client VERSION 1.0.0 LANGUAGES C)

# =============================================================================
# Options
# =============================================================================
option(LUNIRIS_ENABLE_SECURE "Enable LBridge encryption (ChaCha20-Poly1305)" ON)

# =============================================================================
# Global settings
# =============================================================================
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# =============================================================================
# LBridge submodule
# =============================================================================
set(LBRIDGE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LBRIDGE_ENABLE_SECURE ${LUNIRIS_ENABLE_SECURE} CACHE BOOL "" FORCE)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/LBridge/CMakeLists.txt")
    add_subdirectory(external/LBridge)
else()
    message(FATAL_ERROR "LBridge not found. Run: git submodule update --init --recursive")
endif()

# =============================================================================
# nanopb runtime library
# =============================================================================
set(NANOPB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/nanopb")

if(EXISTS "${NANOPB_SRC_DIR}/pb.h")
    set(NANOPB_SRCS
        ${NANOPB_SRC_DIR}/pb_common.c
        ${NANOPB_SRC_DIR}/pb_encode.c
        ${NANOPB_SRC_DIR}/pb_decode.c
    )
    add_library(nanopb STATIC ${NANOPB_SRCS})
    target_include_directories(nanopb PUBLIC ${NANOPB_SRC_DIR})
    target_compile_features(nanopb PUBLIC c_std_11)
else()
    message(FATAL_ERROR "nanopb not found. Run: git submodule update --init --recursive")
endif()

# =============================================================================
# Client library
# =============================================================================
set(CLIENT_SOURCES
    src/luniris_client.c
    src/luniris_messages.pb.c
)

set(CLIENT_HEADERS
    src/luniris_client.h
    src/luniris_messages.pb.h
    src/luniris_rpc_ids.h
)

add_library(luniris_client STATIC ${CLIENT_SOURCES} ${CLIENT_HEADERS})

target_include_directories(luniris_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(luniris_client PUBLIC
    lbridge
    nanopb
)

target_compile_features(luniris_client PUBLIC c_std_11)

if(MSVC)
    target_compile_options(luniris_client PRIVATE /W3)
else()
    target_compile_options(luniris_client PRIVATE -Wall -Wextra)
endif()

# =============================================================================
# Installation
# =============================================================================
install(TARGETS luniris_client
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES
    src/luniris_client.h
    src/luniris_messages.pb.h
    src/luniris_rpc_ids.h
    DESTINATION include/luniris
)

# =============================================================================
# Configuration summary
# =============================================================================
message(STATUS "")
message(STATUS "Luniris Client Configuration:")
message(STATUS "  Encryption: ${LUNIRIS_ENABLE_SECURE}")
message(STATUS "")
