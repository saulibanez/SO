#!/bin/bash

# Autor: Saúl Ibáñez Cerro
# Grado en Telemática
# Programa: renword.sh

if [ $# -ne 1 ]; then
	echo "Usage: $0 [word]" 1>&2
	exit 1
fi


for i in `ls`; do
	if [ -f $i ]; then
		if grep '^'$1 $i > /dev/null 2>&1; then
			mv $i $i.$1
		fi
	fi
done