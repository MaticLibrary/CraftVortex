#!/usr/bin/env bash
set -e

if [ -f "$(g++ -print-file-name=libstdc++.a)" ]; then
    echo "Static g++ library is already installed, nothing to do"
    exit 0
fi

echo "libstdc++ not found, installing..."
if command -v apt > /dev/null; then CMD="sudo apt install -y libstdc++-$(g++ -dumpversion | cut -d. -f1)-dev"
elif command -v dnf > /dev/null; then CMD="sudo dnf install -y libstdc++-static"
elif command -v pacman > /dev/null; then CMD="sudo pacman -S --noconfirm --needed gcc"
else
    echo "Package manager not recognized. Please install libstdc++ manually."
    exit 1
fi

read -r -p "Install now? [y/N] " answer
case "$answer" in
    [tTyY]*) eval "$CMD" ;;
    *) echo "The installation has been aborted." ;;
esac