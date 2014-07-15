#include	<stdio.h>

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


extern	long	random();
extern	long	time();

main(ac,av)
int	ac;
char	*av[];
{
	char	buf[BUFSIZ];
	int	todo;
	int	maxlen = 50;
	int	len;
	long	junk;
	int	x;
	int	y;

	(void)time(&junk);
	(void)srandom((int)junk);

	if(ac < 3) {
		(void)fprintf(stderr,"usage: words count [len]\n");
		exit(1);
	}

	/* how many inserts to perform */
	todo = atoi(av[1]);

	if(ac > 2) {
		maxlen = atoi(av[2]);
		if(maxlen < 1) {
			(void)fprintf(stderr,"max len less than 1 is stupid!\n");
			exit(1);
		}
	}

	for(x = 0; x < todo; x++) {
		len = ((int)random() % maxlen) + 1;
		for(y = 0; y < len; y++)
			buf[y] = ((int)random() % 25) + 'a';		

		buf[len] = '\0';
		(void)printf("%s\n",buf);
	}
	exit(0);
}
