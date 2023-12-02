#!/bin/sh
LOADER=../ldelf/ldelf
for i in `seq -w 0 30`
do
	f="b$i.x"
	if test -f $f
	then
		n=`echo $i | sed 's/^0\+//'`
		$LOADER ./$f
		o="$?"
		if test $o != $n
		then
			echo "$f	$o"
		fi
	fi
done

for x in t??.x
do
	$LOADER ./$x
	o=$?
	if test "$o" != "0"
	then
		echo "$x	$o"
	fi
done
