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
# $Header: /atreus/mjr/hacks/btree/btdbmlib/RCS/Makefile,v 1.1 89/10/24 10:09:23 mjr Rel $
#

BTHDRDIR=../btlib
DBHDRDIR=.

# compiler to use
CC= cc

#CFLAGS = -I$(BTHDRDIR) -I$(DBHDRDIR) -g
#CFLAGS = -I$(BTHDRDIR) -I$(DBHDRDIR) -p
CFLAGS = -I$(BTHDRDIR) -I$(DBHDRDIR) -O

LDFLAGS= -g

# where to build the library. must be set in testrack/Makefile
# if you change it from here.
LIBDIR=..
LIB=	$(LIBDIR)/libbtdbm.a

CFILES=	btdbmclose.c \
	btdbmdelete.c \
	btdbmfetch.c \
	btdbmfirstkey.c \
	btdbmnextkey.c \
	btdbmopen.c \
	btdbmstore.c \
	stalloc.c \
	stclose.c \
	stcopy.c \
	stdecref.c \
	sterrs.c \
	stfree.c \
	stgethed.c \
	stincref.c \
	stlinka.c \
	stlinkb.c \
	stopen.c \
	stputhed.c \
	stread.c \
	strealloc.c \
	streallocbuf.c \
	stunlink.c \
	stwrite.c \
	stwsuper.c

HFILES=	btdbm.h \
	stoconf.h \
	store.h \
	stointern.h

OFILES=	btdbmclose.o \
	btdbmdelete.o \
	btdbmfetch.o \
	btdbmfirstkey.o \
	btdbmnextkey.o \
	btdbmopen.o \
	btdbmstore.o \
	stalloc.o \
	stclose.o \
	stcopy.o \
	stdecref.o \
	sterrs.o \
	stfree.o \
	stgethed.o \
	stincref.o \
	stlinka.o \
	stlinkb.o \
	stopen.o \
	stputhed.o \
	stread.o \
	strealloc.o \
	streallocbuf.o \
	stunlink.o \
	stwrite.o \
	stwsuper.o

FILES=	$(CFILES) \
	$(HFILES) \
	README \
	Makefile

$(LIB):	$(OFILES) 
	ar crv $(LIB) $(OFILES)
	ranlib $(LIB)

$(OFILES): $(HFILES)

clean:
	rm -f *.o $(LIB) core tags rectest

ci:
	ci -u $(FILES) < /dev/null

tags:
	ctags $(CFILES)

lint:	$(CFILES)
	lint -I$(BTHDRDIR) -I$(DBHDRDIR) -u $(CFILES) | sed '/pointer alignment/d'

