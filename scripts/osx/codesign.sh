SHORT_COMMIT_ID=$(git rev-parse --short HEAD)

SHORT_COMMIT_ID=$(git rev-parse --short HEAD)	

 if [ -z "$OSX_P12_CERTIFICATE" ]	
then	
	echo "No code signing certificate specified."	
else	
	echo "Code signing certificate specified"	

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
	codesign --deep --force --verbose --sign "Outrun Labs, LLC" _release/Onivim2.app --options runtime --entitlements _release/entitlements.plist
	echo "Onivim2.app codesign complete!"

	# Validate
	echo "Validate codesigning..."
	codesign --verify --deep --strict --verbose=2 _release/Onivim2.app
	echo "Validation complete!"

        ditto -c -k --rsrc --keepParent _release/Onivim2.app _release/Onivim2.app.zip
fi
