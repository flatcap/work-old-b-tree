#include	<stdio.h>
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
static char *rcsid = "$Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/btdbmclose.c,v 1.1 89/10/24 10:09:10 mjr Rel $";
#endif


btdbm_close(db)
BTDBM	*db;
{
	if(store_close(db->st) != STO_OK || bt_close(db->bt) != BT_OK) {
		btdbm_error(db) = 1;
		return(1);
	}
	btdbm_clearerror(db);
	return(0);
}
