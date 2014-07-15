#
#/*
#         (C) Copyright, 1988, 1989 Marcus J. Ranum
#                    All rights reserved
#
#
#          This software, its documentation,  and  supporting
#          files  are  copyrighted  material  and may only be
#          distributed in accordance with the terms listed in
#          the COPYRIGHT document.
#*/

cnt=1
keys=2000
lengths=8
pagesiz=61
while [ 1  = 1 ]; do
	echo "$keys keys for #$cnt ..."
	words $keys $lengths > input
	sort input | uniq > input.srt
	echo "running #$cnt ..."
	testrack input input.srt $pagesiz
	if [ $? != 0 ]; then
		echo " failed!"
		echo "input producing the failure is left in \"input\""
		exit 1
	fi
	echo "passed"

	cnt=`expr $cnt + 1`
done
