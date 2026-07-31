find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Network)

qt_standard_project_setup()

qt_add_executable(bc2-wallet-simulator
    WIN32
    MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow_pages.cpp
    src/mainwindow.h
    src/appsettings.cpp
    src/appsettings.h
    src/designtokens.h
    src/thememanager.cpp
    src/thememanager.h
    src/pagerouter.cpp
    src/pagerouter.h
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
    src/components/bc2button.cpp
    src/components/bc2button.h
    src/components/bc2card.cpp
    src/components/bc2card.h
    src/components/bc2dialog.cpp
    src/components/bc2dialog.h
    src/components/bc2header.cpp
    src/components/bc2header.h
    src/components/bc2loadingwidget.cpp
    src/components/bc2loadingwidget.h
    src/components/bc2qrwidget.cpp
    src/components/bc2qrwidget.h
    src/components/bc2statusbar.cpp
    src/components/bc2statusbar.h
    src/pages/walletpages.cpp
    src/pages/walletpages.h
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
    target_compile_options(bc2-wallet-simulator PRIVATE /W4)
else()
    target_compile_options(bc2-wallet-simulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(bc2-wallet-simulator PROPERTIES
    OUTPUT_NAME "BC2 Cold Wallet"
    MACOSX_BUNDLE_BUNDLE_NAME "BC2 Cold Wallet"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.bitcoin-ii.bc2coldwallet"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

install(TARGETS bc2-wallet-simulator
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

if(WIN32 OR APPLE)
    qt_generate_deploy_app_script(
        TARGET bc2-wallet-simulator
        OUTPUT_SCRIPT bc2_deploy_script
    )
    install(SCRIPT ${bc2_deploy_script})
endif()
