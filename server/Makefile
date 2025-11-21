export PATH := $(PATH):/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin

all: aesdsocket

aesdsocket: aesdsocket.c
ifdef CROSS_COMPILE
	$(CROSS_COMPILE)gcc aesdsocket.c -o aesdsocket
else
	${CC} aesdsocket.c -o aesdsocket	
endif

clean:
	rm -f aesdsocket *.o