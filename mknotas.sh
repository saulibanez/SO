#!/bin/bash

# Autor: Saúl Ibáñez Cerro
# Grado en Telemática
# Programa: mknotas.sh

if [ $# -ne "4" ]; then
	echo "Usage: $0 [file1] [file2] [file3] [file4]" 1>&2
	exit 1
fi

alumnos=`awk '{print $1}' $* | sort -u`

for i in $alumnos; do
	aprobado=`awk '$1 ~ /^'$i'$/ && $2 ~ /^si$/' $* | wc -l`
	if [ $aprobado -ge "2" ]; then
		echo $i si
	else
		echo $i no
	fi
done
