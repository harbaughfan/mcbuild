CFLAGS= -g -Warray-bounds=2
DEFS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -g -fstack-protector-strong
INC=-I../libhelper
LIBS_LIBHELPER=-L../libhelper -lhelper
LIBS=$(LIBS_LIBHELPER) -lm -lpng -lz -lcurl -lcrypto -ljson-c -lresolv

SRC_BASE=$(addsuffix .c, mcp_packet mcp_ids mcp_types nbt slot entity helpers mcp_database)
SRC_MCPROXY=$(addsuffix .c, mcproxy mcp_gamestate mcp_game mcp_build mcp_arg mcp_bplan hud) $(SRC_BASE)
#SRC_MCPDUMP=$(addsuffix .c, mcpdump mcp_gamestate anvil) $(SRC_BASE)
#SRC_QHOLDER=$(addsuffix .c, qholder) $(SRC_BASE)
#SRC_DUMPREG=$(addsuffix .c, dumpreg anvil) $(SRC_BASE)
#SRC_MAPPER=$(addsuffix .c, mapper) $(SRC_BASE)
#SRC_ALL=$(SRC_MCPROXY) mcpdump.c varint.c
SRC_ALL=$(SRC_MCPROXY) varint.c

#ALLBIN=mcproxy mcpdump varint qholder dumpreg mapper
ALLBIN=mcproxy varint

HDR_ALL=$(addsuffix .h, mcp_packet mcp_ids mcp_types nbt mcp_game mcp_gamestate mcp_build mcp_arg mcp_bplan slot entity)

DEPFILE=make.depend

ifeq ($(shell uname -s),SunOS)
	INC  += -I$(HOME)/include
	LIBS += -lsocket -lnsl -lmd5 -L$(HOME)/lib
	CC   = gcc
else
ifeq ($(shell uname -o),Cygwin)
	CFLAGS=-std=gnu99
endif
endif


all: $(ALLBIN)

mcproxy: $(SRC_MCPROXY:.c=.o)
	$(CC) -o $@ $^ $(LIBS)

#mcpdump: $(SRC_MCPDUMP:.c=.o)
#	$(CC) -o $@ $^ $(LIBS)

#qholder: $(SRC_QHOLDER:.c=.o)
#	$(CC) -o $@ $^ $(LIBS)

#dumpreg: $(SRC_DUMPREG:.c=.o)
#	$(CC) -o $@ $^ $(LIBS)

#mapper: $(SRC_MAPPER:.c=.o)
#	$(CC) -o $@ $^ $(LIBS)

varint: varint.c
	$(CC) $(CFLAGS) $(INC) $(DEFS) -DTEST=1 -o $@ $^ $(LIBS)



.c.o: $(DEPFILE)
	$(CC) $(CFLAGS) $(DEFS) $(INC) $(CONFIG) -o $@ -c $<

$(DEPFILE): $(SRC_ALL) $(HDR_ALL)
	@rm -rf $(DEPFILE) $(DEPFILE).bak
	@touch $(DEPFILE)
	makedepend -Y -f $(DEPFILE) $(SRC_ALL) 2> /dev/null

install: $(ALLBIN)
	@test -d $(HOME)/bin || mkdir $(HOME)/bin
	@cp -a scripts/* $(HOME)/bin
	@chmod +x $(HOME)/bin/mcb*
	@command -v mcb_update 2>&1 >> /dev/null || ( echo 'export PATH="$$HOME/bin:$$PATH"' >> ~/.bashrc ; echo "Please restart your shell or source ~/.bashrc to set PATH" )

clean:
	rm -f *.o *~ $(ALLBIN) $(TSTBIN) $(DEPFILE)

FORCE:

sinclude $(DEPFILE)
