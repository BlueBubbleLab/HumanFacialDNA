PROJECT(INSIGHT_DEMO)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#-------------------------------------------------------------------------------
# General Settings
#-------------------------------------------------------------------------------
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_MODULE_PATH "${INSIGHT_DEMO_SOURCE_DIR}/cmake/modules")
SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DDEBUG")

#-------------------------------------------------------------------------------
# Extract and set the version
#-------------------------------------------------------------------------------
EXECUTE_PROCESS(
    COMMAND git describe --tags
    OUTPUT_VARIABLE GIT_VERSION
    ERROR_VARIABLE GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

IF(GIT_VERSION)
    SET(VERSION ${GIT_VERSION})
ELSE(NOT GIT_VERSION)
    SET(VERSION "1.0")
ENDIF(GIT_VERSION)

CONFIGURE_FILE(
    "${INSIGHT_DEMO_SOURCE_DIR}/cmake/demo_version.h.in"
    "${INSIGHT_DEMO_BINARY_DIR}/demo_version.h"
)

#-------------------------------------------------------------------------------
# Find 3rd party packages and include macros
#-------------------------------------------------------------------------------
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(QWT REQUIRED)

#-------------------------------------------------------------------------------
# Include sources
#-------------------------------------------------------------------------------
INCLUDE(cmake/sources.cmake)
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(${INSIGHT_DEMO_BINARY_DIR})

#-------------------------------------------------------------------------------
# Add InSight subproject
#-------------------------------------------------------------------------------
SET(INSIGHT_STANDALONE FALSE)
ADD_SUBDIRECTORY(${INSIGHT_DEMO_SOURCE_DIR}/inSight insight)
INCLUDE_DIRECTORIES(${INSIGHT_SOURCE_DIR}/src)
LINK_DIRECTORIES(${INSIGHT_BINARY_DIR})
FILE(COPY ${INSIGHT_SOURCE_DIR}/data DESTINATION ${INSIGHT_DEMO_BINARY_DIR})

#-------------------------------------------------------------------------------
# Include 3rd party library headers
#-------------------------------------------------------------------------------
INCLUDE_DIRECTORIES(
    ${QT_INCLUDE_DIR}
    ${QWT_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIR}
)

#-------------------------------------------------------------------------------
# Add executables and link libraries
#-------------------------------------------------------------------------------
ADD_EXECUTABLE(${TARGET_GAZE_DEMO} ${GAZE_DEMO_SOURCE})
TARGET_LINK_LIBRARIES(${TARGET_GAZE_DEMO}
    insight
    ${QT_LIBRARIES}
    ${QT_QTMAIN_LIBRARY}
    ${OpenCV_LIBS}
    ${QWT_LIBRARIES}
)
