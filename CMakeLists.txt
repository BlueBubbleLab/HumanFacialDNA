PROJECT(CROWDSIGHT_DEMO)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#-------------------------------------------------------------------------------
# General Settings
#-------------------------------------------------------------------------------
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_MODULE_PATH "${CROWDSIGHT_DEMO_SOURCE_DIR}/cmake/modules")

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
    SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
    SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DDEBUG")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#-------------------------------------------------------------------------------
# Platform specific settings Settings
#-------------------------------------------------------------------------------
IF (APPLE)
    IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        MESSAGE(STATUS " Compiling for OSX")
        SET(OSX ON)
    ELSE ()
        MESSAGE(STATUS " Compiling for IOS")
        SET(IOS ON)
    ENDIF ()
ENDIF ()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    MESSAGE(STATUS " Compiling for Linux")
    SET(LINUX ON)
ENDIF()

IF (MINGW)
    MESSAGE(STATUS " Compiling for Windows with MinGW")
    SET(COMPILER "mingw")
ENDIF ()

IF (MSVC)
    MESSAGE(STATUS " Compiling for Windows with Visual Studio")
    SET(COMPILER "vs2010")
ENDIF ()


#-------------------------------------------------------------------------------
# Extract and set the version
#-------------------------------------------------------------------------------
SET(PRODUCT "CrowdSightDemo")

FIND_PACKAGE(Git REQUIRED)

EXECUTE_PROCESS(
    COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
    OUTPUT_VARIABLE GIT_VERSION
    ERROR_VARIABLE GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

IF(GIT_VERSION)
    SET(VERSION ${GIT_VERSION})
ELSE(NOT GIT_VERSION)
    SET(MAJOR_VERSION "10")
    SET(MINOR_VERSION "00")
    SET(VERSION "${MAJOR_VERSION}.${MINOR_VERSION}")
    MESSAGE(STATUS " git tag not usable, using ${VERSION} as VERSION")
ENDIF(GIT_VERSION)

EXECUTE_PROCESS(
    COMMAND ${GIT_EXECUTABLE} log --pretty=format:%h -n 1
    OUTPUT_VARIABLE COMMIT
    ERROR_VARIABLE GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

CONFIGURE_FILE(
    "${CROWDSIGHT_DEMO_SOURCE_DIR}/cmake/demo_version.h.in"
    "${CROWDSIGHT_DEMO_BINARY_DIR}/demo_version.h"
)

#-------------------------------------------------------------------------------
# Find 3rd party packages and include macros
#-------------------------------------------------------------------------------
IF (WIN32)
    SET(WINBIN "${CMAKE_CURRENT_SOURCE_DIR}/../winbin" CACHE PATH
      "Location of the WINBIN git repository")
    
    IF(NOT EXISTS ${WINBIN})
      MESSAGE(ERROR
        "${WINBIN} doesn't exists, please set WINBIN to ThirdSight WINBIN project")
    ELSE(NOT EXISTS ${WINBIN})
        SET(CMAKE_MODULE_PATH "${WINBIN}/cmake/")
        INCLUDE(winbin)
    ENDIF(NOT EXISTS ${WINBIN})
    
    LINK_DIRECTORIES(
        ${OpenCV_LIB_DIR}
	    ${CURL_LIB_DIR}
	)
ELSE (WIN32)
    FIND_PACKAGE(OpenCV REQUIRED)
ENDIF (WIN32)

#-------------------------------------------------------------------------------
# Include sources
#-------------------------------------------------------------------------------
INCLUDE(cmake/sources.cmake)
INCLUDE_DIRECTORIES(${CROWDSIGHT_DEMO_BINARY_DIR})

#-------------------------------------------------------------------------------
# Add InSight subproject
#-------------------------------------------------------------------------------
SET(CROWDSIGHT_STANDALONE TRUE) #Don't pass product id to crowdsight
ADD_SUBDIRECTORY(${CROWDSIGHT_DEMO_SOURCE_DIR}/CrowdSight crowdsight)
INCLUDE_DIRECTORIES(${CROWDSIGHT_SOURCE_DIR}/src)
LINK_DIRECTORIES(${CROWDSIGHT_BINARY_DIR})
FILE(COPY ${CROWDSIGHT_SOURCE_DIR}/data DESTINATION ${CROWDSIGHT_DEMO_BINARY_DIR})

#-------------------------------------------------------------------------------
# Include 3rd party library headers
#-------------------------------------------------------------------------------
INCLUDE_DIRECTORIES(
    ${OpenCV_INCLUDE_DIR}
)

#-------------------------------------------------------------------------------
# Add executables and link libraries
#-------------------------------------------------------------------------------
IF(OSX)
    ADD_EXECUTABLE(${TARGET_GAZE_DEMO} MACOSX_BUNDLE ${GAZE_DEMO_SOURCE})
ELSE(OSX)
    ADD_EXECUTABLE(${TARGET_GAZE_DEMO} ${GAZE_DEMO_SOURCE})
ENDIF(OSX)
TARGET_LINK_LIBRARIES(${TARGET_GAZE_DEMO}
    crowdsight
    ${OpenCV_LIBS}
)

#-------------------------------------------------------------------------------
# Windows post build
#-------------------------------------------------------------------------------

IF(WIN32)
    ADD_CUSTOM_COMMAND(
        TARGET ${TARGET_GAZE_DEMO}
        POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${CROWDSIGHT_DEMO_BINARY_DIR}/crowdsight/libcrowdsight.dll
                ${CROWDSIGHT_DEMO_BINARY_DIR}
    )
ENDIF()

#-------------------------------------------------------------------------------
# Windows packaging
#-------------------------------------------------------------------------------

IF(WIN32)
    INSTALL(TARGETS ${TARGET_GAZE_DEMO} DESTINATION .)

    INSTALL(FILES ${OpenCV_DLL_FILES} DESTINATION .)
    INSTALL(FILES ${CURL_DLL_FILES} DESTINATION .)
    INSTALL(DIRECTORY ${CROWDSIGHT_DEMOS_SOURCE_DIR}resources DESTINATION .)
    IF (MINGW)
        INSTALL(FILES ${MINGW_DLL_FILES} DESTINATION .)
    ENDIF (MINGW)
	
    # hack to install libstdc++-6.dll
    find_file(LIBSTDC libstdc++-6.dll)
    INSTALL(FILES ${LIBSTDC} DESTINATION .)
ENDIF()

#-------------------------------------------------------------------------------
# OSX devel example bundle
#-------------------------------------------------------------------------------
IF (OSX)
    INSTALL(TARGETS ${TARGET_GAZE_DEMO} BUNDLE DESTINATION . COMPONENT Runtime)

    # Install resources and set icon
    INSTALL(FILES "${CROWDSIGHT_DEMO_SOURCE_DIR}/resources/icons/thirdsight.icns" DESTINATION ${CMAKE_INSTALL_PREFIX}/${TARGET_GAZE_DEMO}.app/Contents/Resources COMPONENT Runtime)
    INSTALL(DIRECTORY "${CROWDSIGHT_SOURCE_DIR}/data" DESTINATION ${CMAKE_INSTALL_PREFIX}/${TARGET_GAZE_DEMO}.app/Contents/Resources COMPONENT Runtime)

    # Set aliases for plugin and resource dir
    SET(plugin_dest_dir ${TARGET_GAZE_DEMO}.app/Contents/MacOS)

    # Set crowdsight dependency directory
    SET(DIRS ${CMAKE_CURRENT_BINARY_DIR}/crowdsight/)


    INSTALL(CODE "
file(GLOB_RECURSE QTPLUGINS
\"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
            include(BundleUtilities)
            SET(BU_CHMOD_BUNDLE_ITEMS ON)
            fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${TARGET_GAZE_DEMO}.app\" \"\${QTPLUGINS}\" \"${DIRS}\")
            " COMPONENT Runtime)

SET(MACOSX_BUNDLE_ICON_FILE thirdsight.icns)
SET(MACOSX_BUNDLE_BUNDLE_NAME "${TARGET_GAZE_DEMO}")
SET(MACOSX_BUNDLE_INFO_STRING "${TARGET_GAZE_DEMO} "${VERSION})
SET(MACOSX_BUNDLE_GUI_IDENTIFIER "com.ThirdSight.${TARGET_GAZE_DEMO}" )
SET(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION})
SET(MACOSX_BUNDLE_LONG_VERSION_STRING ${VERSION})
SET(MACOSX_BUNDLE_COPYRIGHT "Copyright 2011 ThirdSight B.V. All Rights Reserved")

ENDIF(OSX)
