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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/btdbmdelete.c,v 1.1 89/10/24 10:09:10 mjr Rel $";
#endif


btdbm_delete(db,key)
BTDBM	*db;
btdatum	key;
{
	off_t	orrv;

	btdbm_error(db) = 1;
	if(bt_find(db->bt,key.dptr,key.dsize,&orrv) == BT_NF)
		return(1);

	/* free the record */
	if(store_free(db->st,orrv) == STO_ERR)
		return(1);

	/* clob the key */
	if(bt_delete(db->bt,key.dptr,key.dsize) != BT_OK)
		return(1);

	btdbm_clearerror(db);
	return(0);
}
