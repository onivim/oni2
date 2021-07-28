$SHORT_COMMIT_ID_UNTRIMMED = (git rev-parse --short HEAD) | Out-String

$SHORT_COMMIT_ID = $SHORT_COMMIT_ID_UNTRIMMED.Trim()



function CodeSign {

    Param ([string]$path)



    Write-Host "Signing $path with certificate $env:CODESIGN_CERTIFICATE"



    &"C:/Program Files (x86)/Windows Kits/10/bin/10.0.17763.0/x64/signtool.exe" sign /tr http://timestamp.digicert.com /fd sha256 /td sha256 /f $env:CODESIGN_CERTIFICATE /p $env:CODESIGN_PASSWORD_WIN $path

}



if (Test-Path env:CODESIGN_CERTIFICATE) {

    Write-Host "Code signing enabled."



    CodeSign("_release/win32/Oni2.exe")

    CodeSign("_release/win32/Oni2_editor.exe")

    CodeSign("_release/win32/rg.exe")

    CodeSign("_release/win32/node.exe")

    CodeSign("_release/win32/*.dll")

    CodeSign("_release/win32/node/node_modules/node-pty/build/Release/*.dll")

    CodeSign("_release/win32/node/node_modules/node-pty/build/Release/*.exe")

    CodeSign("_release/win32/node/node_modules/node-pty/build/Release/*.node")

}



mkdir -p _publish



Compress-Archive -Path _release/win32 -DestinationPath _publish/Onivim2-$SHORT_COMMIT_ID.zip



esy "@release" create-win-setup



npm install -g innosetup-compiler

innosetup-compiler _release/setup.iss --O=_publish



if (Test-Path env:CODESIGN_CERTIFICATE) {

    CodeSign("_publish/Onivim2-win.exe");

}



mv _publish/Onivim2-win.exe _publish/Onivim2-$SHORT_COMMIT_ID-x64.exe

