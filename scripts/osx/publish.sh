npm install -g appdmg

mkdir -p _publish

tar -C _release -cvzf _publish/Onivim2.tar.gz Onivim2.App

appdmg _publish/appdmg.json _release/Onivim2.dmg
