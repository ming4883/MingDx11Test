
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

# example_cloth_simulation
SET( NAME example_cloth_simulation )
SET( SRC 
	${EXAMPLE_COMMON}
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
	SET_SOURCE_FILES_PROPERTIES( ${RES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
ELSE()
	SET( RES
		Common.glsl
		ClothSimulation.glsl
		ShadowMap.glsl
		)
ENDIF()

ADD_EXECUTABLE( ${NAME} ${CONSOLE_SYSTEM} ${SRC} ${RES} )
TARGET_LINK_LIBRARIES( ${NAME} xprender ${PLATFORM_LIBS} )

IF( APPLE )
	SET_TARGET_PROPERTIES( ${NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PLIST_FILE} )
ENDIF()

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

