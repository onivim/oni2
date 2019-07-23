SHORT_COMMIT_ID=$(git rev-parse --short HEAD)

npm install -g appdmg

mkdir -p _publish

tar -C _release -cvzf _publish/Onivim2-$SHORT_COMMIT_ID-darwin.tar.gz Onivim2.App

appdmg _release/appdmg.json _publish/Onivim2-$SHORT_COMMIT_ID.dmg
