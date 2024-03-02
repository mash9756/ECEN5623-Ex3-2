#
#	
#	referenced stack overflow post here
#		https://stackoverflow.com/questions/14639794/getting-make-to-create-object-files-in-a-specific-directory
#
#
INCLUDE_DIRS	= 
LIB_DIRS 		= 
CC 				= gcc
CDEFS 			=
CFLAGS 			= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS 			= -pthread

SRC_NAME = mutex

HFILES 	= 
CFILES 	= src/${SRC_NAME}.c
SRCS 	= ${HFILES} ${CFILES}
OBJS 	= bin/${SRC_NAME}.o
TRGT	= bin/${SRC_NAME}

all: build ${SRC_NAME}

build:
	mkdir -p bin

${SRC_NAME}: ${SRC_NAME}.o
	$(CC) $(LIBS) $(CFLAGS) $(OBJS) -o $(TRGT) -lm

${SRC_NAME}.o: build
	$(CC) $(LIBS) $(CFLAGS) -c $(SRCS) -o $(OBJS) -lm

clean:
	rm -r bin