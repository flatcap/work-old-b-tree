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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/btdbmfirstkey.c,v 1.1 89/10/24 10:09:11 mjr Rel $";
#endif


btdatum
btdbm_firstkey(db)
BTDBM	*db;
{
	static	btdatum	ret;

	btdbm_error(db) = 1;
	if(bt_goto(db->bt,BT_BOF) == BT_ERR) {
		ret.dptr = 0;
		ret.dsize = 0;
		return(ret);
	}
	
	ret = btdbm_nextkey(db);
	return(ret);
}
