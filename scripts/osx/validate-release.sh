mkdir -p _unpacked

ls $SYSTEM_ARTIFACTSDIRECTORY

tar -xzf $SYSTEM_ARTIFACTSDIRECTORY/Release_Darwin/Onivim2.tar.gz _unpacked/.

ls _unpacked

ONI2_DEBUG=1 ./_unpacked/Onivim2.App/Contents/MacOS/Oni2 -f --checkhealth
