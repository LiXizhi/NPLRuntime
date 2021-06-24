set (file_root_source_files
	src/cMP3Decoder.cpp
	src/main.cpp
)
set (file_root_mpaudec
	mpaudec/bits.c
	mpaudec/mpaudec.c
	mpaudec/internal.h
	mpaudec/mpaudec.h
	mpaudec/mpaudectab.h
	mpaudec/mpegaudio.h
)
set (file_root_header_files
	include/cMP3Decoder.h
	include/cMP3DecoderFactory.h
	include/cMP3Plugin.h
)

source_group("Source Files" FILES ${file_root_source_files})
source_group("mpaudec" FILES ${file_root_mpaudec})
source_group("Header Files" FILES ${file_root_header_files})


set (file_root
	${file_root_source_files}
	${file_root_mpaudec}
	${file_root_header_files}
)
