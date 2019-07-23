$SHORT_COMMIT_ID_UNTRIMMED = (git rev-parse --short HEAD) | Out-String
$SHORT_COMMIT_ID = $SHORT_COMMIT_ID_UNTRIMMED.Trim()

mkdir -p _publish

Compress-Archive -Path _release/win32 -DestinationPath _publish/Onivim2-$SHORT_COMMIT_ID.zip

esy create-win-setup

npm install -g innosetup-compiler
innosetup-compiler _release/setup.iss --O=_publish
mv _publish/Onivim2-0.2.0-win.exe _publish/Onivim2-$SHORT_COMMIT_ID-x64.exe
