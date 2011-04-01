
SET( EXAMPLE_COMMON 
	Common.c
	Common.h
	Stream.c
	Stream.h
	Remote.c
	Remote.h
	Remote.impl.h
	Mesh.c
	Mesh.obj.c
	Mesh.h
	Material.c
	Material.h
	Pvr.c
	Pvr.h
	)

IF( WIN32 )
	SET( EXAMPLE_COMMON_D3D9
		${EXAMPLE_COMMON}
		../lib/xprender/Framework.h
		../lib/xprender/Framework.windows.d3d9.c
		Common.generic.c
		Remote.impl.windows.c
		Remote.impl.windows.h
	)
	
	LIST( APPEND EXAMPLE_COMMON
		../lib/xprender/Framework.h
		../lib/xprender/Framework.windows.gl.c
		Common.generic.c
		Remote.impl.windows.c
		Remote.impl.windows.h
	)
	
ELSEIF( APPLE )
	LIST( APPEND EXAMPLE_COMMON
		../lib/xprender/Framework.h
		../lib/xprender/Framework.ios.m
		Common.ios.m
		Remote.impl.ios.m
		Remote.impl.ios.h
	)
	
	SET( PLIST_FILE ${CMAKE_CURRENT_LIST_DIR}/Example-Info.plist )
	
ENDIF()

LIST( SORT EXAMPLE_COMMON )

SOURCE_GROUP( "common" FILES ${EXAMPLE_COMMON} )

MACRO( CREATE_EXAMPLE APP_NAME SRC RES )

ADD_EXECUTABLE( ${APP_NAME} ${CONSOLE_SYSTEM} ${SRC} ${RES} ${EXAMPLE_COMMON} )
TARGET_LINK_LIBRARIES( ${APP_NAME} xprender ${PLATFORM_LIBS} )

IF( APPLE )
	SET_SOURCE_FILES_PROPERTIES( ${RES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
	SET_TARGET_PROPERTIES( ${APP_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PLIST_FILE} )
ENDIF()

IF( WIN32 )
	SET( RES_D3D9 ${RES} )
	STRING( REPLACE glsl hlsl RES_D3D9 "${RES_D3D9}" )
	ADD_EXECUTABLE( ${APP_NAME}-d3d9 ${CONSOLE_SYSTEM}  ${SRC} ${RES_D3D9} ${EXAMPLE_COMMON_D3D9} )
	TARGET_LINK_LIBRARIES( ${APP_NAME}-d3d9 xprender-d3d9 ${PLATFORM_LIBS_D3D9} )
ENDIF()

ENDMACRO()


# example_cloth_simulation
SET( SRC 
	ClothSimulation.example.c
	Cloth.c
	Cloth.h
	Sphere.c
	Sphere.h
	)
	
IF( APPLE )
	SET( RES
		Common.gles
		ClothSimulation.gles
		ShadowMap.gles
		Example.xib
		)
ELSE()
	SET( RES
		Common.glsl
		ClothSimulation.glsl
		ShadowMap.glsl
		)
ENDIF()

CREATE_EXAMPLE( example_cloth_simulation "${SRC}" "${RES}" )

# example_embedded_httpd
SET( SRC 
	EmbeddedHttpd.example.c
	)
	
IF( APPLE )
	SET( RES
		Example.xib
		)
ELSE()
	SET( RES
		)
ENDIF()

CREATE_EXAMPLE( example_embedded_httpd "${SRC}" "${RES}" )

# example_mesh
SET( SRC 
	Mesh.example.c
	)
	
IF( APPLE )
	SET( RES
		Mesh.gles
		../media/monkey.obj
		Example.xib
		)
ELSE()
	SET( RES
		Mesh.glsl
		)
ENDIF()

CREATE_EXAMPLE( example_mesh "${SRC}" "${RES}" )

# example_mesh_performance
SET( SRC 
	MeshPerformance.example.c
	)
	
IF( APPLE )
	SET( RES
		MeshPerformance.gles
		../media/monkey.obj
		Example.xib
		)
ELSE()
	SET( RES
		MeshPerformance.glsl
		)
ENDIF()

CREATE_EXAMPLE( example_mesh_performance "${SRC}" "${RES}" )

IF( NOT APPLE )
# example_label
SET( SRC 
	Label.example.c
	Label.c
	Label.h
	)
	
IF( APPLE )
	SET( RES
		Common.gles
		Example.xib
		)
ELSE()
	SET( RES
		Common.glsl
		)
ENDIF()

CREATE_EXAMPLE( example_label "${SRC}" "${RES}" )

# example_triangle_tessellation
SET( SRC 
	TriangleTessellation.example.c
	)
	
IF( APPLE )
	SET( RES
		#TriangleTessellation.glsl
		Example.xib
		)
ELSE()
	SET( RES
		#TriangleTessellation.glsl
		)
ENDIF()

CREATE_EXAMPLE( example_triangle_tessellation "${SRC}" "${RES}" )

ENDIF()