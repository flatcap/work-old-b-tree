# 
#
#/*
#         (C) Copyright, 1988, 1989 Marcus J. Ranum
#                    All rights reserved
#
#
#          This software, its documentation,  and  supporting
#          files  are  copyrighted  material  and may only be
#          distributed in accordance with the terms listed in
#          the COPYRIGHT document.
#*/
#
# $Header: /atreus/mjr/hacks/btree/utils/RCS/Makefile,v 1.1 89/10/24 10:09:30 mjr Rel $
#

BTHDRDIR=../btlib
DBHDRDIR=../btdbmlib

LIBDIR=..
BTLIB= $(LIBDIR)/libbtree.a
DBLIB= $(LIBDIR)/libbtdbm.a

# if compiled with -DBTOPEN test programs will use simpler version
# of open that is smaller and has less frills
# if compiled with -DDEBUG, testrack will give an idea of what is
# going on inside

CC= cc

#CFLAGS = -I$(BTHDRDIR) -I$(DBHDRDIR) -g
#CFLAGS = -I$(BTHDRDIR) -I$(DBHDRDIR) -p -DDEBUG
CFLAGS = -I$(BTHDRDIR) -I$(DBHDRDIR) -O

LDFLAGS = -p
#LDFLAGS = -s


CFILES=		testrack.c \
		words.c \
		btoptim.c \
		rectest.c \
		dbtest.c \
		btest.c

all:	testrack btest rectest dbtest words btoptim

testrack:	testrack.o $(BTLIB)
	$(CC) $(LDFLAGS) -o testrack testrack.o $(BTLIB)

btest:	btest.o $(BTLIB)
	$(CC) $(LDFLAGS) -o btest btest.o $(BTLIB)

rectest:	rectest.o $(DBLIB)
	$(CC) $(LDFLAGS) -o rectest rectest.o $(DBLIB)

dbtest:	dbtest.o $(DBLIB) $(BTLIB)
	$(CC) $(LDFLAGS) -o dbtest dbtest.o $(DBLIB) $(BTLIB)

words:	words.o
	$(CC) $(LDFLAGS) -o words words.o

btoptim:	btoptim.o $(BTLIB)
	$(CC) $(LDFLAGS) -o btoptim btoptim.o $(BTLIB)

flog:	testrack words flog.sh
	sh flog.sh

clean:
	rm -f testrack words btest *.o core input input.srt test.dat \
	test2.dat btoptim rectest dbtest

lint:
	lint -I$(BTHDRDIR) -I$(DBHDRDIR) -u $(CFILES) | sed '/pointer alignment/d'

ci:	clean
	ci -u $(CFILES) flog.sh Makefile README < /dev/null
