temp_path="AppIcon.iconset"
filename="icon.png"

sips -z 16 16 $filename --out $temp_path/icon_16x16.png
sips -z 32 32 $filename --out $temp_path/icon_16x16@2x.png
sips -z 32 32 $filename --out $temp_path/icon_32x32.png
sips -z 64 64 $filename --out $temp_path/icon_32x32@2x.png
sips -z 128 128 $filename --out $temp_path/icon_128x128.png
sips -z 256 256 $filename --out $temp_path/icon_128x128@2x.png
sips -z 256 256 $filename --out $temp_path/icon_256x256.png
sips -z 512 512 $filename --out $temp_path/icon_256x256@2x.png
sips -z 512 512 $filename --out $temp_path/icon_512x512.png
sips -z 1024 1024 $filename --out $temp_path/icon_512x512@2x.png

iconutil -c icns $temp_path -o icon.icns
