cmake_minimum_required(VERSION 3.20)

project(
    bc2_cold_wallet
    VERSION 0.17.3
    LANGUAGES C CXX
)

option(BC2_BUILD_SIMULATOR "Build the Qt 6 desktop simulator" OFF)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto)

add_library(
    bc2_wallet_core
    firmware/crypto/bc2_crypto.c
    firmware/encoding/bc2_encoding.c
    firmware/platform/bc2_secure.c
    firmware/platform/bc2_device_state.c
    firmware/platform/bc2_hal.c
    firmware/platform/bc2_device_ui.c
    firmware/platform/bc2_device_flow.c
    firmware/platform/bc2_device_service.c
    firmware/protocol/bc2_usb_protocol.c
    firmware/wallet/bc2_address.c
    firmware/wallet/bc2_bip32.c
    firmware/wallet/bc2_bip39.c
    firmware/wallet/bc2_network.c
    firmware/wallet/bc2_psbt.c
    firmware/wallet/bc2_sign.c
    firmware/wallet/bc2_wallet.c
)

target_include_directories(
    bc2_wallet_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/firmware/include
)

target_link_libraries(bc2_wallet_core PUBLIC OpenSSL::Crypto)

target_compile_definitions(
    bc2_wallet_core
    PRIVATE
        _POSIX_C_SOURCE=200809L
        BC2_BIP39_WORDLIST_PATH="${CMAKE_CURRENT_SOURCE_DIR}/third_party/bip39/english.txt"
)

if(MSVC)
    target_compile_options(bc2_wallet_core PRIVATE /W4 /WX)
else()
    target_compile_options(
        bc2_wallet_core
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            -Wno-deprecated-declarations
    )
endif()

add_executable(bc2-wallet-cli firmware/app/main.c)
target_link_libraries(bc2-wallet-cli PRIVATE bc2_wallet_core)

if(MSVC)
    target_compile_options(bc2-wallet-cli PRIVATE /W4 /WX)
else()
    target_compile_options(
        bc2-wallet-cli
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            -Wno-deprecated-declarations
    )
endif()

enable_testing()

foreach(test_name crypto encoding bip39 bip32 address sign wallet device_state device_flow device_service psbt hal usb_protocol)
    add_executable(test_${test_name} tests/test_${test_name}.c)
    target_link_libraries(test_${test_name} PRIVATE bc2_wallet_core)

    if(MSVC)
        target_compile_options(test_${test_name} PRIVATE /W4 /WX)
    else()
        target_compile_options(
            test_${test_name}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Werror
                -Wno-deprecated-declarations
                -UNDEBUG
        )
    endif()

    add_test(NAME ${test_name} COMMAND test_${test_name})
endforeach()

if(BC2_BUILD_SIMULATOR)
    add_subdirectory(simulator)
endif()

# Desktop release packaging
set(CPACK_PACKAGE_NAME "BC2-Cold-Wallet")
set(CPACK_PACKAGE_VENDOR "BC2")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "bc2-cold-wallet-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_CHECKSUM SHA256)
include(CPack)
