set( SRC
	VdbReadCollection.cpp
	FragmentBlobIterator.cpp
	FragmentBlob.cpp
	VdbReference.cpp
	ReferenceBlob.cpp
	ReferenceBlobIterator.cpp
	VdbAlignment.cpp
	NGS-VDB.cpp
)

GenerateStaticLibsWithDefs( ngs-vdb "${SRC}" "" "${CMAKE_SOURCE_DIR}/ngs" )

if ( NOT TOOLS_ONLY )
    add_subdirectory( test )
endif()
