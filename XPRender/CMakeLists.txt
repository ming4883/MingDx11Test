CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( xprender )


FILE( GLOB FREEGLUT lib/freeglut/*.c lib/freeglut/*.h )

IF( WIN32 )

    FILE( GLOB XPRENDER lib/xprender/*.c lib/xprender/*.h )

    SOURCE_GROUP( "freeglut" FILES ${FREEGLUT} )
	SOURCE_GROUP( "xprender" FILES ${XPRENDER} )

    ADD_DEFINITIONS( -DFREEGLUT_STATIC )
    ADD_DEFINITIONS( -DFREEGLUT_LIB_PRAGMAS=0 )
    #ADD_DEFINITIONS( -DGLEW_STATIC )
    #ADD_DEFINITIONS( -DOPENCTM_STATIC )
    ADD_DEFINITIONS( /wd4996 )
    SET( PLATFORM_LIBS opengl32 glu32 winmm )
    #SET( CONSOLE_SYSTEM WIN32 )

ELSEIF( UNIX )

    FILE( GLOB XPRENDER xprender/*.c xprender/*.h )
	
    SET( PLATFORM_LIBS X11 GL )

ENDIF()

INCLUDE_DIRECTORIES(
    lib/freeglut
)

ADD_LIBRARY( xprender
    ${FREEGLUT}
    ${XPRENDER}
	)

# ANSI C SAMPLES:

ADD_EXECUTABLE( example_cloth_simulation ${CONSOLE_SYSTEM}
	example/ClothSimulation.c
	example/Cloth.c
	example/Cloth.h
	)
TARGET_LINK_LIBRARIES( example_cloth_simulation xprender ${PLATFORM_LIBS} )

ADD_EXECUTABLE( MosegaardsClothTutorial ${CONSOLE_SYSTEM}
	example/MosegaardsClothTutorial.cpp
	)
TARGET_LINK_LIBRARIES( MosegaardsClothTutorial xprender ${PLATFORM_LIBS} )
