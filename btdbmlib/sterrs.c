#include	<stdio.h>
#include	<sys/types.h>
#include	"stoconf.h"
#include	"store.h"


/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/sterrs.c,v 1.1 89/10/24 10:09:14 mjr Rel $";
#endif

/*
	error strings and a perror() like routine.
*/

extern	int	errno;

char	*sto_errs[] = {
/* STO_NOERROR */		"no record error",
/* STO_BADHDR */		"bad record header",
/* STO_IOERR */			"record I/O error",
/* STO_NOSTO */			"no such record",
/* STO_TOOSMALL */		"record too small",
0};


void
store_perror(r,s)
STORE	*r;
char		*s;
{
	static	char	*cmesg = "cannot open";
	static	char	*fmt1 = "%s\n";
	static	char	*fmt2 = "%s: %s\n";

	if(r == NULL) {
		if(s == NULL || *s == '\0')
			(void)fprintf(stderr,fmt1,cmesg);
		else
			(void)fprintf(stderr,fmt2,s,cmesg);
		return;
	}
	if(store_errno(r) == STO_NOERROR && errno != 0) {
		perror(s);
	} else {
		if(s == NULL || *s == '\0')
			(void)fprintf(stderr,fmt1,sto_errs[store_errno(r)]);
		else
			(void)fprintf(stderr,fmt2,s,sto_errs[store_errno(r)]);
	}
}
