CFLAGS ?= -Wall -Werror
HEADER ?= aesd-circular-buffer.h
SOURCE ?= aesd-circular-buffer.c
LDFLAGS ?=

## Invoke cross compilation with: >> make CROSS_COMPILE=aarch64-none-linux-gnu- all

## Line below not required because was have added the path to .bashrc file
# export PATH := $(PATH):/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin

all: local_test_aesd-circular-buffer

local_test_aesd-circular-buffer: local_test_aesd-circular-buffer.o aesd-circular-buffer.o
	${CC} ${CFLAGS} ${LDFLAGS} -o local_test_aesd-circular-buffer local_test_aesd-circular-buffer.o aesd-circular-buffer.o

local_test_aesd-circular-buffer.o: local_test_aesd-circular-buffer.c ${HEADER}
	${CC} ${CFLAGS} -c local_test_aesd-circular-buffer.c ${LDFLAGS}

aesd-circular-buffer.o: ${SOURCE} ${HEADER}
	${CC} ${CFLAGS} -c ${SOURCE} ${LDFLAGS}

clean:
	rm -f local_test_aesd-circular-buffer *.o