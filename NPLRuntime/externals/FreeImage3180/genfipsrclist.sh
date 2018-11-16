#!/bin/sh

DIRLIST=". Source Source/Metadata Source/FreeImageToolkit Source/LibJPEG Source/LibPNG Source/LibTIFF4 Source/ZLib Source/LibOpenJPEG Source/OpenEXR Source/OpenEXR/Half Source/OpenEXR/Iex Source/OpenEXR/IlmImf Source/OpenEXR/IlmThread Source/OpenEXR/Imath Source/OpenEXR/IexMath Source/LibRawLite Source/LibRawLite/dcraw Source/LibRawLite/internal Source/LibRawLite/libraw Source/LibRawLite/src Source/LibWebP Source/LibJXR Source/LibJXR/common/include Source/LibJXR/image/sys Source/LibJXR/jxrgluelib Wrapper/FreeImagePlus"


echo "VER_MAJOR = 3" > fipMakefile.srcs
echo "VER_MINOR = 18.0" >> fipMakefile.srcs

echo -n "SRCS = " >> fipMakefile.srcs
for DIR in $DIRLIST; do
	VCPRJS=`echo $DIR/*.2013.vcxproj`
	if [ "$VCPRJS" != "$DIR/*.2013.vcxproj" ]; then
		egrep 'ClCompile Include=.*\.(c|cpp)' $DIR/*.2013.vcxproj | cut -d'"' -f2 | tr '\\' '/' | awk '{print "'$DIR'/"$0}' | tr '\r\n' '  ' | tr -s ' ' >> fipMakefile.srcs
	fi
done
echo >> fipMakefile.srcs

echo -n "INCLUDE =" >> fipMakefile.srcs
for DIR in $DIRLIST; do
	echo -n " -I$DIR" >> fipMakefile.srcs
done
echo >> fipMakefile.srcs

