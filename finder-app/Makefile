export PATH := $(PATH):/home/ryan/projects/aarch64_toolchain_install_dir/install/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin

# Invoke cross compilation to aarch64 with the following:
# make writer CROSS_COMPILE=aarch64-none-linux-gnu-

writer: writer.c
ifdef CROSS_COMPILE
	$(CROSS_COMPILE)gcc writer.c -o writer	
else
	cc writer.c -o writer	
endif

clean:
	rm -f writer *.o
