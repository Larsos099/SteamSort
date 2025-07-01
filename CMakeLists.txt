cmake_minimum_required(VERSION 3.19)
project(SteamSorter LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
set(CMAKE_CXX_STANDARD 23)
find_package(CURL REQUIRED)
qt_standard_project_setup()
qt_add_executable(SteamSorter
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    jsonhandler.hpp jsonhandler.cpp
    json.hpp
    AcfParser.hpp
    HttpRequest.hpp
    process.hpp
    process.cpp
    process.cpp
)

target_link_libraries(SteamSorter
    PRIVATE
        Qt::Core
        Qt::Widgets
        CURL::libcurl
)


include(GNUInstallDirs)

install(TARGETS SteamSorter
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET SteamSorter
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
