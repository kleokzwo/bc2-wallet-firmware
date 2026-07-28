cmake_minimum_required(VERSION 3.20)

project(
    bc2_wallet_firmware
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(
    bc2_wallet_core
    firmware/wallet/bc2_network.c
    firmware/wallet/bc2_derivation.c
    firmware/crypto/bc2_sha256.c
    firmware/crypto/bc2_hash.c
    firmware/crypto/bc2_hex.c
)

target_include_directories(
    bc2_wallet_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/firmware/include
)

target_compile_options(
    bc2_wallet_core
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

add_executable(
    bc2_wallet
    firmware/app/main.c
)

target_link_libraries(
    bc2_wallet
    PRIVATE
    bc2_wallet_core
)

enable_testing()

#
# Network Tests
#

add_executable(
    test_bc2_network
    tests/test_bc2_network.c
)

target_link_libraries(
    test_bc2_network
    PRIVATE
    bc2_wallet_core
)

add_test(
    NAME bc2_network_parameters
    COMMAND test_bc2_network
)

#
# Derivation Tests
#

add_executable(
    test_bc2_derivation
    tests/test_bc2_derivation.c
)

target_link_libraries(
    test_bc2_derivation
    PRIVATE
    bc2_wallet_core
)

add_test(
    NAME bc2_derivation_paths
    COMMAND test_bc2_derivation
)

#
# SHA-256 Tests
#

add_executable(
    test_bc2_sha256
    tests/test_bc2_sha256.c
)

target_link_libraries(
    test_bc2_sha256
    PRIVATE
    bc2_wallet_core
)

add_test(
    NAME bc2_sha256_vectors
    COMMAND test_bc2_sha256
)

#
# Hash Tests
#

add_executable(
    test_bc2_hash
    tests/test_bc2_hash.c
)

target_link_libraries(
    test_bc2_hash
    PRIVATE
    bc2_wallet_core
)

add_test(
    NAME bc2_hash_vectors
    COMMAND test_bc2_hash
)

#
# Hex Tests
#

add_executable(
    test_bc2_hex
    tests/test_bc2_hex.c
)

target_link_libraries(
    test_bc2_hex
    PRIVATE
    bc2_wallet_core
)

add_test(
    NAME bc2_hex_conversion
    COMMAND test_bc2_hex
)