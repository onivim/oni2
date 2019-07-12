echo "cur bin $cur__bin"

rm -rf _release
rm -rf _staging

mkdir -p _release/linux
mkdir -p _staging

cp -r $cur__bin _release/linux

wget -O _staging/linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x _staging/linuxdeploy-x86_64.AppImage

wget -O _staging/appimagetool-x86_64.AppImage https://github.com/AppImage/AppImageKit/releases/download/12/appimagetool-x86_64.AppImage
chmod +x _staging/appimagetool-x86_64.AppImage

cp scripts/linux/Onivim2.desktop _release/linux/Onivim2.desktop
cp assets/images/icon512.png _release/linux/Onivim2.png

./_staging/linuxdeploy-x86_64.AppImage -e _release/linux/bin/Oni2_editor --appdir _release/Onivim2.AppDir -d _release/linux/Onivim2.desktop -i _release/linux/Onivim2.png

cp scripts/linux/Onivim2.desktop _release/Onivim2.AppDir/Onivim2.desktop
cp assets/images/icon512.png _release/Onivim2.AppDir/Onivim2.png

cp _release/linux/bin/*.* _release/Onivim2.AppDir/usr/bin
cp _release/linux/bin/Oni2 _release/Onivim2.AppDir/usr/bin/Oni2

ARCH=x86_64 _staging/appimagetool-x86_64.AppImage _release/Onivim2.AppDir _release/Onivim2-x86_64.AppImage

