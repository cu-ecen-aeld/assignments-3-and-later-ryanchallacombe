// Test for aesd-circular-buffer.c and aesd-circular-buffer.h implementations

#include "aesd-circular-buffer.h"
#include <stdio.h>
#include <string.h>

int main(void) 
{
	/*******************************************/
	/***** setup the circular buffer ***********/
	/*******************************************/
	struct aesd_circular_buffer circ_buff;
	aesd_circular_buffer_init( &circ_buff );

	/*******************************************/
	/************** setup strings **************/
	/*******************************************/
	char *str1 = "buf1";

	/*******************************************/
	/************** setup entries **************/
	/*******************************************/
	struct aesd_buffer_entry entry1;
	entry1.buffptr = str1;
	entry1.size = strlen( entry1.buffptr );

	printf("entry1.buffptr: %s\n", entry1.buffptr );
	printf("entry1.size: %d\n", (unsigned int) entry1.size );

	/*******************************************/
	/************** add entries   **************/
	/*******************************************/
	aesd_circular_buffer_add_entry(&circ_buff, &entry1);



	return 0;
}