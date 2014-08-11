##########################################################
#Copyright(C) 2012 WAF PROJECT TEAM
#Author(A) shajianfeng
##########################################################

include ../make.include
CFLAGS  = ${BUILD_CFLAGS} -ggdb3 -O0 -rdynamic -D_GNU_SOURCE -pthread
#CFLAGS += -I../include 
define  V
endef
CFLAGS += $(shell xml2-config --cflags) -fPIC
PREFIX_BIN = /usr/local/lib/
PREFIX_INCLUDE = /usr/local/include/
LDFLAGS  = ${BUILD_LDFLAGS} -lpthread -lxml2 -lpcre -lzmq  -L${PREFIX_BIN}


ce_util_SOURCES = ce_string.c ce_palloc.c ce_alloc.c ce_array.c
ce_util_SOURCES += ce_atom.c   
ce_util_SOURCES += ce_connection.c ce_epoll.c ce_errno.c 
ce_util_SOURCES += ce_file.c ce_inet_ntop.c  ce_inet_pton.c
ce_util_SOURCES += ce_listen.c  ce_pollset.c
ce_util_SOURCES += ce_qsort.c ce_rbtree.c 
ce_util_SOURCES += ce_regex.c  ce_sem.c  ce_sendrecv.c 
ce_util_SOURCES += ce_shm.c ce_signals.c ce_sockaddr.c
ce_util_SOURCES += ce_socket.c ce_sockopt.c ce_tables.c
ce_util_SOURCES += ce_thread.c ce_thread_cond.c ce_thread_mutex.c
ce_util_SOURCES += ce_thread_rwlock.c 

ce_util_SOURCES += ce_time.c ce_times.c ce_timer.c 

ce_util_SOURCES += ce_buf.c ce_map.c ce_reader.c ce_kv_filter.c
ce_util_SOURCES += ce_bitmap.c ce_bitops.c ce_maptree.c ce_writer.c
ce_util_SOURCES += ce_fs_monitor.c
ce_util_SOURCES += ce_data_reader.c  ce_data_reader_encoding.c  ce_data_record.c  ce_data_writer.c  ce_data_writer_encoding.c
ce_util_SOURCES += ce_log.c
ce_util_SOURCES += ce_ipc_context.c  #ce_ipc_shm.c
ce_util_SOURCES += ce_varbuf.c ce_line_conf_file.c

ce_util_OBJECTS = $(patsubst %.c,%.o,$(ce_util_SOURCES))
ce_util_DEPENDS = $(patsubst %.c,%.d,$(ce_util_SOURCES))
ce_util_ASMFILE = $(patsubst %.c,%.s,$(ce_util_SOURCES))
ce_package = libce_util.so

.PHONY: all clean install

all: $(ce_package)

$(ce_package): $(ce_util_OBJECTS)
	gcc -fPIC -shared -ggdb3 -O0   -o $(ce_package) $^ $(LDFLAGS) 

clean:
	@rm -fr $(ce_util_OBJECTS) $(ce_util_DEPENDS) $(ce_util_ASMFILE) $(ce_package)
	@rm -fr *.d *.o *.s 

install:$(TAEGET)
	cp $(ce_package) $(PREFIX_BIN)
	cp ../include/*.h $(PREFIX_INCLUDE)
uninstall:
	rm -f $(PREFIX_BIN)/$(ce_package)
