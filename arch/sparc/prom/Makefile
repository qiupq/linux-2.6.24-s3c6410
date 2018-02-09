# $Id: Makefile,v 1.1.1.1 2008/02/18 02:22:55 ihlee215 Exp $
# Makefile for the Sun Boot PROM interface library under
# Linux.
#

lib-y := bootstr.o devmap.o devops.o init.o memory.o misc.o mp.o \
	 palloc.o ranges.o segment.o console.o printf.o tree.o

lib-$(CONFIG_SUN4) += sun4prom.o
