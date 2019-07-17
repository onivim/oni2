npm install -g appdmg

mkdir -p _publish

tar -C _release -cvzf _publish/Onivim2.tar.gz Onivim2.App

appdmg _release/appdmg.json _publish/Onivim2.dmg
