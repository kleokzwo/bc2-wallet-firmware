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