cmake_minimum_required(VERSION 3.19)
# One version, in the spec: the RPM, the About page and the release tag all follow it
# (tools/check_version.sh holds the tag to it).
file(STRINGS rpm/harbour-ourasync.spec APP_VERSION_LINE REGEX "^Version:")
string(REGEX REPLACE "^Version:[ \t]*" "" APP_VERSION "${APP_VERSION_LINE}")
project(harbour-ourasync VERSION ${APP_VERSION} LANGUAGES CXX)

# std::optional in the Qt-free core (src/ring, src/analysis) needs C++17. The
# core is compiled and tested with plain g++ as well; see tests/README.md.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# No "Bluetooth" Qt component: Sailfish OS 5.1's SDK target has no
# pkgconfig(Qt5Bluetooth) at all (QtConnectivity is not part of its Qt5), so the
# ring is reached through BlueZ's D-Bus API (org.bluez.*) with Qt5::DBus, the
# way harbour-suuntosync does it.
find_package(Qt5 COMPONENTS Core Sql DBus Network Qml Gui Quick Concurrent LinguistTools REQUIRED)
find_package(ZLIB REQUIRED)

include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp REQUIRED)
pkg_search_module(SAILFISHSECRETS sailfishsecrets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# The Qt-free core: protocol, decoders, clock, analysis. Everything in here is
# built and tested with plain g++ in CI, so a bug in it never needs a phone.
set(CORE_SOURCES
    src/ring/aes128.cpp
    src/ring/protocol.cpp
    src/ring/events.cpp
    src/ring/session.cpp
    src/analysis/ringclock.cpp
    src/analysis/sleepscore.cpp
    src/analysis/daysummary.cpp
    src/analysis/oura_sleep_score.cpp
    src/analysis/oura_sleep_state.cpp
    src/analysis/stager.cpp
    src/analysis/breathing.cpp
    src/analysis/activity_score.cpp
    src/analysis/readiness_estimate.cpp
    src/analysis/oura_night_score.cpp
    src/analysis/cloudmerge.cpp
    src/export/healthexport.cpp
    src/cloud/json.cpp
    src/cloud/csv.cpp
    src/cloud/zip.cpp
    src/cloud/clouddoc.cpp
)

add_executable(harbour-ourasync
    src/harbour-ourasync.cpp
    src/controller/appcontroller.cpp
    src/ble/bluezmanagedobjects.cpp
    src/ble/ringscanner.cpp
    src/ble/ringlink.cpp
    src/store/ringstore.cpp
    src/store/cloudstore.cpp
    src/cloud/exportimport.cpp
    src/cloud/ouraclient.cpp
    src/secrets/keyvault.cpp
    ${CORE_SOURCES}
)

target_compile_definitions(harbour-ourasync PRIVATE
    APP_VERSION="${APP_VERSION}"
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_include_directories(harbour-ourasync PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    $<BUILD_INTERFACE:
    ${SAILFISH_INCLUDE_DIRS}
    ${SAILFISHSECRETS_INCLUDE_DIRS}
>)

target_link_libraries(harbour-ourasync
    ZLIB::ZLIB
    Qt5::Network
    Qt5::Quick
    Qt5::Sql
    Qt5::Concurrent
    Qt5::DBus
    ${SAILFISH_LDFLAGS}
    ${SAILFISHSECRETS_LDFLAGS}
)

install(TARGETS harbour-ourasync RUNTIME DESTINATION bin)
install(DIRECTORY qml DESTINATION share/harbour-ourasync)
install(FILES harbour-ourasync.desktop DESTINATION share/applications)
foreach(size 86x86 108x108 128x128 172x172)
    install(FILES icons/${size}/harbour-ourasync.png
        DESTINATION share/icons/hicolor/${size}/apps)
endforeach()

# Source strings (qsTr()/tr()) are English. translations/harbour-ourasync.ts
# tracks them as the extraction base and each harbour-ourasync-<lang>.ts is a
# translated copy. qt5_add_translation only compiles .ts to .qm; a normal build
# never touches the committed .ts files (run lupdate by hand, see README).
file(GLOB TS_FILES "${CMAKE_SOURCE_DIR}/translations/*.ts")
file(GLOB LANG_TS_FILES "${CMAKE_SOURCE_DIR}/translations/harbour-ourasync-*.ts")
if(LANG_TS_FILES)
    qt5_add_translation(QM_FILES ${LANG_TS_FILES})
    add_custom_target(translations ALL DEPENDS ${QM_FILES})
    add_dependencies(harbour-ourasync translations)
    install(FILES ${QM_FILES} DESTINATION share/harbour-ourasync/translations)
endif()

# Get the other files reachable from the project tree in Qt Creator.
file(GLOB_RECURSE QML_FILES "${CMAKE_SOURCE_DIR}/qml/*.qml" "${CMAKE_SOURCE_DIR}/qml/*.js")
add_custom_target(distfiles
    SOURCES
        harbour-ourasync.desktop
        rpm/harbour-ourasync.spec
        ${QML_FILES}
        ${TS_FILES})

# Tell Qt Creator where the application executable would be located on the device.
file(WRITE "${CMAKE_BINARY_DIR}/QtCreatorDeployment.txt"
    "${CMAKE_INSTALL_PREFIX}
${CMAKE_BINARY_DIR}/harbour-ourasync:bin
")
