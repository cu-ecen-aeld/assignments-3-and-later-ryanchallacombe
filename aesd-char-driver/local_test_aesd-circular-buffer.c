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

	char *str1 = malloc(5);
	strcpy(str1, "buf0");
	char *str2 = malloc(8);
	strcpy(str2, "buffer1");
	char *str3 = malloc(9);
	strcpy(str3, "buffer_2");
	char *str4 = malloc(16);
	strcpy(str4, "buffer_number_3");
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
	
	aesd_circular_buffer_add_entry(&circ_buff, entry5);		// 5th
	aesd_circular_buffer_add_entry(&circ_buff, entry6);		// 6th
	aesd_circular_buffer_add_entry(&circ_buff, entry7);		// 7th
	aesd_circular_buffer_add_entry(&circ_buff, entry8);		// 8th
	aesd_circular_buffer_add_entry(&circ_buff, entry9);		// 9th
	aesd_circular_buffer_add_entry(&circ_buff, entry10);	// 10th
	
	const char *rtn_ptr = NULL;
	rtn_ptr = aesd_circular_buffer_add_entry(&circ_buff, entry11);
	rtn_ptr=rtn_ptr;
	
	if ( rtn_ptr != NULL ) {
		printf("freeing overwritten memory\n");
		free( (void *) rtn_ptr);
	}
		

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
	 	int i;
	 	struct aesd_buffer_entry *entryptr;
		AESD_CIRCULAR_BUFFER_FOREACH(entryptr, &circ_buff, index) {
			if ( entryptr->buffptr != NULL )
			{
				char c;
				for( i = 0; i < entryptr->size; i++ ) {
					c = *( entryptr->buffptr + i);
					printf("%c\n", c);
				}

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
			//free(entryptr);		// freeing here doesn't work for some reason
			
		}
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
		free( (void *) entry11->buffptr);
	}

	

	return 0;
}