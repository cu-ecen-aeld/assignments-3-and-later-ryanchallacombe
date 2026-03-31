CFLAGS ?= -Wall -Werror -Iinclude/
UTILS_HEADER ?= include/utils.h
UTILS_SOURCE ?= include/utils.c
TIMESTAMP_HEADER ?= include/time_functions_shared.h
TIMESTAMP_SOURCE ?= include/timestamp_thread.c
LDFLAGS ?= -pthread -lrt

## Invoke cross compilation with: >> make CROSS_COMPILE=aarch64-none-linux-gnu- all

## Line below not required because was have added the path to .bashrc file
# export PATH := $(PATH):/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin

all: aesdsocket

aesdsocket: aesdsocket.o utils.o timestamp_thread.o
ifdef CROSS_COMPILE
	$(CROSS_COMPILE)gcc ${CFLAGS} ${LDFLAGS} -o aesdsocket aesdsocket.o utils.o timestamp_thread.o
else
	${CC} ${CFLAGS} ${LDFLAGS} -o aesdsocket aesdsocket.o utils.o timestamp_thread.o
endif

aesdsocket.o: aesdsocket.c ${UTILS_HEADER}
ifdef CROSS_COMPILE
	$(CROSS_COMPILE)gcc ${CFLAGS} -c aesdsocket.c ${LDFLAGS} 
else
	${CC} ${CFLAGS} -c aesdsocket.c ${LDFLAGS}
endif

utils.o: ${UTILS_SOURCE} ${UTILS_HEADER}
ifdef CROSS_COMPILE
	$(CROSS_COMPILE)gcc ${CFLAGS} -c ${UTILS_SOURCE} ${LDFLAGS}
else
	${CC} ${CFLAGS} -c ${UTILS_SOURCE} ${LDFLAGS}
endif

timestamp_thread.o: ${TIMESTAMP_SOURCE} ${TIMESTAMP_HEADER}
ifdef CROSS_COMPILE
	$(CROSS_COMPILE)gcc ${CFLAGS} -c ${TIMESTAMP_SOURCE} ${LDFLAGS}
else
	${CC} ${CFLAGS} -c ${TIMESTAMP_SOURCE} ${LDFLAGS}
endif


clean:
	rm -f aesdsocket *.o