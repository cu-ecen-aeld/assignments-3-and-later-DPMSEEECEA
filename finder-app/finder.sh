#!/bin/bash

#Argumnets
filedir=$1
searchstr=$2
X=$(find $filedir $searchstr -type f 2>/dev/null | wc -l)
Y=$(grep -sEnioR $searchstr $filedir | wc -l)
message="The number of files are $X and the number of matching lines are $Y"

if [[ ! $1 ]] 
then 
	echo "Error: parameter not specified: missing path"
	exit 1
fi
if [[ ! $2 ]] 
then 
	echo "Error: parameter not specified: missing search string"
	exit 1
fi
if [[ -d $1 ]] 
then
	echo $message
else
	echo "Error: valid parameter not specified: no such directory"
fi