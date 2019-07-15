mkdir -p _unpacked

ls $env:SYSTEM_ARTIFACTSDIRECTORY
ls $env:SYSTEM_ARTIFACTSDIRECTORY/Release_Windows

Expand-Archive -Path $env:SYSTEM_ARTIFACTSDIRECTORY/Release_Windows/Onivim2.zip -DestinationPath _unpacked/

ls _unpacked
$env:ONI2_DEBUG=1
./_unpacked/win32/Oni2.exe -f --checkhealth
