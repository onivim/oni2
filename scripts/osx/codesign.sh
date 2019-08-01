SHORT_COMMIT_ID=$(git rev-parse --short HEAD)


	echo "Starting codesign..."
	codesign --deep --force --verbose --sign "Outrun Labs, LLC" _release/Onivim2.App --options runtime --entitlements _release/entitlements.plist
	echo "Onivim2.App codesign complete!"

	# Validate
	echo "Validate codesigning..."
	codesign --verify --deep --strict --verbose=2 _release/Onivim2.App
	echo "Validation complete!"

ditto -c -k --rsrc --keepParent _release/Onivim2.App _release/Onivim2.App.zip
