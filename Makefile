CC = g++
LDFLAGS = -levent -lpthread
CCFLAGS =

defs = def0.h	\
		def1.h	\
		game.h	\
		player.h	\
		tools.h	\
		main_serv.h

serv_objs = main.o	\
		game.o	\
		player.o \
		tools.o	\
		main_serv.o

cli_objs = cli_main.o \
		   tools.o

objs = $(serv_objs) $(cli_objs)

all : serv_up cli_up
serv_up : $(serv_objs)
	$(CC) $(serv_objs) -o serv_up $(LDFLAGS)
cli_up : $(cli_objs)
	$(CC) $(cli_objs) -o cli_up

$(objs) : %.o: %.cc $(defs)
	$(CC) -c $(CCFLAGS) $< -o $@


.PHONY : clean
clean:
	-rm serv_up cli_up *.o
