CC =gcc
CFLAGS =-O0 -g3 -Wall
LIBS =-lcstl -lm -lssl -lcrypto

OBJS =cJSON.o spider.o sslayer.o user.o

all: ZHSpider

ZHSpider: $(OBJS)
	echo 'Building target: $@'
	echo 'Invoking: GCC C Linker'
	gcc $(CFLAGS) -o $@ $(OBJS) $(LIBS)
	echo 'Finished building target: $@'
	echo ' '
	
clean:
	rm $(OBJS) ZHSpider

./%.o: ./%.c
	echo 'Building file: $<'
	echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -o "$@" -c "$<" $(LIBS)
	echo 'Finished building: $<'
	echo ' '
	
