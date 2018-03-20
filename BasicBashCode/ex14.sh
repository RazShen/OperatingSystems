#!/bin/bash

sum=0

cat $2 | (while read line
do

   if [[ "${line/$1}" != "$line" ]]
   then
	echo $line
	string=($line)
	a=${string[2]}
	sum=$[$sum+$a]
   fi
  
done
echo "Total balance: $sum")
