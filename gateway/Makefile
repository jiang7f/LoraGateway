
CFLAGS := -O2 -Wall -Wextra -std=gnu99 -Iinc -I.   -I../libloragw/inc -lpaho-mqtt3c -lpaho-mqtt3a
OBJDIR = obj
INCLUDES = $(wildcard inc/*.h) 
ALLCLASS := obj/PacketReceiveThrd.o  obj/UnpackingThrd.o obj/LoraGateway.o  \
						obj/Packet.o obj/UnpackingOperator.o obj/NodeManager.o \
						obj/NodeManageThrd.o

all: libuser.a  $(ALLCLASS)

clean:
	rm -f libuser.a
	rm -f $(OBJDIR)/*.o
	
$(OBJDIR)/%.o: src/%.c $(INCLUDES)  ../libloragw/libloragw.a
	$(CC) -c $(CFLAGS)  $< -o $@ 

### static library

libuser.a: $(ALLCLASS)
	ar rcs $@ $^


### EOF
