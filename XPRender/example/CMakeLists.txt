
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

MACRO( CREATE_EXAMPLE APP_NAME SRC RES )

SET( ALL_FILES ${SRC} ${RES} ${EXAMPLE_COMMON} )

ADD_EXECUTABLE( ${APP_NAME} ${CONSOLE_SYSTEM} ${ALL_FILES} )
TARGET_LINK_LIBRARIES( ${APP_NAME} xprender ${PLATFORM_LIBS} )

IF( APPLE )
	SET_SOURCE_FILES_PROPERTIES( ${RES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
	SET_TARGET_PROPERTIES( ${APP_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PLIST_FILE} )
ENDIF()

IF( WIN32 )
	SET( ALL_FILES ${SRC} ${RES} ${EXAMPLE_COMMON_D3D9} )
	ADD_EXECUTABLE( ${APP_NAME}-d3d9 ${CONSOLE_SYSTEM} ${ALL_FILES} )
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


# example_label
ADD_EXECUTABLE( example_label ${CONSOLE_SYSTEM}
	${EXAMPLE_COMMON}
	Label.example.c
	Label.c
	Label.h
	)
TARGET_LINK_LIBRARIES( example_label xprender ${PLATFORM_LIBS} )

# example_triangle_tessellation
ADD_EXECUTABLE( example_triangle_tessellation ${CONSOLE_SYSTEM}
	${EXAMPLE_COMMON}
	TriangleTessellation.example.c
	TriangleTessellation.glsl
	)
TARGET_LINK_LIBRARIES( example_triangle_tessellation xprender ${PLATFORM_LIBS} )

# example_embedded_httpd
ADD_EXECUTABLE( example_embedded_httpd ${CONSOLE_SYSTEM}
	${EXAMPLE_COMMON}
	EmbeddedHttpd.example.c
	)
TARGET_LINK_LIBRARIES( example_embedded_httpd xprender ${PLATFORM_LIBS} )

# example_mesh
SET( NAME example_mesh )
SET( SRC 
	${EXAMPLE_COMMON}
	Mesh.example.c
	)
	
IF( APPLE )
	SET( RES
		Mesh.gles
		../media/monkey.obj
		Example.xib
		)
	SET_SOURCE_FILES_PROPERTIES( ${RES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
ELSE()
	SET( RES
		Mesh.glsl
		)
ENDIF()

ADD_EXECUTABLE( ${NAME} ${CONSOLE_SYSTEM} ${SRC} ${RES} )
TARGET_LINK_LIBRARIES( ${NAME} xprender ${PLATFORM_LIBS} )

IF( APPLE )
	SET_TARGET_PROPERTIES( ${NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PLIST_FILE} )
ENDIF()

# example_mesh_performance
ADD_EXECUTABLE( example_mesh_performance ${CONSOLE_SYSTEM}
	${EXAMPLE_COMMON}
	MeshPerformance.example.c
	MeshPerformance.glsl
	)
TARGET_LINK_LIBRARIES( example_mesh_performance xprender ${PLATFORM_LIBS} )

