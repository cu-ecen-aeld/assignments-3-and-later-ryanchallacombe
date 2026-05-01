// Test for aesd-circular-buffer.c and aesd-circular-buffer.h implementations

#include "aesd-circular-buffer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
*
VALGRIND checks:
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=/home/ryan/projects/A7P1_assignments-3-and-later-ryanchallacombe/aesd-char-driver/valgrind-out.txt ./local_test_aesd-circular-buffer
*
*/

#define DO_FREE_MEM		1

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
	/*
	const char *str1 = "buf0";
	const char *str2 = "buffer1";
	const char *str3 = "buffer_2";
	const char *str4 = "buffer_number_3";
	*/

	char *str1 = malloc(5);
	strcpy(str1, "buf0");
	char *str2 = malloc(8);
	strcpy(str2, "buffer1");
	char *str3 = malloc(9);
	strcpy(str3, "buffer_2");
	char *str4 = malloc(16);
	strcpy(str4, "buffer_number_3");

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

	/*******************************************/
	/************** test position search func **************/
	/*******************************************/


	size_t char_pos = 5;
	//struct aesd_buffer_entry *ret_ent = malloc( sizeof( struct aesd_buffer_entry ) );		// can't figure out how to free this if dynamically alloc'd
	struct aesd_buffer_entry *ret_ent;
	size_t ret_offset;
	ret_ent = aesd_circular_buffer_find_entry_offset_for_fpos(&circ_buff, char_pos, &ret_offset);

	printf("testing for char_pos = %lu\n", char_pos);
	if ( ret_ent != NULL ) {
		printf("ret_offset: %lu\n", ret_offset);
		printf("ret_ent->buffptr: %s\n", ret_ent->buffptr);
	} else {
		printf("nothing found at char_pos = %lu\n", char_pos);
	}

	/*******************************************/
	/************** add entries   **************/
	/*******************************************/
	/*
	aesd_circular_buffer_add_entry(&circ_buff, entry1);
	aesd_circular_buffer_add_entry(&circ_buff, entry2);
	aesd_circular_buffer_add_entry(&circ_buff, entry3);
	aesd_circular_buffer_add_entry(&circ_buff, entry4);
	aesd_circular_buffer_add_entry(&circ_buff, entry1);
	aesd_circular_buffer_add_entry(&circ_buff, entry2);
	aesd_circular_buffer_add_entry(&circ_buff, entry3);
	//aesd_circular_buffer_add_entry(&circ_buff, entry4);
	*/

	/*
	char_pos = 100;
	ret_ent = NULL;
	ret_offset = 99;
	ret_ent = aesd_circular_buffer_find_entry_offset_for_fpos(&circ_buff, char_pos, &ret_offset);

	printf("testing for char_pos = %lu\n", char_pos);
	if ( ret_ent != NULL ) {
		printf("ret_offset: %lu\n", ret_offset);
		printf("ret_ent->buffptr: %s\n", ret_ent->buffptr);
	} else {
		printf("nothing found at char_pos = %lu\n", char_pos);
	}
	*/
	
	if ( DO_FREE_MEM ) {

		//free(ret_ent); // can't figure out how to free this if dynamically alloc'd

	 	uint8_t index;
	 	struct aesd_buffer_entry *entryptr;
		AESD_CIRCULAR_BUFFER_FOREACH(entryptr, &circ_buff, index) {
			if ( entryptr->buffptr != NULL )
			{
				printf("entryptr->size: %zu\n", entryptr->size);
				printf("entryptr->buffptr: %s\n", entryptr->buffptr);
				
			}
			free((void *) entryptr->buffptr);
			//free(entryptr);		// freeing here doesn't work for some reason
			
		}
		free(entry1);
		free(entry2);
		free(entry3);
		free(entry4);
	}

	

	return 0;
}