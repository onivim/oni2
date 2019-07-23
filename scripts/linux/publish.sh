SHORT_COMMIT_ID=$(git rev-parse --short HEAD)

mkdir -p _publish

tar -C _release -cvzf _publish/Onivim2-$SHORT_COMMIT_ID-linux.tar.gz Onivim2.AppDir

cp _release/Onivim2-x86_64.AppImage _publish/Onivim2-$SHORT_COMMIT_ID-x86_64.AppImage
