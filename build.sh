#!/bin/bash

NC='\033[0m'

F_BOLD="\033[1m\033[38;5;15m"
C_MAIN="\033[38;5;243m"

set -e

echo -e "${F_BOLD}𐅡 --- HMCrypt Builder ---${C_MAIN}"

GIT_VERSION=$(git describe --tags --always 2>/dev/null || echo "1.0.0")
PKG_VER=$(echo "$GIT_VERSION" | sed 's/^v//' | tr '-' '.')

mkdir -p build
cd build

echo -e "${F_BOLD}𐅡 конфигурация CMake...${C_MAIN}"
cmake ..

echo -e "${F_BOLD}𐅡 компиляция...${NC}"
cmake --build .

echo -e "${F_BOLD}𐅡 подготовка структуры пакета...${C_MAIN}"
pkg_root="pkg_root"
rm -rf "$pkg_root"
mkdir -p "$pkg_root/usr/bin"

cp hmcrypt "$pkg_root/usr/bin/hmcrypt"

file_size=$(stat -c%s "$pkg_root/usr/bin/hmcrypt")
build_date=$(date +%s)

echo -e "${F_BOLD}𐅡 генерация .PKGINFO${C_MAIN}"
cat << EOF > "$pkg_root/.PKGINFO"
pkgname = hmcrypt
pkgver = $PKG_VER-1
pkgdesc = HMCrypt utility compiled binary
url = https://github.com
builddate = $build_date
packager = samine
size = $file_size
arch = x86_64
license = MIT
depend = glibc
depend = gcc-libs
EOF

echo -e "${F_BOLD}𐅡 упаковка .pkg.tar.zst${C_MAIN}"
tar -cv --zstd -f hmcrypt-release.pkg.tar.zst -C "$pkg_root" .PKGINFO usr

cd ..
ln -s build/compile_commands.json compile_commands.json
rm -rf "$pkg_root"

echo -e "${F_BOLD}𐅡 --- сборка завершена ---${C_MAIN}"
echo -e "${F_BOLD}Вы можете установить пакет командой: sudo pacman -U build/hmcrypt-release.pkg.tar.zst${NC}"
