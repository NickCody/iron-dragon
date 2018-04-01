all: idpcdu

#include "tmpdep"
CC     = g++
OBJS   = BSD/main.o \
	 BSD/command_line.o \
	 BSD/connection.o \
	 BSD/connection_idpcd.o \
	 BSD/connection_pool.o \
	 BSD/group_pool.o \
	 BSD/idpcd_group.o \
	 BSD/idpcd_group_messages.o \
	 BSD/net_messages_x.o \
	 BSD/startup_daemon.o \
	 BSD/thread_listening.o \
	 BSD/thread_processing.o \
	 BSD/unique_id.o \
	 BSD/common.o \
	 BSD/result.o \
	 BSD/standard_config_file.o \
	 BSD/database_subsystem.o \
	 BSD/file_io_subsystem.o \
	 BSD/port_subsystem.o \
	 BSD/net_subsystem.o \
	 BSD/users.o \
	 BSD/rank_calc.o

LIBS   = 
INCL   = -I../nic_lib
#CFLAGS = -Wall -pedantic -DUNIX $(INCL)
CFLAGS = -fhandle-exceptions -DUNIX $(INCL)

BSD/main.o: main.cpp
	$(CC) $(CFLAGS) -c -o BSD/main.o main.cpp

BSD/command_line.o: command_line.cpp
	$(CC) $(CFLAGS) -c -o BSD/command_line.o command_line.cpp

BSD/connection.o: connection.cpp
	$(CC) $(CFLAGS) -c -o BSD/connection.o connection.cpp

BSD/connection_idpcd.o: connection_idpcd.cpp
	$(CC) $(CFLAGS) -c -o BSD/connection_idpcd.o connection_idpcd.cpp

BSD/connection_pool.o: connection_pool.cpp
	$(CC) $(CFLAGS) -c -o BSD/connection_pool.o connection_pool.cpp

BSD/database_subsystem.o: database_subsystem.c
	$(CC) $(CFLAGS) -c -o BSD/database_subsystem.o database_subsystem.c

BSD/file_io_subsystem.o: file_io_subsystem.c
	$(CC) $(CFLAGS) -c -o BSD/file_io_subsystem.o file_io_subsystem.c

BSD/port_subsystem.o: port_subsystem.c
	$(CC) $(CFLAGS) -c -o BSD/port_subsystem.o port_subsystem.c

BSD/net_subsystem.o: net_subsystem.c
	$(CC) $(CFLAGS) -c -o BSD/net_subsystem.o net_subsystem.c

BSD/group_pool.o: group_pool.cpp
	$(CC) $(CFLAGS) -c -o BSD/group_pool.o group_pool.cpp

BSD/idpcd_group.o: idpcd_group.cpp
	$(CC) $(CFLAGS) -c -o BSD/idpcd_group.o idpcd_group.cpp

BSD/idpcd_group_messages.o: idpcd_group_messages.cpp
	$(CC) $(CFLAGS) -c -o BSD/idpcd_group_messages.o idpcd_group_messages.cpp

BSD/net_messages_x.o: net_messages_x.cpp
	$(CC) $(CFLAGS) -c -o BSD/net_messages_x.o net_messages_x.cpp

BSD/startup_daemon.o: startup_daemon.cpp
	$(CC) $(CFLAGS) -c -o BSD/startup_daemon.o startup_daemon.cpp

BSD/thread_listening.o: thread_listening.cpp
	$(CC) $(CFLAGS) -c -o BSD/thread_listening.o thread_listening.cpp

BSD/thread_processing.o: thread_processing.cpp
	$(CC) $(CFLAGS) -c -o BSD/thread_processing.o thread_processing.cpp

BSD/unique_id.o: unique_id.cpp
	$(CC) $(CFLAGS) -c -o BSD/unique_id.o unique_id.cpp

BSD/rank_calc.o: rank_calc.c
	$(CC) $(CFLAGS) -c -o BSD/rank_calc.o rank_calc.c

BSD/users.o: users.c
	$(CC) $(CFLAGS) -c -o BSD/users.o users.c


#
# nic_lib files
#
BSD/common.o: ../nic_lib/common.cpp
	$(CC) $(CFLAGS) -c -o BSD/common.o ../nic_lib/common.cpp

BSD/result.o: ../nic_lib/result.cpp
	$(CC) $(CFLAGS) -c -o BSD/result.o ../nic_lib/result.cpp

BSD/standard_config_file.o: ../nic_lib/standard_config_file.cpp
	$(CC) $(CFLAGS) -c -o BSD/standard_config_file.o ../nic_lib/standard_config_file.cpp


idpcdu: $(OBJS) dep
	- $(CC) -o BSD/idpcdu $(OBJS) $(LIBS)

depend dep:
	@for i in *.cpp;do echo -n;$(CC) -M $(INCL) $$i;done > tmpdep

strip:
	@for i in *.h;do (cat $$i | tr -d "\r" > tmp); mv -f tmp $$i;done
	@for i in *.cpp;do (cat $$i | tr -d "\r" > tmp); mv -f tmp $$i;done
	@for i in *.c;do (cat $$i | tr -d "\r" > tmp); mv -f tmp $$i;done
	@for i in ../nic_lib/*.h;do (cat $$i | tr -d "\r" > tmp); mv -f tmp $$i;done
	@for i in ../nic_lib/*.cpp;do (cat $$i | tr -d "\r" > tmp); mv -f tmp $$i;done
	@for i in ../nic_lib/*.c;do (cat $$i | tr -d "\r" > tmp); mv -f tmp $$i;done
	@rm -f tmp

clean:
	@echo cleaning intermediate build files...
	@rm BSD/*.o
	@echo done cleaning!
