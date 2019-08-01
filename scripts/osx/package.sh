SHORT_COMMIT_ID=$(git rev-parse --short HEAD)


npm install -g appdmg

mkdir -p _publish

appdmg _release/appdmg.json _publish/Onivim2-$SHORT_COMMIT_ID.dmg

ditto -c -k --rsrc --keepParent _release/Onivim2.App _publish/Onivim2-$SHORT_COMMIT_ID.App.zip
