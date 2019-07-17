echo "** Validating raw app **"
mkdir -p _unpacked

ls $SYSTEM_ARTIFACTSDIRECTORY

tar -xzf $SYSTEM_ARTIFACTSDIRECTORY/Release_Darwin/Onivim2.tar.gz -C _unpacked

ls _unpacked

ONI2_DEBUG=1 ./_unpacked/Onivim2.App/Contents/MacOS/Oni2 -f --checkhealth

echo "** Validating DMG **"
rm -rf _unpacked
mkdir _unpacked
sudo hdiutil attach $SYSTEM_ARTIFACTSDIRECTORY/Onivim2.dmg
cp -rf "/Volumes/Onivim 2"/*.App _unpacked
sudo hdiutil detach "/Volumes/Onivim 2"
ONI2_DEBUG=1 ./_unpacked/Onivim2.App/Contents/MacOS/Oni2 -f --checkhealth




