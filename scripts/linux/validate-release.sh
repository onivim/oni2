mkdir -p _unpacked

ls $SYSTEM_ARTIFACTSDIRECTORY

tar -xzf $SYSTEM_ARTIFACTSDIRECTORY/Release_linux/Onivim2.tar.gz -C _unpacked

ls _unpacked

ONI2_DEBUG=1 ./_unpacked/Onivim2.AppDir/usr/bin/Oni2 -f --checkhealth
