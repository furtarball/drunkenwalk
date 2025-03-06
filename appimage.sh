#!/bin/bash
set -Eeuxo pipefail
meson configure -Dprefix=/usr
DESTDIR=AppDir meson install
wget https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/latest/download/linuxdeploy-plugin-appimage-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy/releases/latest/download/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-plugin-appimage-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
NO_STRIP=true ./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage
