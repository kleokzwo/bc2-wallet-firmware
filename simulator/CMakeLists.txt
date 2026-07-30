find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Network)

qt_standard_project_setup()

qt_add_executable(bc2-wallet-simulator
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/devicecontroller.cpp
    src/devicecontroller.h
    src/electrumclient.cpp
    src/electrumclient.h
    src/watchonlymodel.cpp
    src/watchonlymodel.h
    src/watchonlysync.cpp
    src/watchonlysync.h
    src/epaperwidget.cpp
    src/epaperwidget.h
    src/simulatorhal.cpp
    src/simulatorhal.h
    src/psbtinspectordialog.cpp
    src/psbtinspectordialog.h
)

target_link_libraries(
    bc2-wallet-simulator
    PRIVATE
        Qt6::Widgets
        Qt6::Network
        bc2_wallet_core
)

target_compile_features(bc2-wallet-simulator PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(bc2-wallet-simulator PRIVATE /W4 /WX)
else()
    target_compile_options(bc2-wallet-simulator PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

install(TARGETS bc2-wallet-simulator RUNTIME DESTINATION bin BUNDLE DESTINATION .)
