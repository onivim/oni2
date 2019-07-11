echo "cur bin $cur__bin"

rm -rf _release
rm -rf _staging

mkdir -p _release/linux
mkdir -p _staging

cp -r $cur__bin _release/linux

wget -O _staging/linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x _staging/linuxdeploy-x86_64.AppImage

./_staging/linuxdeploy-x86_64.AppImage -e _release/linux/bin/Oni2_editor --appdir _release/linux


