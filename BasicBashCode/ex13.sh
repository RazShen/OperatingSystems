#!/bin/bash

if [[ ! -z $1 && ! -z $2 ]] || [ -z $1 ]
then
	echo "error: only one argument is allowed"
else
	if [ ! -e $1 ]
	then
		echo "error: there is no such file"
	else
		if [ ! -d safe_rm_dir ]
		then
			mkdir safe_rm_dir
		fi
		cp $1 safe_rm_dir
		rm $1
		echo "done!"
	fi		
fi


