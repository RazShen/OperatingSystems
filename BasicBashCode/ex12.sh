#!/bin/bash

cd $1

for i in $(ls)
do
    if [ -d $i ]
    then
        echo "$i is a directory"	
    else
        if [ -f $i ]
        then
            echo "$i is a file"
	fi
    fi
done
