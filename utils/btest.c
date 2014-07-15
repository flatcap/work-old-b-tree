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
	this program does pretty much one of everything to do with the
	b+tree library. it does the usual deletes, scans, dumps, etc,
	and shows a few of the tricks that can be done with user defined
	data types, builtin data types, and so forth. no attempt is
	made to make the code gorgeous, rather simple and readable.

	in a real application, some of the things done here should NOT
	be done. the dump structure code and page dump code relies on
	internal stuff. for real user-level applications, anything
	that includes btinternal.h is making a BIG mistake and is
	poking its nose where it should not. users should not make
	calls to bt_rpage() unless they are doing something that has
	been carefully thought out. besides, that internal code
	could change without warning. every attempt will be made to
	keep the interface to the user-level functions the same,
	despite any internal mods.

	Enjoy!
		--mjr();
*/

/* only included because this uses some internal routines */
#include	"btconf.h"
#include	"btintern.h"

#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/utils/RCS/btest.c,v 1.1 89/10/24 10:09:28 mjr Rel $";
#endif

/*	do not judge the library code by this humble test harness	*/
/*	the globals are simply a convenience, here where it matters not	*/
BT_INDEX	*globf = NULL;	/* global index handle */

#define	MAXARG	40
char	*globargv[MAXARG];		/* args for commands */
int	globargc;
char	globname[500];

/* for tweaking, using big pages is boring */
#define	TESTPSIZ	61

extern	char	*strcpy();
extern	char	*malloc();
extern	char	*rindex();
extern	long	atol();
extern	float	atof();

void do_open(), do_close(), do_quit(), do_wlabel(), printk();
void do_insert(), do_help(), do_zap(), do_stats(), do_revload();
void do_find(), do_head(), do_tail(), do_dump(), do_load();
void do_next(), do_prev(), do_rlabel(), do_delete();

void	enargv();	/* function to parse user args into strings */
void	doit();		/* dispatch function */
caddr_t	encode();	/* user argument encoder funct. */

/* this is the comparison function needed for the sample user-defined data */
int mycompare();

/* and this is the sample data structure */
struct	point	{
	int	xcoord;
	int	ycoord;
};

#ifdef	YES_BT_DEBUG
void do_pagedump(), do_struct();
#endif

struct	cmd {
	char	*name;
	char	*usage;
	void	(*func)();
	int	narg;		/* # of args req'd */
	int	op;		/* needs an open index to call */
};

static	char	*prompt = "btest->";

/* command dispatch table */
struct	cmd	cmds[] = {
"close",	"close",			do_close,	1,	1,
"delete",	"delete key [key2...]",		do_delete,	2,	1,
"dump",		"dump [-r]",			do_dump,	1,	1,
"exit",		"exit",				do_quit,	1,	0,
"find",		"find key",			do_find,	2,	1,
"head",		"head",				do_head,	1,	1,
"help",		"help",				do_help,	1,	0,
"insert",	"insert key [rrn]",		do_insert,	2,	1,
"label",	"label string",			do_wlabel,	2,	1,
"load",		"load file (unsorted input)",	do_load,	2,	1,
"next",		"next [count]",			do_next,	1,	1,
"open",		"open database [type] [size]",	do_open,	2,	0,
#ifdef	YES_BT_DEBUG
"page",		"page page# [page#...]",	do_pagedump,	2,	1,
#endif
"plabel",	"plabel (show label)",		do_rlabel,	1,	1,
"prev",		"prev [count]",			do_prev,	1,	1,
"quit",		"quit",				do_quit,	1,	0,
"revload",	"revload file (rev sorted)",	do_revload,	2,	1,
"stats",	"stats (print internal data)",	do_stats,	1,	1,
"tail",		"tail",				do_tail,	1,	1,
#ifdef	YES_BT_DEBUG
"x",		"x (dump struct)",		do_struct,	1,	1,
#endif
"zap",		"zap (toast current index)",	do_zap,		1,	1,
"?",		"?",				do_help,	1,	0,
0,		0,				0,		0,	0
};


main(ac,av)
int	ac;
char	**av;
{
	int	x;
	char	buf[BUFSIZ];

	/* enargv() makes a string look like an arg. vector. */
	/* doit dispatches the stuff, calls functions, etc */
	/* functions must have at least the given # of args */
	/* last flag in a command if not zero means an index */
	/* must be open in order to call that function */
	
	if(ac > 1) {
		char	**gap = globargv;
	
		globargc = ac - 1;
		while(*++av)
			*gap++ = *av;
		doit();
	} else {
		(void)printf(prompt);
		while(gets(buf)) {
			enargv(buf);
			if(globargv[0] != NULL)
				doit();

			for(x = 0; x < globargc; x++)
				(void)free(globargv[x]);

			(void)printf(prompt);
		}
		(void)printf("EOF.\n");
	}
	do_quit();
}


void
doit()
{
	struct	cmd	*c = cmds;

	/* main dispatcher */
	while(c->name != 0) {
		if(strncmp(c->name,globargv[0],strlen(globargv[0])) == 0) {
			if(globargc < c->narg) {
				(void)printf("usage:\"%s\"\n",c->usage);
				return;
			} else {
				if(c->op != 0 && globf == NULL) {
					(void)printf("no index open.\n");
					return;
				}
				(*c->func)();
				return;
			}
		}
		c++;
	}
	(void)printf("unknown command: \"%s\"\n",globargv[0]);
	return;
}


char *
wordparse(line,word,delim)
char	*line,	*word;
int	delim;
{
	int	quot =0;

	/* parse a word, or quoted string into a buffer. */
	while (*line && (isspace(*line) || *line == delim))
		line++;

	if(*line == '\n')
		line++;

	if (!*line) {
		*word = '\0';
		return(0);
	}

	while (*line)
	{
		if(!quot && (*line == '\"' || *line == '\''))
			quot = *line++;

		if((isspace(*line) || *line == delim) && !quot) {
			break;
		} else {
			if(quot && *line == quot) {
				quot = 0;
				line++;
			} else {
				*word++ = *line++;
			}
		}
	}
	*word = '\0';
	return(line);
}


void
enargv(buf)
char	*buf;
{
	char	*bufptr;
	char	pbuf[BUFSIZ];
	
	globargc =0;
	bufptr = buf;

	/* this is kinda gross, but the easiest way to handle */
	/* quoted strings, since I already had wordparse() written */
	while(bufptr = wordparse(bufptr,pbuf,0)) {
		globargv[globargc] = malloc((unsigned)(strlen(pbuf) + 1));
		(void)strcpy(globargv[globargc++],pbuf);
	}
	globargv[globargc] = NULL;
}


void
do_open()
{
	int	ps = TESTPSIZ;
	int	typ = BT_DFLTDTYPE;

	if(globf != NULL) {
		(void)printf("try closing %s first, ok ?\n",globname);
		return;
	}

#ifdef	BTOPEN
	if(globargc > 2) {
		(void)printf("cannot specify data type when using bt_open()\n");
		return;
	}
#else
	if(globargc > 2) {
		if(!strcmp(globargv[2],"int")) {
			typ = BT_INT;
		} else if(!strcmp(globargv[2],"float")) {
			typ = BT_FLOAT;
		} else if(!strcmp(globargv[2],"long")) {
			typ = BT_LONG;
		} else if(!strcmp(globargv[2],"string")) {
			typ = BT_STRING;
		} else if(!strcmp(globargv[2],"userdef")) {
			typ = BT_USRDEF;
		} else {
			(void)printf("unknown data type \"%s\" - not opened\n",globargv[2]);
			(void)printf("use: float, string, long, int, or userdef\n");
			return;
		}
	}
#endif

	if(globargc > 3) {
		if((ps = atoi(globargv[3])) == 0) {
			(void)printf("invalid page size 0\n");
			return;
		}
	}

#ifdef	BTOPEN
	globf = bt_open(globargv[1],O_CREAT,0600);
#else
	if(typ != BT_USRDEF) {
		/* if user-defined we need to pass compare */
		/* func, too - hence this special case code */
		globf = bt_optopen(BT_PATH,	globargv[1],
				BT_OMODE,	O_CREAT,
				BT_DTYPE,	typ,
				BT_PSIZE,	ps,
		0);
	} else {
		globf = bt_optopen(BT_PATH,	globargv[1],
				BT_OMODE,	O_CREAT,
				/* note - pass the comparison function */
				BT_DTYPE,	typ,	mycompare,
				BT_PSIZE,	ps,
		0);
	}
#endif

	if(globf == NULL) {
		(void)printf("error opening database ");
		perror(globargv[1]);
		(void)printf("\n");
	} else {
		(void)printf("opened %s, type %d, with page size %d\n",globargv[1],bt_dtype(globf),bt_pagesiz(globf));
		(void)strcpy(globname,globargv[1]);
	}

	/* in the case where we are opening a user defined data type */
	/* tree, that already exists, the pointer will not be set, */
	/* because of the logic above (IE - it wont know its a userdef */
	/* until it opens) so we set it using a macro from btree.h */
	if(globargc == 2 && bt_dtype(globf) == BT_USRDEF)
		bt_cmpfn(globf) = mycompare;
}


void
do_close()
{
	if(globf != NULL) {
		if(bt_close(globf) == BT_OK)
			(void)printf("closed OK\n");
		else
			(void)printf("error closing\n");
	} else {
		(void)printf("nothing open\n");
	}
	globf = NULL;
}


#ifdef	YES_BT_DEBUG
void
do_pagedump()
{
	struct	bt_cache *p;
	int	x;
	off_t	num;

	for(x = 1; x < globargc; x++) {
		num = (off_t)atol(globargv[x]);
		if(num == 0L) {
			(void)printf("cannot read \"%s\" as a page #\n",globargv[x]);
		} else {
			if((p = bt_rpage(globf,num)) == NULL) {
				(void)printf("cannot read page %ld\n",num);
				return;
			} else {
				bt_dump(globf,p);
			}
		}
	}
}
#endif


void
do_quit()
{
	if(globf != NULL) {
		(void)printf("closing %s\n",globname);
		if(bt_close(globf) != BT_OK) {
			(void)printf("problems closing %s\n",globname);
			exit(1);
		}
	}
	exit(0);
}


void
do_insert()
{
	caddr_t	ptr;
	off_t	rrn;
	static	off_t	rrnval = 1L;
	int	len;

	if((globargc > 2 && bt_dtype(globf) != BT_USRDEF))
		rrn = atol(globargv[2]);
	else
		rrn = rrnval++;

	ptr = encode(globargv[1],globargv[2],&len);

	if(bt_insert(globf,ptr,len,rrn,0)== BT_ERR) {
		bt_perror(globf,"error in insert");
		return;
	}
}


void
do_revload()
{
	caddr_t	ptr;
	off_t	rrnval = 1L;
	int	len;
	FILE	*inp;
	char	*p;
	char	fuf[BUFSIZ];

	if((inp = fopen(globargv[1],"r")) == NULL) {
		perror(globargv[1]);
		return;
	}

	/* call #1 to bt_load with flag BT_BOF to tell it to start */
	if(bt_load(globf,0,0,0L,BT_BOF)== BT_ERR) {
		bt_perror(globf,"cannot initialize btree load");
		(void)fclose(inp);
		return;
	}

	while(fgets(fuf,sizeof(fuf),inp) != NULL) {
		if((p = rindex(fuf,'\n')) != NULL)
			*p = '\0';

		/* files of user def stuff must be tab separated */
		/* for the purposes of this example. */
		if(bt_dtype(globf) == BT_USRDEF) {
			if((p = rindex(fuf,'\t')) != NULL) {
				*p++ = '\0';
			} else {
				(void)fprintf(stderr,"user-defined data must be tabbed when reading from a file\n");
				continue;
			}
		}
	
		ptr = encode(fuf,p,&len);
		if(bt_load(globf,ptr,len,rrnval++,BT_OK)== BT_ERR) {
			bt_perror(globf,"error in load");
			break;
		} else
			(void)printf(".");

	}

	/* a final call to bt_load with BT_EOF tells it to stop */
	if(bt_load(globf,0,0,0L,BT_EOF) == BT_ERR) {
		bt_perror(globf,"cannot shut down load");
		(void)fclose(inp);
		return;
	}

	(void)fclose(inp);
	(void)printf("\ndone\n");
}


void
do_load()
{
	caddr_t	ptr;
	off_t	rrnval = 1L;
	int	len;
	FILE	*inp;
	char	*p;
	char	fuf[BUFSIZ];

	if((inp = fopen(globargv[1],"r")) == NULL) {
		perror(globargv[1]);
		return;
	}

	while(fgets(fuf,sizeof(fuf),inp) != NULL) {
		if((p = rindex(fuf,'\n')) != NULL)
			*p = '\0';

		/* files of user def stuff must be tab separated */
		/* for the purposes of this example. */
		if(bt_dtype(globf) == BT_USRDEF) {
			if((p = rindex(fuf,'\t')) != NULL) {
				*p++ = '\0';
			} else {
				(void)fprintf(stderr,"user-defined data must be tabbed when reading from a file\n");
				continue;
			}
		}
	
		ptr = encode(fuf,p,&len);
		if(bt_insert(globf,ptr,len,rrnval++,1)== BT_ERR) {
			bt_perror(globf,"\nerror in insert");
			break;
		} else
			(void)printf(".");
	}
	(void)fclose(inp);
	(void)printf("\ndone\n");
}


void
do_help()
{
	struct	cmd	*c = cmds;
	(void)printf("short list of commands::\n------------------------\n");
	while(c->name != 0) {
		(void)printf("%s\n",c->usage);
		c++;
	}
}


void
do_zap()
{
	if(bt_zap(globf) == BT_ERR) {
		bt_perror(globf,"zap");
		return;
	}
	(void)printf("zapped %s\n",globname);
}


void
do_stats()
{
	(void)printf("superblock:\n");
	(void)printf("struct\tbt_super {\n\tlong\tmagic=%ld;\n",globf->sblk.magic);
	(void)printf("\tint\tpsiz=%d;\n",bt_pagesiz(globf));
	(void)printf("\tint\tdtype=%d;\n",bt_dtype(globf));
	(void)printf("\tint\tlevs=%d;\n",globf->sblk.levs);
	(void)printf("\toff_t\troot=%ld;\n",globf->sblk.root);
	(void)printf("\toff_t\tfree=%ld;\n",globf->sblk.free);
	(void)printf("\toff_t\thigh=%ld;\n};\n",globf->sblk.high);

	(void)printf("struct\tbt_index {\n\tint\tfd=%d;\n",bt_fileno(globf));
	(void)printf("\tchar\tdirt=%d;\n",globf->dirt);
	(void)printf("\tint\tcflg=%d;\n",globf->cflg);
	(void)printf("\tint\tcky=%d;\n",globf->cky);
	(void)printf("\toff_t\tcpag=%ld;\n",globf->cpag);
	(void)printf("\tint\tshih=%d;\n};\n",globf->shih);
}


void
do_find()
{
	off_t	rrnval;
	caddr_t	ptr;
	int	len;

	ptr = encode(globargv[1],globargv[2],&len);

	switch(bt_find(globf,ptr,len,&rrnval)) {
		case BT_OK:
			(void)printf("found with pointer val=%ld\n",rrnval);
			break;

		case BT_NF:
			(void)printf("key not found.\n");
			break;

		case BT_ERR:
			bt_perror(globf,"find failed");
			break;

		default:
			(void)printf("this should never happen!\n");
	}
}


void
do_goto(whence)
int	whence;
{
	if(bt_goto(globf,whence) == BT_ERR)
		bt_perror(globf,"error - cannot seek to EOF/BOF of tree!");


}


void
do_tail()
{
	do_goto(BT_EOF);
}


void
do_head()
{
	do_goto(BT_BOF);
}


void
do_traverse(whence)
int	whence;
{
	char	buf[BUFSIZ];
	int	retlen;
	off_t	rrv;
	int	x;
	int	cnt;
	int	ret;

	/* users should not probably look at this */
	if(globf->cpag == BT_NULL)
		(void)printf("(no currently defined location in tree)\n");

	if(globargc == 1)
		cnt = 1;
	else
		cnt = atoi(globargv[1]);

	for(x = 0; x < cnt; x++) {
		ret = bt_traverse(globf,whence,buf,BUFSIZ,&retlen,&rrv);
		switch(ret) {
			case BT_ERR:
				bt_perror(globf,"cannot traverse to key!");
				return;

			case BT_EOF:
				(void)printf("EOF.\n");
				break;

			case BT_BOF:
				(void)printf("BOF.\n");
				break;

			case BT_OK:
				printk(buf,retlen,rrv);
				break;

			default:
				(void)printf("this should never happen!\n");
		}

		if(ret == whence)
			return;
	}
}


void
do_next()
{
	do_traverse(BT_EOF);
}


void
do_prev()
{
	do_traverse(BT_BOF);
}


void
do_dump()
{
	char	buf[BUFSIZ];
	int	ret;
	int	retlen;
	off_t	junk;
	int	d;

	if(globargc > 1) {
		d = BT_BOF;
		if(bt_goto(globf,BT_EOF) == BT_ERR) {
			bt_perror(globf,"cannot goto EOF");
			return;
		}
	} else {
		d = BT_EOF;
		if(bt_goto(globf,BT_BOF) == BT_ERR) {
			bt_perror(globf,"cannot goto BOF");
			return;
		}
	}

	while((ret = bt_traverse(globf,d,buf,BUFSIZ,&retlen,&junk)) == BT_OK) {
		printk(buf,retlen,junk);
	}
	if(ret == BT_ERR)
		bt_perror(globf,"error traversing!");
}


#ifdef	YES_BT_DEBUG
void
do_struct()
{
	off_t	rrv;
	struct	bt_cache *p;

	(void)printf("levels:%d\t\troot:%ld\n",globf->sblk.levs,globf->sblk.root);


	rrv = bt_pagesiz(globf);
	(void)printf("Index Pages:\n");
	while(rrv < globf->sblk.high && ((p = bt_rpage(globf,rrv)) != NULL)) {
		if(HIPT(p->p) != BT_NULL && HIPT(p->p) != BT_FREE)
			bt_dump(globf,p);
		rrv += bt_pagesiz(globf);
	}

	rrv = bt_pagesiz(globf);
	(void)printf("\nLeaf Pages:\n");
	while(rrv < globf->sblk.high && ((p = bt_rpage(globf,rrv)) != NULL)) {
		if(HIPT(p->p) == BT_NULL)
			bt_dump(globf,p);
		rrv += bt_pagesiz(globf);
	}
}
#endif


void
do_wlabel()
{
	/* write label size + 1 to include null terminator - hack */
	if(bt_wlabel(globf,globargv[1],strlen(globargv[1]) + 1) == BT_ERR) {
		bt_perror(globf,"cannot write label");
		return;
	}
}


void
do_rlabel()
{
	char	lbuf[BUFSIZ];

	/* sample read-back of label */
	if(bt_rlabel(globf,lbuf,BUFSIZ) == BT_ERR) {
		bt_perror(globf,"cannot read label");
		return;
	}

	(void)printf("tree label is \"%s\"\n",lbuf);
}


void
do_delete()
{
	int	len;
	caddr_t	ptr;

	ptr = encode(globargv[1],globargv[2],&len);
	switch(bt_delete(globf,ptr,len)) {
		case BT_OK:
			(void)printf("deleted.\n");
			break;

		case BT_NF:
			(void)printf("key not found.\n");
			break;

		case BT_ERR:
			bt_perror(globf,"delete failed");
			break;

		default:
			(void)printf("this should never happen!\n");
	}
}


caddr_t
encode(b1,b2,lp)
char	*b1;
char	*b2;
int	*lp;
{
	static	int	iv;
	static	float	fv;
	static	long	lv;
	static	struct	point	p;

	/* since this test program uses a bunch of data types, this */
	/* function interprets the arguments based on what the user */
	/* enters, places them in a buffer of the appropriate type */
	/* and returns a pointer to it. this is somewhat gross, but */
	/* for the purposes of example should suffice. presumably */
	/* "real" applications wont need to do all this kinda stuff */
	/* the encoded length is stashed in the provided pointer */

	switch(bt_dtype(globf)) {
		case	BT_STRING:
			/* interpret input as a string. just return it */
			*lp = strlen(b1);
			return(b1);

		case	BT_INT:
			iv = atoi(b1);
			*lp = (int)sizeof(int);
			return((caddr_t)&iv);
			break;

		case	BT_LONG:
			lv = atol(b1);
			*lp = (int)sizeof(long);
			return((caddr_t)&lv);
			break;

		case	BT_FLOAT:
			fv = atof(b1);
			*lp = (int)sizeof(float);
			return((caddr_t)&fv);
			break;

		case	BT_USRDEF:
			/* this is somewhat gross, but you get the idea */
			if(b1 != NULL)
				p.xcoord = atoi(b1);
			else
				p.xcoord = 0;
			if(b2 != NULL)
				p.ycoord = atoi(b2);
			else
				p.ycoord = 0;
			*lp = (int)sizeof(p);
			return((caddr_t)&p);
	}
	return(NULL);
}


void
printk(buf,rl,rv)
caddr_t	buf;
int	rl;
off_t	rv;
{
	struct	point *p;
	char	*cp;

	switch(bt_dtype(globf)) {
		case	BT_STRING:
			buf[rl] = '\0';
			cp = buf;
			(void)printf("key=\"");
			while(*cp != '\0') {
				if(isprint(*cp))
					(void)printf("%c",*cp);
				else
					(void)printf("(0x%x)",*cp);
				cp++;
			}
			(void)printf("\",len=%d,rrv=%ld",rl,rv);
			break;

		case	BT_INT:
			(void)printf("key=%d,rrv=%ld",*(int *)buf,rv);
			break;

		case	BT_LONG:
			(void)printf("key=%ld,rrv=%ld",*(long *)buf,rv);
			break;

		case	BT_FLOAT:
			(void)printf("key=%f,rrv=%ld",*(float *)buf,rv);
			break;

		case	BT_USRDEF:
			/* here we are pretending it is a struct point* */
			p = (struct point *)&buf[0];
			(void)printf("point=(%d,%d),rrv=%ld",p->xcoord,p->ycoord,rv);
			break;
	}
	(void)printf("\n");
}


/* sample comparison function for user defined data type. NOTE - I have */
/* kinda played loose here, since s1 and s2 are really caddr_ts not pointers */
/* to struct points, but this should work just fine. kinda sloppy, though. */
mycompare(s1,l1,s2,l2)
struct point 	*s1;
int	l1;
struct point	*s2;
int	l2;
{
	/* since we are using a struct, the lengths are irrelevant */
	if(s1->xcoord == s2->xcoord) {
		if(s1->ycoord == s2->ycoord)
			return(0);
		else
			return(s1->ycoord - s2->ycoord);
	}
	return(s1->xcoord - s2->xcoord);
}
