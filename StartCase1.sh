#!/bin/bash 
plist=( 4 5 4 3 2  ) 

for I in "${plist[@]}"
do
  xterm -e ./client $I &
 done
./manager
