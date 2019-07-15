mkdir -p _unpacked

Expand-Archive -Path $env:SYSTEM_ARTIFACTSDIRECTORY/Onivim2.zip -DestinationPath _unpacked/

ls _unpacked
$env:ONI2_DEBUG=1
./_unpacked/win32/Oni2.exe -f --checkhealth
