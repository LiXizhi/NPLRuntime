set (file_root_src
	src/analysis.c
	src/window.c
	src/bitrate.c
	src/block.c
	src/codebook.c
	src/envelope.c
	src/floor0.c
	src/floor1.c
	src/info.c
	src/lookup.c
	src/lpc.c
	src/lsp.c
	src/mapping0.c
	src/mdct.c
	src/psy.c
	src/registry.c
	src/res0.c
	src/sharedbook.c
	src/smallft.c
	src/synthesis.c
	src/vorbisenc.c
	src/vorbisfile.c
)
set (file_root_include
	src/backends.h
	src/bitrate.h
	src/codebook.h
	include/vorbis/codec.h
	src/codec_internal.h
	src/envelope.h
	src/modes/floor_all.h
	src/books/floor/floor_books.h
	src/highlevel.h
	src/lookup.h
	src/lookup_data.h
	src/lpc.h
	src/lsp.h
	src/masking.h
	src/mdct.h
	src/misc.h
	src/os.h
	src/books/coupled/res_books_stereo.h
	src/modes/setup_X.h
	src/books/uncoupled/res_books_uncoupled.h
	src/modes/residue_16.h
	src/modes/residue_44.h
	src/modes/residue_44u.h
	src/modes/residue_8.h
	src/scales.h
	src/modes/setup_11.h
	src/modes/setup_16.h
	src/modes/setup_22.h
	src/modes/setup_32.h
	src/modes/setup_44.h
	src/modes/setup_44u.h
	src/modes/setup_8.h
	src/psy.h
	src/modes/psych_8.h
	src/modes/psych_11.h
	src/modes/psych_16.h
	src/modes/psych_44.h
	src/registry.h
	src/smallft.h
	include/vorbis/vorbisenc.h
	include/vorbis/vorbisfile.h
	src/window.h
)

source_group("src" FILES ${file_root_src})
source_group("include" FILES ${file_root_include})

set (file_root
	${file_root_src}
	${file_root_include}
)
