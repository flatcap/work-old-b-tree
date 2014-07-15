#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/file.h>
#include	"store.h"

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/

/*
	Test rack program for the record storage library. This allows some
	minimal interaction with record stores. All records are identified
	with the header of their offset. The only way to get a valid header
	is by alloc'ing a new record. Good luck. This is not worth documenting.
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/utils/RCS/rectest.c,v 1.1 89/10/24 10:09:27 mjr Rel $";
#endif


STORE	*globf = NULL;

#define	MAXARG	40
char	*globargv[MAXARG];		/* args for commands */
int	globargc;
char	globname[500];

extern	char	*strcpy();
extern	char	*malloc();
extern	char	*rindex();
extern	long	atol();
extern	float	atof();

void do_open(), do_close(), do_quit(), do_help();
void do_read(), do_write(), do_alloc(), do_pheader();
void do_increc(), do_decrec(), do_free(), do_copy();
void do_realloc(), do_linkrec(), do_unlink();

void	enargv();	/* function to parse user args into strings */
void	doit();		/* dispatch function */

static	char	*grokmsg = "Cannot interpret \"%s\" as a record number\n";


struct	cmd {
	char	*name;
	char	*usage;
	void	(*func)();
	int	narg;		/* # of args req'd */
	int	op;		/* needs an open index to call */
};

static	char	*prompt = "rectest-> ";

/* command dispatch table */
struct	cmd	cmds[] = {
"alloc",	"alloc nbytes",			do_alloc,	1,	1,
"close",	"close",			do_close,	1,	1,
"copy",		"copy fromrec# torec#",		do_copy,	2,	1,
"decrement",	"decrement rec# (ref count)",	do_decrec,	2,	1,
"free",		"free rec#",			do_free,	2,	1,
"increment",	"increment rec# (ref count)",	do_increc,	2,	1,
"link",		"link rec# after|before recd#",	do_linkrec,	3,	1,
"open",		"open database",		do_open,	2,	0,
"printheader",	"printheader rec#",		do_pheader,	2,	1,
"quit",		"quit",				do_quit,	1,	0,
"read",		"read rec#",			do_read,	2,	1,
"realloc",	"realloc rec# size",		do_realloc,	3,	1,
"unlink",	"unlink rec#",			do_unlink,	2,	1,
"write",	"write rec#",			do_write,	2,	1,
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
					(void)printf("no store file open.\n");
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
	if(globf != NULL) {
		(void)printf("try closing %s first, ok ?\n",globname);
		return;
	}

	globf = store_open(globargv[1],O_CREAT,0600);

	if(globf == NULL) {
		(void)printf("error opening store file ");
		perror(globargv[1]);
	} else {
		(void)printf("opened %s\n",globargv[1]);
		(void)strcpy(globname,globargv[1]);
	}
}


void
do_close()
{
	if(globf != NULL) {
		if(store_close(globf) == STO_OK) {
			(void)printf("closed OK\n");
			globf = NULL;
		} else
			(void)printf("error closing\n");
	} else {
		(void)printf("nothing open\n");
	}
}


void
do_alloc()
{
	int	siz;
	sto_ptr	ret;

	if((siz = atoi(globargv[1])) == 0) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}

	ret = store_alloc(globf,siz);
	if(ret != STO_NULL) {
		(void)printf("allocated record #%ld\n",ret);
	} else {
		store_perror(globf,"cannot allocate record");
	}
}


void
do_quit()
{
	if(globf != NULL) {
		(void)printf("closing %s\n",globname);
		if(store_close(globf) != STO_OK) {
			(void)printf("problems closing %s\n",globname);
			exit(1);
		}
	}
	exit(0);
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
do_read()
{
	char	buf[BUFSIZ];
	int	rv;
	int	byts;
	off_t	offset =0L;
	sto_ptr	r;

	if((r = atol(globargv[1])) == 0L) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}

	if(globargc > 2) {
		if((offset = atol(globargv[2])) == 0L) {
			(void)printf(grokmsg,globargv[1]);
			return;
		} else {
			(void)printf("offset is %ld\n",offset);
		}
	}

	while(1) {
		rv = store_read(globf,r,offset,(unsigned char *)buf,sizeof(buf),&byts);
		switch(rv) {
			case STO_OK:
				(void)printf("got %d bytes:\n",byts);
				buf[byts] = '\0';
				(void)printf("%s\n",buf);
				return;

			case STO_MORE:
				(void)printf("got %d bytes:\n",byts);
				buf[byts] = '\0';
				(void)printf("%s\n---(press return for more)---",buf);
				(void)gets(buf);
			
			case STO_ERR:
				store_perror(globf,"cannot read record");
				return;
		}
	}
}


void
do_write()
{
	char	buf[BUFSIZ];
	int	rv;
	off_t	offset =0L;
	sto_ptr	r;

	if((r = atol(globargv[1])) == 0L) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}

	if(globargc > 2) {
		if((offset = atol(globargv[2])) == 0L) {
			(void)printf(grokmsg,globargv[1]);
			return;
		} else {
			(void)printf("offset is %ld\n",offset);
		}
	}

	(void)printf("\tinput-> ");
	if(fgets(buf,BUFSIZ,stdin) != NULL) {
		rv = store_write(globf,r,offset,(unsigned char *)buf,strlen(buf));
		if(rv != STO_OK) {
			store_perror(globf,"cannot store record");
		}
	}
}

void
do_pheader()
{
	struct	sto_hdr	rbuf;
	sto_ptr	r;

	if((r = atol(globargv[1])) == 0L) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}

	if(store_gethdr(globf,r,&rbuf) == STO_ERR) {
		store_perror(globf,"cannot get record header");
		return;
	}
	(void)printf("struct\tsto_hdr {\n\toff_t\tmagic = %ld\n",rbuf.magic);
	(void)printf("\tint\tsize = %d\n\tint\tused = %d\n",rbuf.size,rbuf.used);
	(void)printf("\tint\trefs = %d\n\tsto_ptr\tnext = %d\n",rbuf.refs,rbuf.next);
	(void)printf("\tsto_ptr\tprev = %d\n};\n",rbuf.prev);
}


void
do_increc()
{
	sto_ptr	r;

	if((r = atol(globargv[1])) == 0L) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}

	if(store_increfcnt(globf,r) == STO_ERR) {
		store_perror(globf,"cannot increment record header");
		return;
	}
}


void
do_decrec()
{
	sto_ptr	r;

	if((r = atol(globargv[1])) == 0L) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}

	if(store_decrefcnt(globf,r) == STO_ERR) {
		store_perror(globf,"cannot decrement record header");
		return;
	}
}


void
do_free()
{
	sto_ptr	r;

	if((r = atol(globargv[1])) == 0L) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}

	if(store_free(globf,r) == STO_ERR) {
		store_perror(globf,"cannot free record");
		return;
	}
}


void
do_copy()
{
	sto_ptr	r1;
	sto_ptr	r2;

	if((r1 = atol(globargv[1])) == 0L) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}
	if((r2 = atol(globargv[2])) == 0L) {
		(void)printf(grokmsg,globargv[2]);
		return;
	}

	if(store_copy(globf,r1,r2) == STO_ERR) {
		store_perror(globf,"cannot copy record");
		return;
	}
}


void
do_realloc()
{
	sto_ptr	rec;
	int	nsize;

	if((rec = atol(globargv[1])) == 0L) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}
	if((nsize = atoi(globargv[2])) == 0L) {
		(void)printf(grokmsg,globargv[2]);
		return;
	}
	if((rec = store_realloc(globf,rec,nsize)) == STO_NULL) {
		store_perror(globf,"realloc failed");
		return;
	}
	(void)printf("new record is %ld\n",rec);
}


void
do_linkrec()
{
	sto_ptr	rec;
	sto_ptr	rec2;
	int	after = 1;
	int	ret;

	if((rec = atol(globargv[1])) == 0L) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}

	if(strncmp(globargv[2],"after",strlen(globargv[2])))
		if(strncmp(globargv[2],"before",strlen(globargv[2]))) {
			(void)printf("specify \"before\" or \"after\"\n");
			return;
		} else
			after = 0;

	if((rec2 = atol(globargv[3])) == 0L) {
		(void)printf(grokmsg,globargv[3]);
		return;
	}

	if(after)
		ret = store_linkafter(globf,rec,rec2);
	else
		ret = store_linkbefore(globf,rec,rec2);
	if(ret == STO_ERR) {
		store_perror(globf,"relink failed");
		return;
	}
}


void
do_unlink()
{
	sto_ptr	rec;

	if((rec = atol(globargv[1])) == 0L) {
		(void)printf(grokmsg,globargv[1]);
		return;
	}

	
	if(store_unlink(globf,rec) == STO_ERR) {
		store_perror(globf,"unlink failed");
		return;
	}
}
