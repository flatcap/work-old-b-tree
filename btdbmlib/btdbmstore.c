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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/btdbmstore.c,v 1.1 89/10/24 10:09:12 mjr Rel $";
#endif


btdbm_store(db,key,data,flag)
BTDBM	*db;
btdatum	key;
btdatum	data;
int	flag;
{
	off_t	orrv;

	btdbm_error(db) = 1;
	if(bt_find(db->bt,key.dptr,key.dsize,&orrv) != BT_NF) {
		if(flag == BTDBM_INSERT) {
			return(1);
		} else {
			/* free old stored record */
			if(store_free(db->st,orrv) == STO_ERR)
				return(1);
		}
	}

	/* allocate new record */
	if((orrv = store_alloc(db->st,data.dsize)) == STO_ERR)
		return(1);

	/* copy data into new record */
	if(store_write(db->st,orrv,0L,data.dptr,data.dsize) == STO_ERR)
		return(1);

	/* update pointer in the b+tree - ok at this point to just replace */
	if(bt_insert(db->bt,key.dptr,key.dsize,orrv,1) == BT_ERR)
		return(1);

	btdbm_clearerror(db);
	return(0);		
}
