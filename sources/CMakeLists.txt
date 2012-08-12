#CMakeLists.txt

#SET (MOC_DIR GeneratedFiles)
#SET (OBJECTS_DIR GeneratedFiles)
#SET (UI_DIR GeneratedFiles)

set(MAIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cconfigure.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/cprofileinfo.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/ctidyup.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/qprofwidget.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/parseprofile.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/aboutform.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/colorform.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/parseprofile_gprof.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/clientsidemap.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/cprofileviewitem.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/dotCallGraph.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/axisbase.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/barchartplotter.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/plotterbase.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/parseprofile_fnccheck.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/parseprofile_pose.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/parseprofile_callgrind.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/parseprofile_oprofile.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/parseprofile_perf.cpp
		    ${CMAKE_CURRENT_SOURCE_DIR}/parseprofile_sleepy.cpp
                PARENT_SCOPE)


set(MAIN_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/includes/cconfigure.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/constants.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/cprofileinfo.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/ctidyup.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/qprofwidget.h
		${CMAKE_CURRENT_SOURCE_DIR}/includes/aboutform.h
		${CMAKE_CURRENT_SOURCE_DIR}/includes/colorform.h
		${CMAKE_CURRENT_SOURCE_DIR}/includes/parseprofile_gprof.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/parseprofile_pose.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/parseprofile_callgrind.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/parseprofile_oprofile.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/parseprofile_sleepy.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/parseprofile_perf.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/axisbase.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/barchartplotter.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/plotterbase.h
		${CMAKE_CURRENT_SOURCE_DIR}/includes/clientsidemap.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/cprofileviewitem.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/dotCallGraph.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/Log.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/parseprofile_fnccheck.h
                ${CMAKE_CURRENT_SOURCE_DIR}/includes/parseprofile.h
                PARENT_SCOPE)


set(MAIN_FORMS  ${CMAKE_CURRENT_SOURCE_DIR}/forms/mainForm.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/call-graph.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/colors.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/aboutForm.ui
                PARENT_SCOPE)

#QT4_WRAP_CPP(MAIN_HEADERS_MOC ${MAIN_HEADERS})
#QT4_WRAP_UI(MAIN_FORMS_HEADERS ${MAIN_FORMS})

# cleanup $build/*.cpp~ on "make clean"
SET_DIRECTORY_PROPERTIES(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*~")


add_subdirectory(forms)