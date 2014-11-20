#!/bin/sh


if [ "$#" -lt 1 ] ; then
     echo "==============usage============="
     echo "build :                     sh  $0  build   <verbose>"
     echo "build clean :               sh  $0  clean"
     exit
elif [ "$1" = "build"  ] ; then	
	 #using clang building  g++ (GCC) 4.9.1  clang version 3.6.0 (trunk)
	 source /data/qqpet/build_clang/usingclang.sh
     mkdir -p build
     cd build
     cmake  ..
     if [ "$2" = "verbose" ] ; then
        make VERBOSE=1
     else
        make
     fi
elif [ "$1" = "clean"  ] ; then	
     rm -rf  build 
fi



