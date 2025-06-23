PROGRAMS = client test_snfs_ping \
test_snfs_create_write_read test_snfs_mkdir_readdir \
test_snfs_copy test_snfs_concurrent

INCLUDES = -I . -I ../include -I ../snfs_lib
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CFLAGS)
CC = gcc
CFLAGS = -g -O0 -Wall -m32 -std=c99
DEFS = -DHAVE_CONFIG_H -DSIMULATE_IO_DELAY 
LIBSTHREAD = ../sthread_lib/libsthread.a 
LIBSOCKS = -lpthread -lnsl
SNFS_LIB_OBJ = ../snfs_lib/snfs_api.o
COMMON = tfuncs.o

OBJECTS = test_snfs_create_write_read.o 



all: $(PROGRAMS)

.SUFFIXES: .c .o

client: $(OBJECTS)
	$(CC) $(CFLAGS) ../snfs_lib/snfs_api.o -o client $(OBJECTS) $(LIBSTHREAD) $(LIBSOCKS)

test_snfs_ping: $(OBJECTS)
	$(CC) $(CFLAGS) ../sthread_lib/sthread_start.o -o test_snfs_ping $(OBJECTS) $(LIBSTHREAD) $(LIBSOCKS)


test_snfs_create_write_read: $(OBJECTS)
	$(CC) $(CFLAGS) ../sthread_lib/sthread_start.o -o test_snfs_create_write_read $(OBJECTS) $(LIBSTHREAD) $(LIBSOCKS)


test_snfs_mkdir_readdir: $(OBJECTS)
	$(CC) $(CFLAGS) ../sthread_lib/sthread_start.o -o test_snfs_mkdir_readdir $(OBJECTS) $(LIBSTHREAD) $(LIBSOCKS)

test_snfs_concurrent: $(OBJECTS)
	$(CC) $(CFLAGS) ../sthread_lib/sthread_start.o -o test_snfs_concurrent $(OBJECTS) $(LIBSTHREAD) $(LIBSOCKS)

test_snfs_copy: $(OBJECTS)
	$(CC) $(CFLAGS) ../sthread_lib/sthread_start.o -o test_snfs_copy $(OBJECTS) $(LIBSTHREAD) $(LIBSOCKS)

libs:
	$(MAKE) libsnfs.a -C ../snfs_lib
	$(MAKE) libsthread.a -C ../sthread_lib

.c.o:
	$(COMPILE) -c -o $@ $<

clean: clean-PROGRAMS
	rm -f *.o

clean-PROGRAMS:
	@list='$(PROGRAMS)'; for p in $$list; do \
	  f=`echo $$p|sed 's/$$//'`; \
	  echo " rm -f $$p $$f"; \
	  rm -f $$p $$f ; \
	done

.NOEXPORT:
