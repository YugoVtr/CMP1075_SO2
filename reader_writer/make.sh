#!/bin/sh

for i in $(seq 1 $1)
do 
    gnome-terminal --hide-menubar --geometry 30X15 -- './main' 1;
done

for i in $(seq 1 $2)
do 
    gnome-terminal --hide-menubar --geometry 30X15 -- './main' 2;
done

exit; 