#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/file.h>
#include	"btree.h"

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/

/*
	This program optimizes an index by recopying it into a
	new one using a reverse load
*/

extern	char	*malloc();

main(ac,av)
int	ac;
char	*av[];
{
	BT_INDEX	*old;
	BT_INDEX	*new;
	char		*infile;
	char		*outfile;
	int		outpsiz = -1;	/* output page size */
	off_t		rrnstore;
	int		lenstore;
	bt_chrp		bufptr;		/* have to allocate this dynamically */
					/* in case the tree has BIG pages */
	int		ret;

	for(ret = 1; ret < ac; ret++) {
			if(av[ret][0] == '-') 
			switch(av[ret][1]) {
				case	'p':
					outpsiz = atoi(av[ret + 1]);
					if(outpsiz == 0) {
						(void)fprintf(stderr,"%s: page size must be an integer > 0\n",av[0]);
						outpsiz = -1;
					}
					break;

				case	'o':
					outfile = av[ret + 1];
					break;

				case	'i':
					infile = av[ret + 1];
					break;

				default:
					exit(usage());
			}
	}

	if(outfile == NULL || infile == NULL)
		exit(usage());

#ifdef	BTOPEN
	old = bt_open(infile,O_RDONLY,0600);
#else
	old = bt_optopen(BT_PATH,	infile,
			BT_OMODE,	O_RDONLY,
	0);
#endif

	if(old == NULL) {
		perror(infile);
		exit(1);
	}

	/* if output page size was not set on the command line, adopt old */
	if(outpsiz == -1)
		outpsiz = bt_pagesiz(old);

	if((bufptr = (bt_chrp)malloc((unsigned)bt_pagesiz(old))) == NULL) {
		perror("cannot allocate buffer memory");
		(void)bt_close(old);
		exit(1);
	}

#ifdef	BTOPEN
	new = bt_open(av[2],O_CREAT|O_TRUNC,0600);
#else
	new = bt_optopen(BT_PATH,	outfile,
			BT_OMODE,	O_CREAT|O_TRUNC,
			BT_OPERM,	0600,
			BT_CACHE,	3,
			BT_PSIZE,	outpsiz,
			BT_CFLAG,	BT_RWCACHE,
	0);
#endif

	if(new == NULL) {
		perror(outfile);
		exit(1);
	}

	/* inform the new file we are going to bt_load it */
	if(bt_load(new,0,0,0L,BT_BOF)== BT_ERR) {
		bt_perror(new,"initialize load");
		exit(1);
	}

	/* since we have not performed any traverses yet, it will start */
	/* at the END of the index, automatically - nifty that, ey? */
	/* REMEMBER, this HAS to be REVERSE order !!! */
	while((ret = bt_traverse(old,BT_BOF,bufptr,bt_pagesiz(old),&lenstore,&rrnstore)) == BT_OK) {
		if(bt_load(new,bufptr,lenstore,rrnstore,BT_OK)== BT_ERR) {
			bt_perror(new,"bt_load");
			exit(1);
		}
	}
	if(ret != BT_BOF) {
		(void)fprintf(stderr,"warning: traverse did not complete at BOF!\n");
	}

	/* a final call to bt_load with BT_EOF tells it to stop */
	if(bt_load(new,0,0,0L,BT_EOF) == BT_ERR) {
		bt_perror(new,"shut down load");
		exit(1);
	}

	if(bt_close(old) != BT_OK || bt_close(new) != BT_OK) {
		(void)fprintf(stderr,"error closing indices\n");
		exit(1);
	}

	(void)free((char *)bufptr);
	exit(0);
}

usage()
{
	(void)fprintf(stderr,"usage: btoptim -i input -o output [-p page size]\n");
	(void)fprintf(stderr,"where input and output are b+tree indices, and page size a positive integer\n");
	return(1);
}
