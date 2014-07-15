#include	<sys/types.h>
#include	<btdbm.h>


/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/btdbmfetch.c,v 1.1 89/10/24 10:09:10 mjr Rel $";
#endif


btdatum
btdbm_fetch(db,key)
BTDBM	*db;
btdatum	key;
{
	off_t	orrv;
	static	btdatum	ret;
	struct	sto_hdr	hd;

	if(bt_find(db->bt,key.dptr,key.dsize,&orrv) == BT_NF)
		goto bombout;

	/* OK - found a record */
	/* 1) read its header info to see if it will fit in the buffer */
	if(store_gethdr(db->st,orrv,&hd) == STO_ERR)
		goto bombout;

	/* 2) if the internal buffer is not big enough, stretch it */
	if(store_bufsiz(db->st) < hd.used)
		if(store_reallocbuf(db->st,hd.used) == STO_ERR)
			goto bombout;

	/* 3) read the stuff into the buffer and return it */
	if(store_read(db->st,orrv,0L,store_buffer(db->st),store_bufsiz(db->st),&ret.dsize) != STO_OK)
		goto bombout;

	ret.dptr = (bt_chrp)store_buffer(db->st);

	btdbm_clearerror(db);
	return(ret);

bombout:
	ret.dptr = 0;
	ret.dsize = 0;
	btdbm_error(db) = 1;
	return(ret);
}
