#
# Makefile for Lab 4 - FUSE file system
# CS 3650 Spring 2024
#

# Makefiles use a horrible language. Variables are pretty much OK:
#
CFLAGS = -Wall -pedantic -g
LDFLAGS = -lfuse3
CC = gcc
EXES = lab4-fuse

# rules are "target: <dependencies>" followed by zero or more
# lines of actions to create the target
#
all: $(EXES) img

# '$^' expands to all the dependencies (i.e. homework.o hw3fuse.o misc.o)
# and $@ expands to the target ('lab4-fuse')
#
lab4-fuse: homework.o lab4fuse.o misc.o
	$(CC) -g $^ -o $@ $(LDFLAGS)

# force test.img to be rebuilt each time
.PHONY: img mount umount

img:
	python3 gen-disk1.py test.img > /dev/null

mount:
	mkdir -p fs
	./lab4-fuse -image test.img fs

umount:
	fusermount -u fs

# action lines have to begin with a **tab**, not a space - for the
# stupid reason why, see:
#  https://beebo.org/haycorn/2015-04-20_tabs-and-makefiles.html

# standard practice is to add a 'clean' target that gets rid of
# build output.
#
clean:
	rm -f *.o $(EXES) test.img
