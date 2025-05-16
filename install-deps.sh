#!/bin/sh
[ "$(id -u)" != "0" ] && printf "%s\n" "Run me as superuser." && exit 1
[ -z "$(command -v pacman)" ] && printf "%s\n" "Pacman not found, install Arch Linux." && exit 1
pacman -S --needed --noconfirm libfat-ogc ppc-libpng ppc-freetype ppc-libjpeg-turbo

return 0
