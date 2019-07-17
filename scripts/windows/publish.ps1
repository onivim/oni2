mkdir -p _publish

Compress-Archive -Path _release/win32 -DestinationPath _publish/Onivim2.zip

esy create-win-setup

npm install -g innosetup-compiler
innosetup-compiler _release/setup.iss --O=_publish/Onivim2-x64.exe
