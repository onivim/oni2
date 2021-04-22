#!/usr/bin/env bash

if [ -z "$CERTIFICATE_NAME" ]
then
   CERTIFICATE_NAME="Outrun Labs, LLC"
fi

SHORT_COMMIT_ID=$(git rev-parse --short HEAD)

manual-codesign() {
    codesign --force --verbose --sign "$CERTIFICATE_NAME" _release/Onivim2.app/$1 --options runtime --entitlements _release/entitlements.plist
}

if [ -z "$OSX_P12_CERTIFICATE" ]      
then   
   echo "No code signing certificate specified."   
else   
   echo "Code signing certificate specified: $OSX_P12_CERTIFICATE, $CERTIFICATE_NAME"       

   # Load cert     
   echo $OSX_P12_CERTIFICATE | base64 --decode > certificate.p12   

   # Create keychain       
   security create-keychain -p p@ssword1 build.keychain    
   security default-keychain -s build.keychain     
   security unlock-keychain -p p@ssword1 build.keychain    

   security import certificate.p12 -k build.keychain -P $CODESIGN_PASSWORD -T /usr/bin/codesign    

   security set-key-partition-list -S apple-tool:,apple: -s -k p@ssword1 build.keychain    

   echo "Checking identities..."   

   security find-identity -v
   echo "Starting codesign..."

   # Codesign individual files that require it
   manual-codesign Contents/MacOS/rg
   manual-codesign Contents/MacOS/node
   manual-codesign Contents/MacOS/Oni2_editor
   manual-codesign Contents/MacOS/Oni2
   manual-codesign Contents/Frameworks/Sparkle.framework/Versions/A/Resources/AutoUpdate.app/Contents/MacOS/fileop
   manual-codesign Contents/Frameworks/Sparkle.framework/Versions/A/Resources/AutoUpdate.app
   manual-codesign Contents/Frameworks/Sparkle.framework/Versions/A
   manual-codesign Contents/Frameworks/Sparkle.framework
   manual-codesign Contents/Resources/node/node_modules/node-pty/build/Release/pty.node
   manual-codesign Contents/Resources/node/node_modules/spdlog/build/Release/spdlog.node
   manual-codesign Contents/Resources/node/node_modules/native-watchdog/build/Release/watchdog.node

   # Finish by codesigning the root
   manual-codesign
   
   echo "Onivim2.app codesign complete!"

   # Validate
   echo "Validate codesigning..."
   codesign --verify --deep --strict --verbose=2 _release/Onivim2.app
   echo "Validation complete!"

   ditto -c -k --rsrc --keepParent _release/Onivim2.app _release/Onivim2.app.zip
fi
