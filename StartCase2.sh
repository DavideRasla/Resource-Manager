#!/bin/bash 
plist=( 1 7 5 3 2  ) 

for I in "${plist[@]}"
do
  xterm -e ./client $I &
 done
./manager
