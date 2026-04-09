/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    size_t total_byte_count = 0;
    struct aesd_buffer_entry *ent = NULL, *rtn_ent = NULL;
    size_t char_num = char_offset + 1;
    unsigned int idx = 0, found_ent = 0, ent_idx;

    while ( !found_ent & (idx < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) ) {
        
        ent_idx = ( idx + buffer->in_offs ) % 10;        
        ent = &( buffer->entry[ent_idx] );

        if ( (total_byte_count <= char_num) & (char_num <= (total_byte_count + ent->size) )  ) {
            found_ent = 1;
            *entry_offset_byte_rtn = char_offset - total_byte_count;
            rtn_ent = ent;
        }
        total_byte_count += ent->size;
        idx++;

    }

    return rtn_ent;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    // create a local pointer to struct aesd_buffer_entry
    // point it to the address of the currently in_offs indexed array element in the circ buff
    struct aesd_buffer_entry *ent = &( buffer->entry[ buffer->in_offs ]);

    // now we can just set the values in add_entry to the local struct values
    // which is pointed at the array address
    ent->buffptr = add_entry->buffptr;
    ent->size = add_entry->size;
    
    /*
    // DEBUG
    printf("********* BEGIN aesd_circular_buffer_add_entry *********\n");
    printf("** Before updates **\n");
    printf("**** ent->buffptr: %s\n", ent->buffptr);
    */

    // if buffer is full:
    //  it cannot become unfull in this implementation
    //      i.e. there is no pulling entries out of the buffer
    //  the out_offs will track with in_offs 
    
    if ( buffer->in_offs == (AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED-1) ) {
        // buffer is full, wrap around
        buffer->full = true;
        buffer->in_offs = 0;
    } else {
        // increment buffer
        buffer->in_offs++;
    }

    if ( buffer->full ) {
        buffer->out_offs = buffer->in_offs;
    }

    /*
    // DEBUG
    printf("** After updates **\n");
    printf("**** in_offs: %d\n", buffer->in_offs);
    printf("**** out_offs: %d\n", buffer->out_offs);
    printf("**** full: %d\n", buffer->full);
    uint8_t index;
    struct aesd_buffer_entry *entryptr;
    AESD_CIRCULAR_BUFFER_FOREACH(entryptr, buffer, index) {
        printf("***entryptr->buffptr: %s\n", entryptr->buffptr);
    }
    printf("********* END aesd_circular_buffer_add_entry *********\n");
    */

    return;
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
