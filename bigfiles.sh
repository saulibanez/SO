#!/bin/bash

# Autor: Saúl Ibáñez Cerro
# Grado en Telemática
# Programa: bigfiles.sh

if [ $# -gt "1" ]; then
	echo "Usage: $0 [N files]" 1>&2
	exit 1
fi

num=1
if [ $# -eq "1" ]; then	
 	num=$1
fi

if echo $1 | egrep -q '^[1-9][0-9]*$' || [ $# -eq "0" ]; then
	files=`du -ab | sort -nr | awk '{print $2}'`
	for i in $files; do
		if [ -f $i ]; then
			du -b $i | awk '{print $2 "\t\t" $1}'
		fi
	done | sed "$num"q
else
	echo "Fail: The command: $1, should be a number greater than 0" 1>&2
	exit 1
fi
