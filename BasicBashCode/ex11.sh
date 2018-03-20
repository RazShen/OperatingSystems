#!/bin/bash

sum=0
cd $1
for i in $(ls)
do
	if [[ ${i: -4} = ".txt" ]]
	then
		let "sum += 1"
	fi
done

echo "Number of files in the directory that end with .txt is $sum"

