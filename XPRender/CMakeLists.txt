CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( xprender )

FILE( GLOB GLEW lib/glew/*.c lib/glew/*.h )
FILE( GLOB GLSW lib/glsw/*.c lib/glsw/*.h )
FILE( GLOB HTTPD lib/httpd/*.c lib/httpd/*.h )

ADD_DEFINITIONS( -DGLEW_STATIC )

SET( XPRENDER_COMMON
		lib/xprender/Buffer.h
		lib/xprender/GpuFormat.h
		lib/xprender/GpuState.h
		lib/xprender/Mat44.c
		lib/xprender/Mat44.h
		lib/xprender/Memory.c
		lib/xprender/Memory.h
		lib/xprender/NvpParser.c
		lib/xprender/NvpParser.h
		lib/xprender/Platform.c
		lib/xprender/Platform.h
		lib/xprender/pstdint.h
		lib/xprender/RenderTarget.h
		lib/xprender/Shader.h
		lib/xprender/StrHash.c
		lib/xprender/StrHash.h
		lib/xprender/StrHashMacro.h
		lib/xprender/StrUtil.h
		lib/xprender/Texture.h
		lib/xprender/Vec2.c
		lib/xprender/Vec2.h
		lib/xprender/Vec3.c
		lib/xprender/Vec3.h
		lib/xprender/Vec4.c
		lib/xprender/Vec4.h
		)

IF( WIN32 )

	#FILE( GLOB XPRENDER lib/xprender/*.c lib/xprender/*.h )
	SET( XPRENDER
		${XPRENDER_COMMON}
		lib/xprender/API.gl.h
		lib/xprender/Buffer.gl.c
		lib/xprender/Buffer.gl.h
		lib/xprender/GpuState.gl.c
		lib/xprender/GpuState.gl.h
		lib/xprender/Mat44.gl.c
		lib/xprender/RenderTarget.gl.c
		lib/xprender/RenderTarget.gl.h
		lib/xprender/Shader.gl.c
		lib/xprender/Shader.gl.h
		lib/xprender/Texture.gl.c
		lib/xprender/Texture.gl.h
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
		SET( PLATFORM_LIBS opengl32 glu32 winmm ws2_32 )
		
	ENDIF()
	
ELSEIF( APPLE )

	SET( XPRENDER
		${XPRENDER_COMMON}
		lib/xprender/API.gl.h
		lib/xprender/Buffer.gl.c
		lib/xprender/Buffer.gl.h
		lib/xprender/GpuState.gl.c
		lib/xprender/GpuState.gl.h
		lib/xprender/Mat44.gl.c
		lib/xprender/RenderTarget.gl.c
		lib/xprender/RenderTarget.gl.h
		lib/xprender/Shader.gl.c
		lib/xprender/Shader.gl.h
		lib/xprender/Texture.gl.c
		lib/xprender/Texture.gl.h
		)
		
	SET( GLEW )
	
	SOURCE_GROUP( "glsw" FILES ${GLSW} )
	SOURCE_GROUP( "httpd" FILES ${HTTPD} )
	SOURCE_GROUP( "xprender" FILES ${XPRENDER} )	
		
	ADD_DEFINITIONS( -DXPR_APPLE_IOS )
	ADD_DEFINITIONS( -DTARGET_OS_IPHONE )
	
	set(APP_TYPE MACOSX_BUNDLE)
	
	SET( CMAKE_OSX_SYSROOT iphoneos )
	SET( CMAKE_OSX_ARCHITECTURES $(ARCHS_STANDARD_32_BIT) )
	SET( CMAKE_CXX_FLAGS "-x objective-c++" )
	SET( CMAKE_EXE_LINKER_FLAGS
		"-framework Foundation -framework CoreGraphics -framework QuartzCore -framework UIKit -framework OpenGLES"
		)
	
	SET(MACOSX_BUNDLE_GUI_IDENTIFIER "com.xprender.\${PRODUCT_NAME:identifier}")
	SET( CONSOLE_SYSTEM MACOSX_BUNDLE )
	
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
IF( NOT APPLE )
	ADD_EXECUTABLE( test_xprender
		test/TestXPRender.c
		lib/cutest/CuTest.c
		lib/cutest/CuTest.h
		lib/cutest/README.txt
		)

	TARGET_LINK_LIBRARIES( test_xprender xprender ${PLATFORM_LIBS} )

ENDIF()

ADD_SUBDIRECTORY( example )

#-------------------------------------------------------------------------------
# d3d9 support
IF( WIN32 )

SET( PLATFORM_LIBS d3d9 d3dx9 winmm ws2_32 )
	
SET( XPRENDER_D3D9
	${XPRENDER_COMMON}
	lib/xprender/API.d3d9.h
	lib/xprender/Buffer.d3d9.c
	lib/xprender/Buffer.d3d9.h
	lib/xprender/GpuState.d3d9.c
	lib/xprender/GpuState.d3d9.h
	lib/xprender/Mat44.d3d9.c
	lib/xprender/RenderTarget.d3d9.c
	lib/xprender/RenderTarget.d3d9.h
	lib/xprender/Shader.d3d9.c
	lib/xprender/Shader.d3d9.h
	lib/xprender/Texture.d3d9.c
	lib/xprender/Texture.d3d9.h
	)
SOURCE_GROUP( "xprender" FILES ${XPRENDER_D3D9} )
	
ADD_LIBRARY( xprender-d3d9
	${GLSW}
	${HTTPD}
	${XPRENDER_D3D9}
	)

SET( EXAMPLE_COMMON 
	
	lib/xprender/Framework.h
	lib/xprender/Framework.windows.d3d9.c
	example/Common.c
	example/Common.h
	example/Common.hlsl
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
	example/Pvr.c
	example/Pvr.h
	)

# example_cloth_simulation
ADD_EXECUTABLE( example_cloth_simulation-d3d9 ${CONSOLE_SYSTEM}
	${EXAMPLE_COMMON}
	example/ClothSimulation.example.c
	example/ClothSimulation.hlsl
	example/ShadowMap.hlsl
	example/Cloth.c
	example/Cloth.h
	example/Sphere.c
	example/Sphere.h
	)
TARGET_LINK_LIBRARIES( example_cloth_simulation-d3d9 xprender-d3d9 ${PLATFORM_LIBS} )

# example_label
ADD_EXECUTABLE( example_label-d3d9 ${CONSOLE_SYSTEM}
	${EXAMPLE_COMMON}
	example/Label.example.c
	example/Label.c
	example/Label.h
	)
TARGET_LINK_LIBRARIES( example_label-d3d9 xprender-d3d9 ${PLATFORM_LIBS} )

# example_embedded_httpd
ADD_EXECUTABLE( example_embedded_httpd-d3d9 ${CONSOLE_SYSTEM}
	${EXAMPLE_COMMON}
	example/EmbeddedHttpd.example.c
	)
TARGET_LINK_LIBRARIES( example_embedded_httpd-d3d9 xprender-d3d9 ${PLATFORM_LIBS} )

# example_mesh
ADD_EXECUTABLE( example_mesh-d3d9 ${CONSOLE_SYSTEM}
	${EXAMPLE_COMMON}
	example/Mesh.example.c
	example/Mesh.hlsl
	)
TARGET_LINK_LIBRARIES( example_mesh-d3d9 xprender-d3d9 ${PLATFORM_LIBS} )

# example_mesh_performance
ADD_EXECUTABLE( example_mesh_performance-d3d9 ${CONSOLE_SYSTEM}
	${EXAMPLE_COMMON}
	example/MeshPerformance.example.c
	example/MeshPerformance.hlsl
	)
TARGET_LINK_LIBRARIES( example_mesh_performance-d3d9 xprender-d3d9 ${PLATFORM_LIBS} )

ENDIF()
