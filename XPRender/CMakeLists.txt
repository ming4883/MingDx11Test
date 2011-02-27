CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( xprender )

#FILE( GLOB FREEGLUT lib/freeglut/*.c lib/freeglut/*.h )
FILE( GLOB GLEW lib/glew/*.c lib/glew/*.h )
FILE( GLOB GLSW lib/glsw/*.c lib/glsw/*.h )
FILE( GLOB HTTPD lib/httpd/*.c lib/httpd/*.h )

#ADD_DEFINITIONS( -DFREEGLUT_STATIC )
#ADD_DEFINITIONS( -DFREEGLUT_LIB_PRAGMAS=0 )
ADD_DEFINITIONS( -DGLEW_STATIC )

IF( WIN32 )

	#FILE( GLOB XPRENDER lib/xprender/*.c lib/xprender/*.h )
	SET( XPRENDER
		lib/xprender/API.gl.h
		lib/xprender/Buffer.h
		lib/xprender/Buffer.gl.c
		lib/xprender/Buffer.gl.h
		lib/xprender/Framework.h
		lib/xprender/Framework.windows.gl.c
		lib/xprender/GpuState.h
		lib/xprender/GpuState.gl.c
		lib/xprender/GpuState.gl.h
		lib/xprender/Mat44.c
		lib/xprender/Mat44.h
		lib/xprender/NvpParser.c
		lib/xprender/NvpParser.h
		lib/xprender/Platform.c
		lib/xprender/Platform.h
		lib/xprender/pstdint.h
		lib/xprender/RenderTarget.h
		lib/xprender/RenderTarget.gl.c
		lib/xprender/RenderTarget.gl.h
		lib/xprender/Shader.h
		lib/xprender/Shader.gl.c
		lib/xprender/Shader.gl.h
		lib/xprender/StrHash.c
		lib/xprender/StrHash.h
		lib/xprender/StrHashMacro.h
		lib/xprender/StrUtil.h
		lib/xprender/Texture.h
		lib/xprender/Texture.format.h
		lib/xprender/Texture.gl.c
		lib/xprender/Texture.gl.h
		lib/xprender/Vec2.c
		lib/xprender/Vec2.h
		lib/xprender/Vec3.c
		lib/xprender/Vec3.h
		lib/xprender/Vec4.c
		lib/xprender/Vec4.h
		)
		
	SET( XPRENDER_D3D9
		lib/xprender/API.d3d9.h
		lib/xprender/Buffer.h
		lib/xprender/Buffer.d3d9.c
		lib/xprender/Buffer.d3d9.h
		lib/xprender/Framework.h
		lib/xprender/Framework.windows.d3d9.c
		lib/xprender/GpuState.h
		lib/xprender/GpuState.d3d9.c
		lib/xprender/GpuState.d3d9.h
		lib/xprender/Mat44.c
		lib/xprender/Mat44.h
		lib/xprender/NvpParser.c
		lib/xprender/NvpParser.h
		lib/xprender/Platform.c
		lib/xprender/Platform.h
		lib/xprender/pstdint.h
		lib/xprender/RenderTarget.h
		lib/xprender/RenderTarget.d3d9.c
		lib/xprender/RenderTarget.d3d9.h
		lib/xprender/Shader.h
		lib/xprender/Shader.d3d9.c
		lib/xprender/Shader.d3d9.h
		lib/xprender/StrHash.c
		lib/xprender/StrHash.h
		lib/xprender/StrHashMacro.h
		lib/xprender/StrUtil.h
		lib/xprender/Texture.h
		lib/xprender/Texture.format.h
		lib/xprender/Texture.d3d9.c
		lib/xprender/Texture.d3d9.h
		lib/xprender/Vec2.c
		lib/xprender/Vec2.h
		lib/xprender/Vec3.c
		lib/xprender/Vec3.h
		lib/xprender/Vec4.c
		lib/xprender/Vec4.h
		)
	
	SET( CONSOLE_SYSTEM WIN32 )
	
	ADD_DEFINITIONS( -DXPR_WIN32 )

	IF( CYGWIN )
		#ADD_DEFINITIONS( -DTIME_WITH_SYS_TIME )
		ADD_DEFINITIONS( -DX_DISPLAY_MISSING )
		ADD_DEFINITIONS( -D_WIN32 )
		#ADD_DEFINITIONS( -std=c99 )
		#SET( PLATFORM_LIBS GL GLU X11 )
		SET( PLATFORM_LIBS opengl32 glu32 winmm ws2_32 gdi32 )
	
	ELSE()
		# assume MSVC
		ADD_DEFINITIONS( /wd4996 )
		SOURCE_GROUP( "glew" FILES ${GLEW} )
		SOURCE_GROUP( "glsw" FILES ${GLSW} )
		SOURCE_GROUP( "httpd" FILES ${HTTPD} )
		SOURCE_GROUP( "xprender" FILES ${XPRENDER} )
		SOURCE_GROUP( "xprender" FILES ${XPRENDER_D3D9} )
		SOURCE_GROUP( "pez" FILES ${PEZ} )
		SET( PLATFORM_LIBS opengl32 glu32 winmm ws2_32 )
		
	ENDIF()
	
ELSEIF( UNIX )

	FILE( GLOB XPRENDER xprender/*.c xprender/*.h )
	
	SET( PLATFORM_LIBS GL GLU X11 )
	
	ADD_DEFINITIONS( -DXPR_LINUX )

ENDIF()

INCLUDE_DIRECTORIES(
	lib/glew
	lib/glsw
	lib/httpd
)

ADD_LIBRARY( xprender
	${GLEW}
	${GLSW}
	${HTTPD}
	${XPRENDER}
	)

# UNIT test_xprender
ADD_EXECUTABLE( test_xprender
	test/TestXPRender.c
	lib/cutest/CuTest.c
	lib/cutest/CuTest.h
	lib/cutest/README.txt
	)
	
TARGET_LINK_LIBRARIES( test_xprender xprender ${PLATFORM_LIBS} )

# example_cloth_simulation
ADD_EXECUTABLE( example_cloth_simulation ${CONSOLE_SYSTEM}
	example/ClothSimulation.example.c
	example/ClothSimulation.glsl
	example/Common.c
	example/Common.h
	example/Common.glsl
	example/Stream.h
	example/Stream.c
	example/Remote.c
	example/Remote.h
	example/Remote.impl.h
	example/Remote.impl.windows.c
	example/Remote.impl.windows.h
	example/Mesh.c
	example/Mesh.h
	example/Material.c
	example/Material.h
	example/Cloth.c
	example/Cloth.h
	example/Sphere.c
	example/Sphere.h
	example/Pvr.c
	example/Pvr.h
	)
TARGET_LINK_LIBRARIES( example_cloth_simulation xprender ${PLATFORM_LIBS} )

# example_label
ADD_EXECUTABLE( example_label ${CONSOLE_SYSTEM}
	example/Label.example.c
	example/Common.c
	example/Common.h
	example/Common.glsl
	example/Stream.h
	example/Stream.c
	example/Label.c
	example/Label.h
	example/Mesh.c
	example/Mesh.h
	example/Material.c
	example/Material.h
	)
TARGET_LINK_LIBRARIES( example_label xprender ${PLATFORM_LIBS} )

# example_triangle_tessellation
ADD_EXECUTABLE( example_triangle_tessellation ${CONSOLE_SYSTEM}
	example/TriangleTessellation.example.c
	example/TriangleTessellation.glsl
	example/Common.c
	example/Common.h
	example/Common.glsl
	example/Stream.h
	example/Stream.c
	example/Remote.c
	example/Remote.h
	example/Remote.impl.h
	example/Remote.impl.windows.c
	example/Remote.impl.windows.h
	example/Mesh.c
	example/Mesh.obj.c
	example/Mesh.h
	example/Material.c
	example/Material.h
	)
TARGET_LINK_LIBRARIES( example_triangle_tessellation xprender ${PLATFORM_LIBS} )

# example_embedded_httpd
ADD_EXECUTABLE( example_embedded_httpd ${CONSOLE_SYSTEM}
	example/EmbeddedHttpd.example.c
	example/Common.c
	example/Common.h
	example/Common.glsl
	example/Stream.h
	example/Stream.c
	example/Remote.c
	example/Remote.h
	example/Remote.impl.h
	example/Remote.impl.windows.c
	example/Remote.impl.windows.h
	example/Material.c
	example/Material.h
	)
TARGET_LINK_LIBRARIES( example_embedded_httpd xprender ${PLATFORM_LIBS} )

# example_android
ADD_EXECUTABLE( example_android ${CONSOLE_SYSTEM}
	example/Android.example.c
	example/Android.glsl
	example/Common.c
	example/Common.h
	example/Common.glsl
	example/Remote.c
	example/Remote.h
	example/Remote.impl.h
	example/Remote.impl.windows.c
	example/Remote.impl.windows.h
	example/Stream.c
	example/Stream.h
	example/Mesh.c
	example/Mesh.obj.c
	example/Mesh.h
	example/Material.c
	example/Material.h
	)
TARGET_LINK_LIBRARIES( example_android xprender ${PLATFORM_LIBS} )

# MosegaardsClothTutorial
#ADD_EXECUTABLE( MosegaardsClothTutorial ${CONSOLE_SYSTEM}
#	example/MosegaardsClothTutorial.cpp
#	)
#TARGET_LINK_LIBRARIES( MosegaardsClothTutorial xprender ${PLATFORM_LIBS} )


IF( WIN32 )
ADD_LIBRARY( xprender-d3d9
	${GLSW}
	${HTTPD}
	${XPRENDER_D3D9}
	)
	
ADD_EXECUTABLE( example_embedded_httpd-d3d9 ${CONSOLE_SYSTEM}
	example/EmbeddedHttpd.example.c
	example/Common.c
	example/Common.h
	example/Common.glsl
	example/Stream.h
	example/Stream.c
	example/Remote.c
	example/Remote.h
	example/Remote.impl.h
	example/Remote.impl.windows.c
	example/Remote.impl.windows.h
	example/Material.c
	example/Material.h
	)
TARGET_LINK_LIBRARIES( example_embedded_httpd-d3d9 xprender-d3d9 ${PLATFORM_LIBS} d3d9 )

ENDIF()
