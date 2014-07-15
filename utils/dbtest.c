#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/file.h>
#include	"btdbm.h"

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/utils/RCS/dbtest.c,v 1.1 89/10/24 10:09:28 mjr Rel $";
#endif


BTDBM	*globf = NULL;

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
void do_store(), do_fetch(), do_delete(), do_first();
void do_next();

void	enargv();	/* function to parse user args into strings */
void	doit();		/* dispatch function */

struct	cmd {
	char	*name;
	char	*usage;
	void	(*func)();
	int	narg;		/* # of args req'd */
	int	op;		/* needs an open index to call */
};

static	char	*prompt = "btdbmtest-> ";

/* command dispatch table */
struct	cmd	cmds[] = {
"close",	"close",			do_close,	1,	1,
"delete",	"delete key",			do_delete,	2,	0,
"fetch",	"fetch key",			do_fetch,	2,	0,
"first",	"first",			do_first,	1,	0,
"next",		"next",				do_next,	1,	0,
"open",		"open database",		do_open,	2,	0,
"quit",		"quit",				do_quit,	1,	0,
"store",	"store key data",		do_store,	3,	1,
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

	globf = btdbm_open(globargv[1],BT_STRING,O_CREAT,0600);

	if(globf == NULL) {
		(void)printf("error opening database");
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
		if(btdbm_close(globf)) {
			(void)printf("error closing\n");
		} else {
			(void)printf("closed OK\n");
			globf = NULL;
		}
	} else {
		(void)printf("nothing open\n");
	}
}


void
do_quit()
{
	if(globf != NULL) {
		(void)printf("closing %s\n",globname);
		if(btdbm_close(globf)) {
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
do_store()
{
	btdatum	key;
	btdatum	data;

	key.dptr = (bt_chrp)globargv[1];
	key.dsize = strlen(globargv[1]);

	data.dptr = (bt_chrp)globargv[2];
	data.dsize = strlen(globargv[2]) + 1;

	if(btdbm_store(globf,key,data,BTDBM_REPLACE))
		printf("could not store\n");
}


void
do_fetch()
{
	btdatum	key;
	btdatum	data;

	key.dptr = (bt_chrp)globargv[1];
	key.dsize = strlen(globargv[1]);

	data = btdbm_fetch(globf,key);
	if(data.dptr == 0)
		printf("could not fetch\n");
	else
		printf("%s\n",data.dptr);
}


void
do_delete()
{
	btdatum	key;

	key.dptr = (bt_chrp)globargv[1];
	key.dsize = strlen(globargv[1]);

	if(btdbm_delete(globf,key) != 0)
		printf("could not delete\n");
	else
		printf("deleted.\n");
}


void
do_first()
{
	btdatum	key;

	key = btdbm_firstkey(globf);
	if(key.dsize == 0)
		printf("no first key\n");
	else
		printf("%s\n",key.dptr);
}


void
do_next()
{
	btdatum	key;

	key = btdbm_nextkey(globf);
	if(key.dsize == 0)
		printf("no next key\n");
	else
		printf("%s\n",key.dptr);
}
