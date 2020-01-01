/*
 * maglevhash.h
 *
 *  Created on: 2017-9-13
 */

#ifndef MAGLEV_HASH_H_
#define MAGLEV_HASH_H_

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <mm_malloc.h>

/*
 * maglev entry size
 * Too small can not guarantee the balance, too large calculations each time,
 * it is recommended to set the prime number near the real node * 100.
*/
#define  MAGLEV_HASH_SIZE_MIN   211
#define  MAGLEV_HASH_SIZE_MAX	40009

struct MAGLEV_SERVICE_PARAMS
{
	int     node_size;
	int     node_add_index;
	void    **node_info_entry;
	char    **node_name;

	int     hash_bucket_size;
	void    **hash_entry;

	int     *permutation;
	int     *next;
};

struct MAGLEV_LOOKUP_HASH
{
	volatile int is_use_index;
	volatile int is_modify_lock;

	struct MAGLEV_SERVICE_PARAMS item[2];
	struct MAGLEV_SERVICE_PARAMS *p_temp;
};

/*
 * maglev_init()
 * */
void  maglev_init(struct MAGLEV_LOOKUP_HASH *psrv);

/*
 *  maglev_update_service
 *  Initialize a maglev consistent hash cache，
 *     node_size 	    = real server number
 *     hash_bucket_size = prime  > node_size * 100;
 *  return
 *     0 	== success
 *     -1 	== false
 * */
int  maglev_update_service(struct MAGLEV_LOOKUP_HASH *psrv, int node_size, int hash_bucket_size);

/*
 *  maglev_add_node
 *      Add a node, the node_name_key must be unique
 *  return
 *  	0	== success
 *  	-1	== false
 * */
int  maglev_add_node(struct MAGLEV_LOOKUP_HASH *psrv, char *node_name_key, void  *rs_info);

/*
 *  maglev_create_ht
 *  create hash table
 * */
void maglev_create_ht(struct MAGLEV_LOOKUP_HASH *psrv);


/*
 *  maglev_swap_entry
 * */
void maglev_swap_entry(struct MAGLEV_LOOKUP_HASH *psrv);


/*
 *  maglev_lookup_node
 * */
void *  maglev_lookup_node(struct MAGLEV_LOOKUP_HASH *psrv, char *key, int key_size);


/* the famous DJB Hash Function for strings */
unsigned int DJBHash(char *str);
unsigned int ngx_murmur_hash2(char *data, int len);


#endif /* MAGLEV_HASH_H_ */
