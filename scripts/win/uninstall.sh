#!/bin/sh

while true; do
    read -p "Do you want to remove Vortex and built-in modules from 'C:/Program Files'? [y/n] " yn
    case $yn in
        [Yy]* ) 
        del /Q "C:\Program Files\vortex.exe" && rmdir /S /Q "C:\Program Files\vortex"
        echo "Removed Vortex and standard modules from Program Files"; 
        break;;
        [Nn]* ) exit;;
        * ) echo "Please answer y or n.";;
    esac
done