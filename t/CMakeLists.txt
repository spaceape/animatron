
add_definitions (${KDE4_DEFINITIONS} ${QT_DEFINITIONS} "-Wall -fexceptions")

add_executable(test_query query.cpp)
target_link_libraries(test_query ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

