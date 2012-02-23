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
    #${GAZE_DEMO_QT_UI}
    #${GAZE_DEMO_QT_CPP}
    src/gaze.cpp
)

#-------------------------------------------------------------------------------
# Emotion executable related sources
#-------------------------------------------------------------------------------
