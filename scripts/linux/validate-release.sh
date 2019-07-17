
mkdir -p _unpacked

ls $SYSTEM_ARTIFACTSDIRECTORY

tar -xzf $SYSTEM_ARTIFACTSDIRECTORY/Release_linux/Onivim2.tar.gz -C _unpacked

ls _unpacked

ONI2_DEBUG=1 ./_unpacked/Onivim2.AppDir/usr/bin/Oni2 -f --checkhealth

cp $SYSTEM_ARTIFACTSDIRECTORY/Release_linux/Onivim2-x86_64.AppImage _unpacked/Onivim2-x86_64.AppImage

ls _unpacked

chmod +x _unpacked/Onivim2-x86_64.AppImage
./_unpacked/Onivim2-x86_64.AppImage -f --checkhealth
