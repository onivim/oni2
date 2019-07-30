SHORT_COMMIT_ID=$(git rev-parse --short HEAD)

	# Script from: https://twocanoes.com/adding-notarization-to-xcode-builds
	echo "Code signing certificate specified - notarizing DMG"

	uuid=$(uuidgen)
	echo "Uploading to apple to notarize: $uuid"
notarize_uuid=$(xcrun altool --notarize-app --primary-bundle-id "${uuid}" --username $APPLE_DEVELOPER_ID --password $APPLE_DEVELOPER_PASSWORD --file "_publish/Onivim2-$SHORT_COMMIT_ID.dmg" 2>&1 | grep RequestUUID | awk '{print $3'})
        echo "Notarize uuid: $notarize_uuid"
	# Load cert

	success=0
	for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
		echo "Checking progress..."
		progress=$(xcrun altool --notarization-info "${notarize_uuid}" -u $APPLE_DEVELOPER_ID -p $APPLE_DEVELOPER_PASSWORD 2>1)
		echo "${progress}"

		if [ $? -ne 0 ] || [[ "${progress}" =~ "Invalid" ]]; then
			echo "Error with notarization. Exiting"
		fi

		if [[ "${progress}" =~ "success" ]]; then
			success = 1
			break
		else
			echo "Not completed yet. Sleeping for 30 seconds."
		fi
		sleep 30
	done

	if [ $success -eq 1 ] ; then
		echo "Stapling and running packaging up"
		xcrun stapler staple "_publish/Onivim2-$SHORT_COMMIT_ID.dmg"
	fi
