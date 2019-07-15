mkdir -p _unpacked

tar -xzf $(SYSTEM_ARTIFACTS_DIRECTORY)/Onivim2.tar.gz _unpacked/.

ONI2_DEBUG=1 ./_unpacked/Onivim2.App/Contents/MacOS/Oni2 -f --checkhealth
