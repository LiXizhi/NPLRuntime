set (file_root
	src/bitwise.c
	src/framing.c
	include/ogg/ogg.h
	include/ogg/os_types.h	
        include/ogg/config_types.h

)

source_group("" FILES ${file_root})

set (file_root
	${file_root}
)
