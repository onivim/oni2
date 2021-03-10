SHORT_COMMIT_ID=$(git rev-parse --short HEAD)

# Script from: https://twocanoes.com/adding-notarization-to-xcode-builds
echo "Code signing certificate specified - notarizing zip"

echo "Uploading to apple to notarize..."
notarize_output=$(xcrun altool --notarize-app --primary-bundle-id "com.outrunlabs.onivim2" --username $APPLE_DEVELOPER_ID --password $APPLE_NOTARIZE_PASSWORD --file "_release/Onivim2.app.zip" 2>&1)

echo "$notarize_output" > notarize_output.tmp
echo "xcrun altool output: $notarize_output"

notarize_uuid=$(grep RequestUUID notarize_output.tmp | awk '{print $3}')

# Load cert
echo "notarize uuid: $notarize_uuid"

rm notarize_output.tmp

if  [ -z "$notarize_uuid" ]
then
	echo "Notarization failed; running again to get error message"
	failure=$(xcrun altool --notarize-app --primary-bundle-id "com.outrunlabs.onivim2" --username $APPLE_DEVELOPER_ID --password $APPLE_NOTARIZE_PASSWORD --file "_release/Onivim2.app.zip" 2>&1)
	echo "MESSAGE: $failure"
	exit 1
else
	echo "Got notarization UUID: $notarize_uuid"
fi

success=0
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
	echo "Checking progress..."
	progress=$(xcrun altool --notarization-info "${notarize_uuid}" -u $APPLE_DEVELOPER_ID -p $APPLE_NOTARIZE_PASSWORD 2>&1)
	echo "Progress: ${progress}"

	if [ $? -ne 0 ] || [[ "${progress}" =~ "Invalid" ]]; then
		echo "Error with notarization. Exiting"
        exit 1
	fi

	if [[ "${progress}" =~ "success" ]]; then
		success=1
		break
	else
		echo "Not completed yet. Sleeping for 30 seconds."
	fi
	sleep 30
done

if [ $success -eq 1 ] ; then
	echo "Stapling and running packaging up"
	xcrun stapler staple "_release/Onivim2.app"
	echo "Staple success!"

	echo "Checking gatekeeper conformance"
	spctl --assess --verbose "_release/Onivim2.app"
	echo "Complete!"
fi
