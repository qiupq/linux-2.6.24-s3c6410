# $Id: Makefile,v 1.1.1.1 2008/02/18 02:22:55 ihlee215 Exp $
#
# Makefile for the linux kernel.
#

extra-y	:= head.o


obj-y   := entry.o traps.o irq.o debugport.o dma.o pinmux.o \
	   process.o ptrace.o setup.o signal.o traps.o time.o \
	   arbiter.o io.o

obj-$(CONFIG_ETRAXFS_SIM) += vcs_hook.o

obj-$(CONFIG_SMP) += smp.o
obj-$(CONFIG_ETRAX_KGDB) += kgdb.o kgdb_asm.o
obj-$(CONFIG_ETRAX_FAST_TIMER) += fasttimer.o
obj-$(CONFIG_MODULES)    += crisksyms.o

clean:

