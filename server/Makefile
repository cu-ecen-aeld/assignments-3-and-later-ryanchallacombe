CFLAGS = -Wall -Werror -Iinclude/
UTILS_HEADER = include/utils.h
UTILS_SOURCE = include/utils.c


export PATH := $(PATH):/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin

# all: aesdsocket

aesdsocket: aesdsocket.o utils.o
	${CC} ${CFLAGS} -o aesdsocket aesdsocket.o utils.o 

aesdsocket.o: aesdsocket.c ${UTILS_HEADER}
	${CC} ${CFLAGS} -c aesdsocket.c

utils.o: ${UTILS_SOURCE} ${UTILS_HEADER}
	${CC} ${CFLAGS} -c ${UTILS_SOURCE}

clean:
	rm -f aesdsocket *.o
	rm -f utils *.o