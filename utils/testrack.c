#include	<stdio.h>
#include	<sys/file.h>
#include	<sys/types.h>
#include	"btree.h"

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/utils/RCS/testrack.c,v 1.1 89/10/24 10:09:25 mjr Rel $";
#endif

extern	char	*index();

/*
	about this program:
	This program slams the btree index routines repeatedly on thier
	heads in an attempt to break them, or detect problems. rather
	than make the checks rely on information about the internal
	structures of the tree itself, these checks verify the BEHAVIOUR
	of the library. that is to say, if 1000 keys are inserted, it
	should be possible to find them all. if 1000 keys are deleted,
	there should be no more keys left, etc. really, the only aspect
	of the functions that is not given a pretty harsh going-over is
	the reverse traversing function. this is mainly because I cant
	think of a nice way to scan an ascii file a line at a time in
	reverse, and didn't want to have another sorted input file.

	before this code has been released, this testrack has been
	allowed to run on a reasonable variety of page sizes for a
	reasonable number of keys (say, 10,000) for, say, a day or
	two, on a Sun4. if you make changes to the library internals
	this suite can come in pretty handy, though it offers zero
	help for debugging.
*/

static	void
die(b1,b2,val)
BT_INDEX	*b1;
BT_INDEX	*b2;
int		val;
{
	/* die is called with a sequentially higher value from each */
	/* possible point of failure. this allows quick location of */
	/* which check may have produced he fatal error condition */
	if(b1 != NULL)
		(void)bt_close(b1);
	if(b2 != NULL)
		(void)bt_close(b2);
	(void)fprintf(stderr,"tests FAILED at error point %d\n",val);
	exit(val);
}

main(ac, av)
int     ac;
char   *av[];
{
	char   		buf[BUFSIZ];
	char   		buf2[BUFSIZ];
	FILE		*input;
	FILE		*sorted;
	BT_INDEX	*b = NULL;	/* main test index */
	BT_INDEX	*d = NULL;	/* index to store deleted keys */
	char		*p;
	off_t  		rrv = 0L;
	int		ret;
	int		keycnt = 0;	/* # of keys inserted */
	int		psiz = BUFSIZ;
	int		retlen;
	int		junk;

	/* boo ! Sun buffers stderr ! */
	(void)setbuf(stderr,0);

	if(ac < 3) {
		(void)fprintf(stderr,"usage: %s input sorted_input [psiz]\n",av[0]);
		die(b,d,1);
	}

	if(ac > 3) {
		if((psiz = atoi(av[3])) == 0) {
			(void)fprintf(stderr,"cannot grok %s as a page size\n",av[3]);
			die(b,d,2);
		}
	}

	if((input = fopen(av[1],"r")) == NULL) {
		perror(av[1]);
		die(b,d,3);
	}

	if((sorted = fopen(av[2],"r")) == NULL) {
		perror(av[2]);
		die(b,d,4);
	}

#ifdef	BTOPEN
	b = bt_open("test.dat",O_CREAT|O_TRUNC,0600);
#else
	b = bt_optopen(BT_PATH,	"test.dat",
			BT_OMODE,	O_CREAT|O_TRUNC,	/* clobber */
			BT_CACHE,	2,
			BT_PSIZE,	psiz,
	0);
#endif

	if(b == NULL) {
		perror("test.dat");
		die(b,d,5);
	}

#ifdef	DEBUG
	(void)fprintf(stderr,"Start: opened all trees OK.\n");
#endif

	/* PHASE I - insert everything from our random-ordered file */
	/* into our newly created tree. This should return all BT_OK */
	while(fgets(buf,BUFSIZ,input) != NULL) {

		/* drop newline */
		if((p = index(buf,'\n')) != NULL)
			*p = '\0';

		/* stuff it */
		ret = bt_insert(b,buf,strlen(buf),++rrv,1);
		if(ret != BT_OK) {
			(void)fprintf(stderr,"insert \"%s\" failed: ",buf);
			bt_perror(b,NULL);
			die(b,d,6);
		}
		keycnt++;
	}
#ifdef	DEBUG
	(void)fprintf(stderr,"passed phase I, inserted %d keys OK.\n",keycnt);
#endif


	/* PHASE II - rewind to the beginning of the input file and */
	/* search for each key again, to make sure we can find it. */
	if(fseek(input,0L,0)) {
		perror("rewind random ordered input file");
		die(b,d,7);
	}
	keycnt = 0;
	while(fgets(buf,BUFSIZ,input) != NULL) {

		/* drop newline */
		if((p = index(buf,'\n')) != NULL)
			*p = '\0';

		/* find it */
		ret = bt_find(b,buf,strlen(buf),&rrv);
		if(ret != BT_OK) {
			(void)fprintf(stderr,"find \"%s\" failed: ",buf);
			bt_perror(b,NULL);
			die(b,d,8);
		}
		keycnt++;
	}
#ifdef	DEBUG
	(void)fprintf(stderr,"passed phase II, located %d keys OK.\n",keycnt);
#endif


	/* PHASE III - go to the beginning of the tree, and read each */
	/* key in order. as we read it, compare it to the matching key */
	/* read out of the sorted input file. they should all match */

	if(fseek(sorted,0L,0)) {
		perror("rewind ordered input file");
		die(b,d,9);
	}

	/* goto head of tree */
	if(bt_goto(b,BT_BOF) == BT_ERR) {
		bt_perror(b,"rewind btree to BOF");
		die(b,d,10);
	}

	while(fgets(buf,BUFSIZ,sorted) != NULL) {

		/* drop newline */
		if((p = index(buf,'\n')) != NULL)
			*p = '\0';

		ret = bt_traverse(b,BT_EOF,buf2,BUFSIZ,&retlen,&rrv);
		if(ret != BT_OK) {
			(void)fprintf(stderr,"read of next key failed.\n");
			die(b,d,11);
		}

		/* NULL terminate buf2 */
		buf2[retlen] = '\0';

		if(strcmp(buf2,buf)) {
			(void)fprintf(stderr,"read-back of \"%s\" and \"%s\" - do not match.\n",buf,buf2);
			die(b,d,12);
		}
	}
#ifdef	DEBUG
	(void)fprintf(stderr,"passed phase III, ordered read-back of keys.\n");
#endif


	/* PHASE IV - delete half the keys from the index */

	/* rewind input file */
	if(fseek(input,0L,0)) {
		perror("rewind input file");
		die(b,d,13);
	}

	/* open a second index to store deleted keys */
#ifdef	BTOPEN
	d = bt_open("test2.dat",O_CREAT|O_TRUNC,0600);
#else
	d = bt_optopen(BT_PATH,	"test2.dat",
			BT_OMODE,	O_CREAT|O_TRUNC,	/* clobber */
			BT_CACHE,	2,
			BT_PSIZE,	psiz,
	0);
#endif
	if(d == NULL) {
		perror("test2.dat");
		die(b,d,14);
	}
	for(junk = 0; junk < keycnt/2; junk++) {
		if(fgets(buf,BUFSIZ,input) == NULL) {
			(void)fprintf(stderr,"EOF in read-back of initial input\n");
			die(b,d,15);
		}

		/* drop newline */
		if((p = index(buf,'\n')) != NULL)
			*p = '\0';

		/* see if the key is a duplicate that was already deleted */
		/* and if so, do not try to re-delete it */
		ret = bt_find(d,buf,strlen(buf),&rrv);
		if(ret == BT_NF) {

			/* not in the deleted index, so delete it and add it */
			/* step 1: delete from real index */
			if(bt_delete(b,buf,strlen(buf)) != BT_OK) {
				bt_perror(b,"delete failed\n");
				die(b,d,16);
			}
			/* step 2: add it to the deleted index */
			if(bt_insert(d,buf,strlen(buf),-1L,1) != BT_OK) {
				bt_perror(d,"insert in deleted failed\n");
				die(b,d,17);
			}
		} else {
			if(ret != BT_OK) {
				bt_perror(d,"search for deleted failed\n");
				die(b,d,18);
			}
		}
	}
#ifdef	DEBUG
	(void)fprintf(stderr,"passed phase IV, delete half of keys.\n");
#endif


	/* PHASE V - go to the beginning of the tree, and read each */
	/* key in order. as we read it, compare it to the matching key */
	/* read out of the sorted input file. they should all match */
	/* OR it should be in the deleted index, its an error */

	if(fseek(sorted,0L,0)) {
		perror("rewind ordered input file");
		die(b,d,19);
	}

	/* goto head of tree */
	if(bt_goto(b,BT_BOF) == BT_ERR) {
		bt_perror(b,"rewind btree to BOF");
		die(b,d,20);
	}

	while(fgets(buf,BUFSIZ,sorted) != NULL) {

		/* drop newline */
		if((p = index(buf,'\n')) != NULL)
			*p = '\0';

		ret = bt_find(d,buf,strlen(buf),&rrv);
		if(ret == BT_NF) {
			ret = bt_traverse(b,BT_EOF,buf2,BUFSIZ,&retlen,&rrv);

			/* NULL terminate buf2 */
			buf2[retlen] = '\0';


			if(strcmp(buf2,buf)) {
				(void)fprintf(stderr,"read-back of \"%s\" and \"%s\" - do not match.\n",buf,buf2);
				die(b,d,21);
			}
		} else {
			if(ret != BT_OK) {
				(void)fprintf(stderr,"search deleted for \"%s\":",buf);
				bt_perror(d,NULL);
				die(b,d,22);
			}
		}
	}
#ifdef	DEBUG
	(void)fprintf(stderr,"passed phase V, ordered read-back of keys.\n");
#endif


	/* PHASE VI - delete the rest of the keys in the input file */

	/* at this point the FILE pointer should still be where we left */
	/* it when we were done deleteing the first half of the keys in */
	/* the index. */
	while(fgets(buf,BUFSIZ,input) != NULL) {

		/* drop newline */
		if((p = index(buf,'\n')) != NULL)
			*p = '\0';

		/* see if the key is a duplicate that was already deleted */
		/* and if so, do not try to re-delete it */
		ret = bt_find(d,buf,strlen(buf),&rrv);
		if(ret == BT_NF) {

			/* not in the deleted index, so delete it and add it */
			/* step 1: delete from real index */
			if(bt_delete(b,buf,strlen(buf)) != BT_OK) {
				bt_perror(b,"delete failed\n");
				die(b,d,23);
			}
			/* step 2: add it to the deleted index */
			if(bt_insert(d,buf,strlen(buf),-1L,1) != BT_OK) {
				bt_perror(d,"insert in deleted failed\n");
				die(b,d,24);
			}
		} else {
			if(ret != BT_OK) {
				bt_perror(d,"search for deleted failed\n");
				die(b,d,25);
			}
		}
	}
#ifdef	DEBUG
	(void)fprintf(stderr,"passed phase VI, delete rest of keys.\n");
#endif

	/* PHASE VII - make sure there are no more keys in original index */
	/* seek to beginning of index and try to traverse. it should give */
	/* an EOF immediately. any other result is an error */
	/* goto head of tree */
	if(bt_goto(b,BT_BOF) == BT_ERR) {
		bt_perror(b,"rewind btree to BOF");
		die(b,d,26);
	}
	ret = bt_traverse(b,BT_EOF,buf2,BUFSIZ,&retlen,&rrv);
	if(ret != BT_EOF) {
		if(ret == BT_ERR) {
			bt_perror(b,"traverse to EOF");
			die(b,d,27);
		}
		(void)fprintf(stderr,"expected EOF, but didn't get it\n");
		die(b,d,28);
	} 
#ifdef	DEBUG
	(void)fprintf(stderr,"passed phase VII, tree is empty!\n");
#endif

	/* PHASE VIII - do an optimal reverse load of the deleted tree */
	/* back into the original tree */
	if(bt_goto(d,BT_EOF) == BT_ERR) {
		bt_perror(b,"rewind deleted btree to EOF");
		die(b,d,29);
	}

	/* inform the old tree we are going to bt_load it */
	if(bt_load(b,0,0,0L,BT_BOF)== BT_ERR) {
		bt_perror(b,"initialize load");
		die(b,d,30);
	}

	while((ret = bt_traverse(d,BT_BOF,buf,BUFSIZ,&retlen,&rrv)) == BT_OK) {
		if(bt_load(b,buf,retlen,rrv,BT_OK)== BT_ERR) {
			bt_perror(b,"bt_load failed!");
			die(b,d,31);
		}
	}

	if(ret != BT_BOF) {
		(void)fprintf(stderr,"deleted tree traverse did not complete at BOF!\n");
		die(b,d,32);
	}

	/* a final call to bt_load with BT_EOF tells it to stop */
	if(bt_load(b,0,0,0L,BT_EOF) == BT_ERR) {
		bt_perror(b,"shut down bt_load");
		die(b,d,33);
	}

#ifdef	DEBUG
	(void)fprintf(stderr,"passed phase VIII, optimal load\n");
#endif

	/* PHASE IX - rewind to the beginning of the input file and */
	/* search for each key again, to make sure we can find it. */
	if(fseek(input,0L,0)) {
		perror("rewind random ordered input file");
		die(b,d,34);
	}
	keycnt = 0;
	while(fgets(buf,BUFSIZ,input) != NULL) {

		/* drop newline */
		if((p = index(buf,'\n')) != NULL)
			*p = '\0';

		/* find it */
		ret = bt_find(b,buf,strlen(buf),&rrv);
		if(ret != BT_OK) {
			(void)fprintf(stderr,"find \"%s\" failed: ",buf);
			bt_perror(b,NULL);
			die(b,d,35);
		}
		keycnt++;
	}
#ifdef	DEBUG
	(void)fprintf(stderr,"passed phase IX, located %d keys OK.\n",keycnt);
#endif

	(void)bt_close(b);
	(void)bt_close(d);
#ifdef	DEBUG
	(void)fprintf(stderr,"passed all tests OK.\n");
#endif
	exit(0);
}
