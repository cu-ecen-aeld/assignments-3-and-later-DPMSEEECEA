#!/bin/bash

#Argumnets
writefile=$1
file_path=$(dirname $1)
writestr=$2

if [[ ! $writefile ]] 
then 
	echo "Error: parameter not specified: missing path\filename"
	exit 1
fi
if [[ ! $writestr ]] 
then 
	echo "Error: parameter not specified: missing write string"
	exit 1
else
    if [[ -d $file_path ]] 
then
    echo "Continue: Filepath already exist: $file_path"
else
    echo "ERROR: Filepath does not exist creating: $file_path"
    mkdir -p $file_path
fi

if [[ -e $writefile ]] 
then
    echo "Continue: File already exist overwriting file: $file_path"
    echo $writestr > $writefile
    if [[ $? -ne 0 ]]
    then
       echo 'ERROR: File overwrite failure'
       exit 1
    else
       echo 'INFO: File overwite succceeded'
    fi
else
    echo "ERROR: File does not exist creating: $file_path"
    touch $writefile; echo $writestr > $writefile
    if [[  $? -ne 0  ]]
    then
        echo 'ERROR: File creation failure'
        exit 1
    else
        echo 'INFO: File creation succceeded'
    fi
fi
fi

