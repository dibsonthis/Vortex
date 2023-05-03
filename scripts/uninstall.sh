#!/bin/sh

while true; do
    read -p "Do you want to remove Vortex from 'usr/local/bin' and built-in modules from 'usr/local/share'? [y/n] " yn
    case $yn in
        [Yy]* ) sudo -s eval 'rm /usr/local/bin/vortex && rm -rf /usr/local/share/vortex'
        echo "Removed Vortex and standard modules from usr/local"; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer y or n.";;
    esac
done