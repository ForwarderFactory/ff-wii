#!/bin/sh
[ "$(id -u)" != "0" ] && printf "%s\n" "Run me as superuser." && exit 1
[ ! -z "$(command -v pacman)" ] && PACMAN="pacman"
[ ! -z "$(command -v dkp-pacman)" ] && PACMAN="dkp-pacman"
[ -z "$PACMAN" ] && printf "%s\n" "No package manager found." && exit 1
$PACMAN -S --needed --noconfirm libfat-ogc ppc-libpng ppc-freetype ppc-libjpeg-turbo
