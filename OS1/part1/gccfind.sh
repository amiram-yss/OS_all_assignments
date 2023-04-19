#!/bin/bash
#Amiram Yassif 314985474

#Check for 2 params to the least
if [[ $2 == "" ]]
then
    echo 'Not enough parameters'
    exit
fi

#Remove all compiled .out files
rm -f $1/*.out

#Comile each .c with the keyword inside.
for i in $(ls $1/*.c 2>/dev/null);
do
    if grep -q -w $2 $i
    then
        gcc $i -w -o ${i::-2}.out
    fi
done

#Active recoursively in case of -r flag
if [[ $3 == "-r" ]]
then
    for d in $(ls -d $1/*/ 2>/dev/null) ; do
        $0 ${d::-1} $2 -r
    done
fi
