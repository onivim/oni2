SHORT_COMMIT_ID=$(git rev-parse --short HEAD)

echo "** Validating raw app **"
mkdir -p _unpacked

ls $SYSTEM_ARTIFACTSDIRECTORY
ls $SYSTEM_ARTIFACTSDIRECTORY/Release_Darwin

tar -xzf $SYSTEM_ARTIFACTSDIRECTORY/Release_Darwin/Onivim2-$SHORT_COMMIT_ID-darwin.tar.gz -C _unpacked

ls _unpacked

ONI2_DEBUG=1 ./_unpacked/Onivim2.app/Contents/MacOS/Oni2 -f --checkhealth

echo "** Validating DMG **"
rm -rf _unpacked
mkdir _unpacked
echo " - Attaching dmg...."
sudo hdiutil attach $SYSTEM_ARTIFACTSDIRECTORY/Release_Darwin/Onivim2-$SHORT_COMMIT_ID.dmg
echo " - DMG attached! Copying..."
cp -rf "/Volumes/Onivim 2"/*.app _unpacked
echo " - Copy completed. Detaching DMG..."
sudo hdiutil detach "/Volumes/Onivim 2"
echo "DMG detached - running health check"
ONI2_DEBUG=1 ./_unpacked/Onivim2.app/Contents/MacOS/Oni2 -f --checkhealth
