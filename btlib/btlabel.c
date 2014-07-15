#include	<sys/types.h>
#include	<sys/file.h>
#include	"btconf.h"
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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btlib/RCS/btlabel.c,v 1.1 89/10/24 10:08:59 mjr Rel $";
#endif

extern	off_t	lseek();

bt_wlabel(b,buf,siz)
BT_INDEX	*b;
bt_chrp		buf;
int		siz;
{
	if(siz > BT_LABSIZ(b)) {
		bt_errno(b) = BT_LTOOBIG;
		return(BT_ERR);
	}

	if((lseek(bt_fileno(b),(off_t)sizeof(struct bt_super),SEEK_SET) !=
		(off_t)sizeof(struct bt_super)) ||
		(write(bt_fileno(b),(char *)buf,siz) != siz))
			return(BT_ERR);
	bt_clearerr(b);
	return(BT_OK);
}

bt_rlabel(b,buf,siz)
BT_INDEX	*b;
bt_chrp		buf;
int		siz;
{
	if(siz < BT_LABSIZ(b)) {
		bt_errno(b) = BT_BTOOSMALL;
		return(BT_ERR);
	}

	if((lseek(bt_fileno(b),(off_t)sizeof(struct bt_super),SEEK_SET) !=
		(off_t)sizeof(struct bt_super)) ||
		(read(bt_fileno(b),(char *)buf,BT_LABSIZ(b)) != BT_LABSIZ(b)))
			return(BT_ERR);
	bt_clearerr(b);
	return(BT_OK);
}
