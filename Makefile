IMGS = fs.img xv6.img

# If the makefile can't find QEMU, specify its path here
# QEMU = qemu-system-i386

# Try to infer the correct QEMU
ifndef QEMU
QEMU = $(shell if which qemu > /dev/null; \
	   then echo qemu; exit; \
	   elif which qemu-system-i386 > /dev/null; \
	   then echo qemu-system-i386; exit; \
	   elif which qemu-system-x86_64 > /dev/null; \
	   then echo qemu-system-x86_64; exit; \
	   else \
	   qemu=/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu; \
	   if test -x $$qemu; then echo $$qemu; exit; fi; fi; \
	   echo "***" 1>&2; \
	   echo "*** Error: Couldn't find a working QEMU executable." 1>&2; \
	   echo "*** Is the directory containing the qemu binary in your PATH" 1>&2; \
	   echo "*** or have you tried setting the QEMU variable in Makefile?" 1>&2; \
	   echo "***" 1>&2; exit 1)
endif

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
		  then echo "-gdb tcp::$(GDBPORT)"; \
		  else echo "-s -p $(GDBPORT)"; fi)
ifndef CPUS
CPUS := 2
endif

QEMUOPTS = -smp $(CPUS) -m 512 \
		   -drive file=fs.img,index=1,media=disk,format=raw \
		   -drive file=xv6.img,index=0,media=disk,format=raw \
		   $(QEMUEXTRA)

# command
default:
	make $(IMGS)

bochs: $(IMGS)
	if [ ! -e .bochsrc ]; then ln -s dot-bochsrc .bochsrc; fi
	bochs -q

qemu: $(IMGS)
	$(QEMU) -serial mon:stdio $(QEMUOPTS)

clean:
	rm -f *.img .gdbinit \
			*.tex *.dvi *.idx *.aux *.log *.ind *.ilg
	make -C kernel clean
	make -C app clean

# files
fs.img: mkfs README
	make -C app
	cp app/_* .
	./mkfs fs.img README _*
	rm -f _*

xv6.img: kernel/xv6.img
	cp $< $@

kernel/xv6.img:
	make -C kernel

mkfs: mkfs.c fs.h
	gcc -Werror -Wall -o $@ $<
