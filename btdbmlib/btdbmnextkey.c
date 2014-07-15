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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/btdbmnextkey.c,v 1.1 89/10/24 10:09:11 mjr Rel $";
#endif


btdatum
btdbm_nextkey(db)
BTDBM	*db;
{
	static	btdatum	ret;
	off_t	rrv;
	int	r;

	r = bt_traverse(db->bt,BT_EOF,store_buffer(db->st),store_bufsiz(db->st),&ret.dsize,&rrv);
	if(r == BT_ERR)
		goto bombout;

	if(r == BT_EOF) {
		ret.dptr = 0;
		ret.dsize = 0;
		btdbm_clearerror(db);
		return(ret);
	}
	
	ret.dptr = (bt_chrp)store_buffer(db->st);

	btdbm_clearerror(db);
	return(ret);

bombout:
	ret.dptr = 0;
	ret.dsize = 0;
	btdbm_error(db) = 1;
	return(ret);
}
