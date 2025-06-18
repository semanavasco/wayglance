#!/bin/bash

# Script to compile the Hyprland Widget

g++ src/main.cpp \
    -o hypr-widget \
    `pkg-config --cflags --libs gtkmm-4.0 gtk4-layer-shell-0` \
    -std=c++20

# Checking if the compilation has succeeded
if [ $? -eq 0 ]; then
    echo "Compilation succeeded. 'hypr-widget' executable has been created."
else
    echo "Compilation has failed."
fi
