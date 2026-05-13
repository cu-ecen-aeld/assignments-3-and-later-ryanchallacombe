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
#define MEM_CPY			0

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

	char *str1 = malloc(6);
	strcpy(str1, "buf0\n");
	char *str2 = malloc(9);
	strcpy(str2, "buffer1\n");
	char *str3 = malloc(10);
	strcpy(str3, "buffer_2\n");
	char *str4 = malloc(17);
	strcpy(str4, "buffer_number_3\n");
	char *str5 = malloc(8);
	strcpy(str5, "write5\n");
	char *str6 = malloc(8);
	strcpy(str6, "write6\n");
	char *str7 = malloc(8);
	strcpy(str7, "write7\n");
	char *str8 = malloc(8);
	strcpy(str8, "write8\n");
	char *str9 = malloc(8);
	strcpy(str9, "write9\n");
	char *str10 = malloc(9);
	strcpy(str10, "write10\n");
	char *str11 = malloc(9);
	strcpy(str11, "write11\n");

	/*******************************************/
	/************** setup entries **************/
	/*******************************************/
	struct aesd_buffer_entry *entry1 = malloc( sizeof( struct aesd_buffer_entry ) );
	printf("entry1 %p\n", (void *) entry1);
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

	struct aesd_buffer_entry *entry5 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry5->buffptr = str5;
	entry5->size = strlen( entry5->buffptr );

	struct aesd_buffer_entry *entry6 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry6->buffptr = str6;
	entry6->size = strlen( entry6->buffptr );

	struct aesd_buffer_entry *entry7 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry7->buffptr = str7;
	entry7->size = strlen( entry7->buffptr );

	struct aesd_buffer_entry *entry8 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry8->buffptr = str8;
	entry8->size = strlen( entry8->buffptr );

	struct aesd_buffer_entry *entry9 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry9->buffptr = str9;
	entry9->size = strlen( entry9->buffptr );

	struct aesd_buffer_entry *entry10 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry10->buffptr = str10;
	entry10->size = strlen( entry10->buffptr );

	struct aesd_buffer_entry *entry11 = malloc( sizeof( struct aesd_buffer_entry ) );
	entry11->buffptr = str11;
	entry11->size = strlen( entry11->buffptr );

	/*******************************************/
	/************** add entries   **************/
	/*******************************************/
	// NOTE:
	// In the circ buffer struct, the entries in the entry array are statically allocated
	// what the ...add_entry() function does is to copy the data from the input structs into the 
	// statically allocated entry
	// It is not setting a point to the entry that it is adding
	// therefore, we can free each entry here after it is used.
	aesd_circular_buffer_add_entry(&circ_buff, entry1);
	free(entry1);
	aesd_circular_buffer_add_entry(&circ_buff, entry2);
	free(entry2);
	aesd_circular_buffer_add_entry(&circ_buff, entry3);
	free(entry3);
	aesd_circular_buffer_add_entry(&circ_buff, entry4);
	free(entry4);

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
	
	aesd_circular_buffer_add_entry(&circ_buff, entry5);		// 5th
	free(entry5);
	aesd_circular_buffer_add_entry(&circ_buff, entry6);		// 6th
	free(entry6);
	aesd_circular_buffer_add_entry(&circ_buff, entry7);		// 7th
	free(entry7);
	aesd_circular_buffer_add_entry(&circ_buff, entry8);		// 8th
	free(entry8);
	aesd_circular_buffer_add_entry(&circ_buff, entry9);		// 9th
	free(entry9);
	aesd_circular_buffer_add_entry(&circ_buff, entry10);	// 10th
	free(entry10);
	
	// 11th
	const char *rtn_ptr = NULL;
	rtn_ptr = aesd_circular_buffer_add_entry(&circ_buff, entry11);
	free(entry11);
	
	if ( rtn_ptr != NULL ) {
		printf("freeing overwritten memory\n");
		free( (void *) rtn_ptr);
	}

	/*******************************************/
	/************** aesd_read() development **************/
	/*******************************************/
	// New 5/9/2026
	// for aesd_read() in the driver we need to return all buffer contents
	// here i will develop a way to do that

	// print out all contents using the macro
	uint8_t idx = 0;
	struct aesd_buffer_entry *entryptr = NULL;
	AESD_CIRCULAR_BUFFER_FOREACH(entryptr, &circ_buff, idx) {
		printf("aesd_read() style loop: entryptr->buffptr: %s\n", entryptr->buffptr);
		printf("aesd_read() style loop: circ buffer idx: %i\n", idx);
	}

	// start at a given position and print out all the contents
	loff_t *f_pos;
	long int rd_char_offset = 0;
	f_pos = &rd_char_offset;
	f_pos = f_pos;

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
	
	if ( DO_FREE_MEM ) {

		//free(ret_ent); // can't figure out how to free this if dynamically alloc'd

	 	uint8_t index = 0;
	 	entryptr = NULL;
		AESD_CIRCULAR_BUFFER_FOREACH(entryptr, &circ_buff, index) {
			if ( entryptr->buffptr != NULL )
			{
				// loop through each entry letter by letter
				/*
				char c;
				for( i = 0; i < entryptr->size; i++ ) {
					c = *( entryptr->buffptr + i);
					printf("%c\n", c);
				}
				*/

				printf("*****************************\n");
				printf("entryptr->size: %zu\n", entryptr->size);
				printf("entryptr->buffptr: %s\n", entryptr->buffptr);
				printf("*****************************\n");

				if ( MEM_CPY ) {
					// void *memcpy(void *dest, const void *src, size_t n)
					struct aesd_buffer_entry *cp_entryptr = malloc( sizeof(struct aesd_buffer_entry) );
					cp_entryptr->buffptr = malloc( entryptr->size );

					memcpy( (void *) cp_entryptr->buffptr, (void *) entryptr->buffptr, entryptr->size );
					cp_entryptr->size = entryptr->size;

					printf("*****************************\n");
					printf("cp_entryptr->size: %zu\n", cp_entryptr->size);
					printf("cp_entryptr->buffptr: %s\n", cp_entryptr->buffptr);
					printf("*****************************\n");

					free(cp_entryptr);
					free( (void *) cp_entryptr->buffptr);
				}

				
			}
			free((void *) entryptr->buffptr);
			// free(entryptr);		// WRONG! the entries in circ buff entry array are statically allocated
			
		}

		/* if we hadn't freed this after each add_entry call we would need to do so here

		free(entry1);
		free(entry2);
		free(entry3);
		free(entry4);
		free(entry5);
		free(entry6);
		free(entry7);
		free(entry8);
		free(entry9);
		free(entry10);
		free(entry11);
		*/
	}


	return 0;
}