project(plasma-wallpaper-animatron)

find_package(KDE4 REQUIRED)
find_package(KDE4Workspace REQUIRED)
include(KDE4Defaults)

add_definitions (${KDE4_DEFINITIONS} ${QT_DEFINITIONS} "-Wall -fexceptions")

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
   ADD_DEFINITIONS("-g -O0")
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
   ADD_DEFINITIONS("-g -O2")
ENDIF()

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(kde4_prefix /usr)

# sources
set(animatron_SRCS animatron.cpp fireflies.cpp)

#uis
kde4_add_ui_files(animatron_SRCS config.ui)

kde4_add_plugin(plasma_wallpaper_animatron ${animatron_SRCS})

target_link_libraries(plasma_wallpaper_animatron
   ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}
)

install(TARGETS plasma_wallpaper_animatron
   DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-wallpaper-animatron.desktop
   DESTINATION ${SERVICES_INSTALL_DIR})

