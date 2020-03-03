SHORT_COMMIT_ID=$(git rev-parse --short HEAD)

manual-codesign() {
    echo "Signing with ${CODESIGN_IDENTITY}"
    codesign --keychain $CODESIGN_KEYCHAIN --force --verbose --sign $CODESIGN_IDENTITY _release/Onivim2.app/$1 --options runtime --entitlements _release/entitlements.plist
}

if [ -z "$OSX_P12_CERTIFICATE" ]      
then   
   echo "No code signing certificate specified."   
   scripts/osx/create-temporary-signing-key.sh
   CODESIGN_PASSWORD=password!
   CODESIGN_IDENTITY="local.outrunlabs.com"
   CODESIGN_KEYCHAIN=onivim.keychain
else   
   echo $OSX_P12_CERTIFICATE | base64 --decode > certificate.p12   
   CODESIGN_IDENTITY="Outrun Labs, LLC"
   CODESIGN_KEYCHAIN=build2.keychain

   # Create keychain       
   security create-keychain -p p@ssword1 build2.keychain    
   security default-keychain -s build2.keychain     
   security unlock-keychain -p p@ssword1 build2.keychain    
   
   security import certificate.p12 -k build2.keychain -P $CODESIGN_PASSWORD -T /usr/bin/codesign    
   
   security set-key-partition-list -S apple-tool:,apple: -s -k p@ssword1 build2.keychain    
fi

echo "Code signing certificate specified: ${CODESIGN_IDENTITY}"       

# Load cert     

echo "Checking identities..."   

security find-identity -v -p codesigning $CODESIGN_KEYCHAIN
echo "Starting codesign..."

# Codesign individual files that require it
manual-codesign Contents/MacOS/rg
manual-codesign Contents/MacOS/rls
manual-codesign Contents/MacOS/node
manual-codesign Contents/MacOS/Oni2_editor
manual-codesign Contents/MacOS/Oni2
manual-codesign Contents/Frameworks/libonig.5.dylib
manual-codesign Contents/Frameworks/libpng16.16.dylib
manual-codesign Contents/Frameworks/libfreetype.6.dylib
manual-codesign Contents/Frameworks/libharfbuzz.0.dylib
manual-codesign Contents/Frameworks/libSDL2-2.0.0.dylib
manual-codesign Contents/Frameworks/libffi.6.dylib
manual-codesign Contents/Resources/node/node_modules/node-pty/build/Release/pty.node

# Finish by codesigning the root
manual-codesign

echo "Onivim2.app codesign complete!"

# Validate
echo "Validate codesigning..."
codesign --verify --deep --strict --verbose=2 _release/Onivim2.app
echo "Validation complete!"

ditto -c -k --rsrc --keepParent _release/Onivim2.app _release/Onivim2.app.zip
