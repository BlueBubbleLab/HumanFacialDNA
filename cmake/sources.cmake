#-------------------------------------------------------------------------------
# Gaze executable related sources
#-------------------------------------------------------------------------------
SET(TARGET_GAZE_DEMO "gaze-demo")

# Add gui forms here
#QT4_WRAP_UI(GAZE_DEMO_QT_UI
#)

# Add qt4 related header files here
#QT4_WRAP_CPP(GAZE_DEMO_QT_CPP
#)

# Add regular cpp files here
SET(GAZE_DEMO_SOURCE
    ${GAZE_DEMO_QT_UI}
    ${GAZE_DEMO_QT_CPP}
    src/gaze-demo.cpp
)

#-------------------------------------------------------------------------------
# Face executable related resources
#-------------------------------------------------------------------------------
SET(TARGET_FACE_DEMO "face-demo")

# Add gui forms here
#QT4_WRAP_UI(FACE_DEMO_QT_UI
#)

# Add qt4 related header files here
#QT4_WRAP_CPP(FACE_DEMO_QT_CPP
#)

# Add regular cpp files here
SET(FACE_DEMO_SOURCE
    ${FACE_DEMO_QT_UI}
    ${FACE_DEMO_QT_CPP}
    src/face-demo.cpp
)
