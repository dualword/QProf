#CMakeLists.txt

set(APP_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/qprofui.qrc
                PARENT_SCOPE)

# cleanup $build/*.cpp~ on "make clean"
SET_DIRECTORY_PROPERTIES(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*~")

