#include	<sys/types.h>
#include	<sys/file.h>
#include	<sys/stat.h>
#include	<varargs.h>
#include	<stdio.h>
#include	"btconf.h"
#include	"btree.h"
#include	"btintern.h"

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btlib/RCS/btoopen.c,v 1.1 89/10/24 10:09:01 mjr Rel $";
#endif

/*	this way of handling opening and configuration is probably	*/
/*	unnecessarily large and awkward, but I think it gives a pretty	*/
/*	high degree of #ifdefability to make it portable across OS 	*/
/*	types, as well as giving a lot of flexibility for those who	*/
/*	never trust default options. 					*/

extern	char	*malloc();
extern	off_t	lseek();


BT_INDEX	*
bt_optopen(va_alist)
va_dcl
{
	va_list		ap;
	BT_INDEX	*ret;
	struct bt_cache *cp1;
	struct bt_cache *cp2;
	int		dtyp = BT_DFLTDTYPE;
	int		csiz = BT_DFLTCACHE;
	int		cflg = BT_DFLTCFLAG;
	int		psiz = BT_DFLTPSIZ;
	int		operm = S_IREAD|S_IWRITE;
	int		omode = O_RDWR;
	off_t		magic = BT_DFLTMAGIC;
	bt_chrp		labp = NULL;
	int		labl;
	int		r;
	char		*path = NULL;

	if((ret = (BT_INDEX *)malloc(sizeof(BT_INDEX))) == NULL)
		return(NULL);

	/* clear error */
	bt_errno(ret) = BT_NOERROR;

	/* zero this just in case */
	bt_cmpfn(ret) = 0;

	va_start(ap);
	while((r = va_arg(ap,int)) != 0) {
		switch(r) {
			case	BT_PATH:
				path = va_arg(ap,char *);
				break;

			case	BT_PSIZE:
				psiz = va_arg(ap,int);
				if(psiz < sizeof(struct bt_super))
					psiz = sizeof(struct bt_super);
				break;

			case	BT_CACHE:
				csiz = va_arg(ap,int);
				if(csiz < 0)
					csiz = 0;
				break;

			case	BT_CFLAG:
				cflg = va_arg(ap,int);
				break;

			case	BT_OMODE:
				omode = va_arg(ap,int) | O_RDWR;
				break;

			case	BT_OPERM:
				operm = va_arg(ap,int);
				break;

			case	BT_MAGIC:
				magic = va_arg(ap,off_t);
				break;

			case	BT_LABEL:
				labp = va_arg(ap,bt_chrp);
				labl = va_arg(ap,int);
				break;

			case	BT_DTYPE:
				dtyp = va_arg(ap,int);
				/* next arg BETTER be a valid funcp ! */
				if(dtyp == BT_USRDEF)
					bt_cmpfn(ret) = va_arg(ap,FUNCP);
				break;

			default:
				goto bombout;
		}
	}

	if(path == NULL || (bt_fileno(ret) = open(path,omode,operm)) < 0)
		goto bombout;

	r = read(bt_fileno(ret),(char *)&ret->sblk,sizeof(struct bt_super));

	/* failure to read anything - initialize tree */
	if(r == 0) {
		char	*jnk;
		if((jnk = malloc((unsigned)psiz)) != NULL) {
			ret->sblk.magic = magic;
			bt_pagesiz(ret) = psiz;
			bt_dtype(ret) = dtyp;
			ret->sblk.levs = 1;
			ret->sblk.root = (off_t)psiz;
			ret->sblk.free = BT_NULL;
			ret->sblk.high = (off_t)(2 * psiz);

			/* mark super block as dirty and sync */
			ret->dirt = 1;

			/* write and pretend we read a legit superblock */
			if(bt_wsuper(ret) == BT_OK)
				r = sizeof(struct bt_super);

			/* now make jnk into an empty first page */
			KEYCNT(jnk) = 0;
			KEYLEN(jnk) = 0;
			LSIB(jnk) = RSIB(jnk) = BT_NULL;
			HIPT(jnk) = BT_NULL;

			/* now, since the cache is not set up yet, we */
			/* must directly write the page. */
			if(lseek(bt_fileno(ret),(off_t)psiz,SEEK_SET) != (off_t)psiz ||
				write(bt_fileno(ret),jnk,psiz) != psiz)
				r = 0;
			(void)free(jnk);
		}
	}

	/* yet another sanity check */
	if(r != sizeof(struct bt_super) || ret->sblk.magic != magic)
		goto bombout;

	/* label it if we are supposed to */
	if(labp && bt_wlabel(ret,labp,labl) == BT_ERR)
		goto bombout;

	/* initialize locator stack */
	ret->shih = ret->sblk.levs + 2;
	ret->stack = (struct bt_stack *)malloc((unsigned)(ret->shih * sizeof(struct bt_stack)));
	if(ret->stack == NULL)
		goto bombout;

	/* initialize cache */
	cp2 = ret->lru = NULL;
	csiz += BT_MINCACHE;

	/* just in case some bonehead asks for tons of unused cache */
	if(cflg == BT_NOCACHE)
		csiz = BT_MINCACHE;

	while(csiz--) {
		cp1 = (struct bt_cache *)malloc(sizeof(struct bt_cache)); 
		if(cp1 == NULL)
			goto bombout;

		if(ret->lru == NULL)
			ret->lru = cp1;
		cp1->prev = cp2;
		cp1->num = BT_NULL;
		cp1->next = NULL;
		cp1->flags = BT_CHE_CLEAN;
		if((cp1->p = (bt_chrp)malloc((unsigned)bt_pagesiz(ret))) == NULL)
			goto bombout;
		if(cp2 != NULL)
			cp2->next = cp1;

		cp2 = cp1;
	}
	ret->mru = cp1;

	/* set cache type flag */
	ret->cflg = cflg;

	/* no valid current key */
	ret->cpag = BT_NULL;

	/* all is well */
	return(ret);

bombout:
	(void)bt_close(ret);
	return(NULL);
}
