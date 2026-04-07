// Test for aesd-circular-buffer.c and aesd-circular-buffer.h implementations

#include "aesd-circular-buffer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
	char *str1 = "buf0";
	char *str2 = "buffer1";
	char *str3 = "buffer_2";
	char *str4 = "buffer_number_3";

	/*******************************************/
	/************** setup entries **************/
	/*******************************************/
	/*
	struct aesd_buffer_entry entry1;
	entry1.buffptr = str1;
	entry1.size = strlen( entry1.buffptr );

	printf("entry1 addr: 0x%lx\n", (long unsigned int) &entry1 );
	printf("entry1.buffptr: %s\n", entry1.buffptr );
	printf("entry1.size: %d\n", (unsigned int) entry1.size );

	struct aesd_buffer_entry entry2;
	entry2.buffptr = str2;
	entry2.size = strlen( entry2.buffptr );

	printf("entry2 addr: 0x%lx\n", (long unsigned int) &entry2 );
	printf("entry2.buffptr: %s\n", entry2.buffptr );
	printf("entry2.size: %d\n", (unsigned int) entry2.size );

	struct aesd_buffer_entry entry3;
	entry3.buffptr = str3;
	entry3.size = strlen( entry3.buffptr );

	printf("entry3 addr: 0x%lx\n", (long unsigned int) &entry3 );
	printf("entry3.buffptr: %s\n", entry3.buffptr );
	printf("entry3.size: %d\n", (unsigned int) entry3.size );
	*/

	/*******************************************/
	/************** setup entries **************/
	/*******************************************/
	struct aesd_buffer_entry *entry1 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry1->buffptr = str1;
	entry1->size = strlen( entry1->buffptr );

	struct aesd_buffer_entry *entry2 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry2->buffptr = str2;
	entry2->size = strlen( entry2->buffptr );

	struct aesd_buffer_entry *entry3 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry3->buffptr = str3;
	entry3->size = strlen( entry3->buffptr );

	struct aesd_buffer_entry *entry4 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry4->buffptr = str4;
	entry4->size = strlen( entry4->buffptr );

	/*******************************************/
	/************** add entries   **************/
	/*******************************************/
	aesd_circular_buffer_add_entry(&circ_buff, entry1);
	aesd_circular_buffer_add_entry(&circ_buff, entry2);
	aesd_circular_buffer_add_entry(&circ_buff, entry3);
	aesd_circular_buffer_add_entry(&circ_buff, entry4);
	aesd_circular_buffer_add_entry(&circ_buff, entry1);
	aesd_circular_buffer_add_entry(&circ_buff, entry2);
	aesd_circular_buffer_add_entry(&circ_buff, entry3);
	aesd_circular_buffer_add_entry(&circ_buff, entry4);

	/*******************************************/
	/************** test position search func **************/
	/*******************************************/

	struct aesd_buffer_entry *ret_ent = malloc( sizeof( struct aesd_buffer_entry ) );
	size_t ret_offset;
	ret_ent = aesd_circular_buffer_find_entry_offset_for_fpos(&circ_buff, 4, &ret_offset);

	printf("ret_offset: %lu\n", ret_offset);
	printf("ret_ent->buffptr: %s\n", ret_ent->buffptr);

	return 0;
}