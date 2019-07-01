include ../common.mk
DEPTH = ..

SRCS = $(shell ls *.c)
OBJS = $(SRCS:.c=.o)
APPS = $(SRCS:%.c=_%)

ULIB = ulib.o usys.o printf.o umalloc.o

apps:
	make $(APPS)

clean:
	rm -f _* *.o *.d *.asm *.sym

_%: %.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*.sym

_forktest: forktest.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ forktest.o ulib.o usys.o
	$(OBJDUMP) -S $@ > forktest.asm
