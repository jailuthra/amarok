add_subdirectory( albums )
add_subdirectory( currenttrack )
add_subdirectory( info )
add_subdirectory( labels )
add_subdirectory( lyrics )
add_subdirectory( photos )
add_subdirectory( tabs )
add_subdirectory( wikipedia )

if( QT_QTOPENGL_FOUND )
    add_subdirectory( analyzer )
endif()

if( LIBLASTFM_FOUND )
    add_subdirectory( upcomingevents )
    add_subdirectory( similarartists )
endif()

