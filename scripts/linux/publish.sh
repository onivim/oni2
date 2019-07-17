mkdir -p _publish

tar -C _release -cvzf _publish/Onivim2.tar.gz Onivim2.AppDir

cp _release/Onivim2-x86_64.AppImage _publish/Onivim2-x86_64.AppImage
