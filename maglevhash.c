/*
 * maglevhash.h
 *
 *  Created on: 2016-9-13
 */

#include "maglevhash.h"
#include <string.h>
#include <math.h>


void  maglev_init(struct MAGLEV_LOOKUP_HASH *psrv )
{
	psrv->is_use_index		= -1;
	psrv->is_modify_lock	= 0;
	psrv->p_temp			= NULL;

	psrv->item[0].m_hash_size	= 0;
	psrv->item[1].m_hash_size	= 0;

	psrv->item[0].m_rs_size	= 0;
	psrv->item[1].m_rs_size	= 0;

	psrv->item[0].m_permutation	= NULL;
	psrv->item[1].m_permutation	= NULL;

	psrv->item[0].m_next		= NULL;
	psrv->item[1].m_next		= NULL;

	psrv->item[0].m_entry		= NULL;
	psrv->item[1].m_entry		= NULL;

	psrv->item[0].m_rs_name	= NULL;
	psrv->item[1].m_rs_name	= NULL;

	psrv->item[0].m_rs_info	= NULL;
	psrv->item[1].m_rs_info	= NULL;

}

static int8_t __is_maglev_prime(int n)
{
    if (n < MAGLEV_HASH_SIZE_MIN)
        return 0;
    if (n > MAGLEV_HASH_SIZE_MAX)
        return 0;
    if (n % 2 == 0)
        return 0;

    int i, j;
    j = (int) sqrt(n + 1);
    for (i = 3; i <= j; i = i + 2)
        if (n % i == 0)
            return 0;
    return 1;
}

static struct MAGLEV_SERVICE_PARAMS*  __create_maglev_service_unit(struct MAGLEV_SERVICE_PARAMS* pServ,int  rs_size , int hash_size )
{
	if (0 == __is_maglev_prime(hash_size)) {
		return NULL;
	}

	pServ->m_hash_size			= hash_size;
	pServ->m_rs_size			= rs_size;
	pServ->m_permutation		= (int *) malloc(rs_size * hash_size * sizeof(int));
	pServ->m_rs_info			= (void **) malloc(rs_size * sizeof(void *) );

	pServ->m_next				= (int *) malloc(rs_size * sizeof(int) );
	pServ->m_entry				= (void **) malloc(hash_size * sizeof(void *) );

	if(NULL == pServ->m_entry )
	{
		return NULL;
	}
	pServ->m_rs_name			= (char **) malloc(rs_size * sizeof(char *) );

	return pServ;
}

static void maglev_loopup_item_clean(struct MAGLEV_LOOKUP_HASH *psrv,int index)
{
	if(NULL == psrv->item[index].m_entry )
	{
		return;
	}

	struct MAGLEV_SERVICE_PARAMS  *p_item	= & psrv->item[index];

	int i;
	for(i=0;i < p_item->m_rs_size; i++)
	{
		free( *( p_item->m_rs_name + i ) );
	}
	free( p_item->m_rs_name );
	free( p_item->m_entry );
	free( p_item->m_rs_info );

	// free
	free( p_item->m_permutation );
	free( p_item->m_next );

	p_item->m_rs_name	= NULL;
	p_item->m_entry		= NULL;
	p_item->m_rs_info	= NULL;
	p_item->m_permutation	= NULL;
	p_item->m_next			= NULL;

	p_item->m_hash_size		= 0;
	p_item->m_rs_size		= 0;

}

/*
 *  maglev_add_serv 初始化一个 meglev 一致性hash缓存 ，
 *  rs_size = real server number
 *  hash_size = 质数  > rs_size * 100;
 * */
int  maglev_add_serv(struct MAGLEV_LOOKUP_HASH *psrv, int rs_size ,int hash_size )
{
	if(psrv->is_modify_lock)
	{
		return -1;
	}

	psrv->is_modify_lock	= 1;

	int	 	i_index	= (psrv->is_use_index + 1 ) % 2;
	maglev_loopup_item_clean(psrv,i_index);

	psrv->p_temp	=  __create_maglev_service_unit(&psrv->item[i_index],rs_size,hash_size);

	//printf("%s,vip:%u ,rs_size:%u ,hash_size:%u  \n ", __FUNCTION__ , psrv->vaddr, rs_size, hash_size);

	if(NULL == psrv->p_temp )
	{
		return -1;
	}
	return 0;
}

/*
 *  maglev_add_rs 增加一个 rs服务器的配置， 其中的服务器名称必须是不能重复的
 * */
int  maglev_add_rs(struct MAGLEV_LOOKUP_HASH *psrv,int rs_order,char *p_rs_srv_name ,void  *rs_info )
{
	if( 0 == psrv->is_modify_lock)
	{
		return -2;
	}

	if(rs_order > psrv->p_temp->m_rs_size )
	{
		return -1;
	}

	int M				= psrv->p_temp->m_hash_size; 			// 用了论文中的 变量名
	int *permutation	= psrv->p_temp->m_permutation;

	unsigned int offset = DJBHash(p_rs_srv_name);
	offset				= offset % M;
	unsigned int skip	= ngx_murmur_hash2(p_rs_srv_name,strlen(p_rs_srv_name) );
	skip				= skip % ( M -1 ) + 1;

	void **cur_rs_info	= psrv->p_temp->m_rs_info;
	*( cur_rs_info + rs_order) = rs_info;

	{  // 保存  rs服务器 关键名称
		int  cur_name_size	= strlen(p_rs_srv_name) + 1;
		char *cur_rs_name			= (char *) malloc( cur_name_size );

		snprintf(cur_rs_name, cur_name_size , "%s", p_rs_srv_name);
		*(psrv->p_temp->m_rs_name + rs_order )	= cur_rs_name;

		// printf("%s test rs_order:%u ,%s \n", __FUNCTION__, rs_order, *(pServ->m_rs_name + rs_order ) );
	}


	int j ;
	for(j = 0; j < psrv->p_temp->m_hash_size ; j++ )
	{
		int perm	= (offset + j * skip) % M;
		*(permutation + rs_order * M + j )	= perm;
		// printf( "%s maglev_addrs name:%s,m:%d,index:%d,offset:%d,skip:%d,permutation:%d ,rs_info:%llu \n",
	}

	return 0;
}

void maglev_create_ht(struct MAGLEV_LOOKUP_HASH *psrv)
{
	if( 0 == psrv->is_modify_lock)
	{
		return;
	}

	struct MAGLEV_SERVICE_PARAMS *pServ		= psrv->p_temp;

	int N				= pServ->m_rs_size ;
	int M				= pServ->m_hash_size ;		// 用了论文中的 变量名
	int *permutation	= pServ->m_permutation ;

	int	*next			= pServ->m_next ;
	void **entry		= pServ->m_entry ;
	void **cur_rs_info	= pServ->m_rs_info ;

	int j;
	for(j=0;j<N;j++)
	{
		*( next + j )  = 0;
	}

	for(j=0;j < M;j++ )
	{
		*( entry + j )  = NULL;
	}

	printf("%s start create ht \n", __FUNCTION__);

	int n	= 0;
	int is_run	= 1;

	while( is_run )
	{
		int i;
		for(i=0;i< N; i++)
		{
			int lsnext	= *( next + i );
			int c = *(permutation + i * M + lsnext );

			while( *(entry + c ) != NULL  )
			{
				// printf("while next[ %d ] = %d, c=%d entry=%d --",i, *( next + i ),c ,*(entry + c ) );
				*( next + i ) 	= *( next + i ) +1;
				lsnext			= *( next + i );
				c = *(permutation + i * M + lsnext );
				// printf("next[ %d ] = %d, c=%d \n",i, *( next + i ),c );
			}

			// *(entry + c )	= i;
			*(entry + c)		= *(cur_rs_info + i );
			*( next + i ) 	= *( next + i ) +1;
			n++;
			if( n == M )
			{
				is_run	= 0;
				break;
			}
		}

	}

}

void maglev_dump(struct MAGLEV_LOOKUP_HASH *psrv)
{

	if( 0 == psrv->is_modify_lock)
	{
	    printf("%s maglev_dump null\n", __FUNCTION__);
		return;
	}

	struct MAGLEV_SERVICE_PARAMS *pServ		= psrv->p_temp;

		int *permutation	= pServ->m_permutation ;

		int rs_size 		= pServ->m_rs_size ;
		int hash_size 		= pServ->m_hash_size ;

		int j;
		// void **cur_rs_info	= pServ->m_rs_info ;
		// char	 **cur_rs_name	= pServ->m_rs_name ;

		for(j=0;j < rs_size; j++)
		{
		    // printf("maglev dump vip:%s rsid:%d  rsinfo:%llu,[", ip_to_str(psrv->vaddr), j, (long long unsigned int) *(cur_rs_info + j) );
			int m;
			for(m=0;m < hash_size;m++ )
			{
			    printf("%d,", *(permutation + j * hash_size + m ) );
			}
			printf("] \n");
		}

		/*
		uint64_t **entry	= pServ->m_entry ;
		for(j=0; j < hash_size; j++ )
		{
			printf("maglev dump hash key:%d  rs:%llu \n ",j, *( entry + j ) );
		}
		*/

}




void maglev_swap_entry(struct MAGLEV_LOOKUP_HASH *psrv)
{

	if( 0 == psrv->is_modify_lock)
	{
		return;
	}

	int	 	i_index	= (psrv->is_use_index + 1 ) % 2;

	psrv->is_use_index		= i_index;

	psrv->p_temp			= NULL;
	psrv->is_modify_lock	= 0;
}

/*
 *  maglev_lookup_rs 查询一个 key 对应的real server
 * */
void *  maglev_lookup_rs(struct MAGLEV_LOOKUP_HASH *psrv,char *key ,int key_size )
{
	int	 	i_index	= psrv->is_use_index;
	if(i_index < 0  )
	{
		return NULL;
	}
	if( 0 >= psrv->item[ i_index ].m_hash_size )
	{
		return NULL;
	}

	void * prs_info;

	unsigned int  new_key	= ngx_murmur_hash2(key,key_size);
	int M				= psrv->item[ i_index ].m_hash_size ;		// 用了论文中的 变量名
	void **entry	= psrv->item[ i_index ].m_entry ;

	unsigned int  hashkey	= new_key % M;
	prs_info	= *(entry + hashkey );

	return prs_info;
}

/* the famous DJB Hash Function for strings */
unsigned int DJBHash(char *str)
{
    unsigned int hash = 5381;
    while (*str){
        hash = ((hash << 5) + hash) + (*str++); /* times 33 */
    }
    hash &= ~(1 << 31); /* strip the highest bit */
    return hash;
}

unsigned int  ngx_murmur_hash2(char *data, int len)
{
	unsigned int  h, k;

    h = 0 ^ len;

    while (len >= 4) {
        k  = data[0];
        k |= data[1] << 8;
        k |= data[2] << 16;
        k |= data[3] << 24;

        k *= 0x5bd1e995;
        k ^= k >> 24;
        k *= 0x5bd1e995;

        h *= 0x5bd1e995;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
    case 3:
        h ^= data[2] << 16;
    case 2:
        h ^= data[1] << 8;
    case 1:
        h ^= data[0];
        h *= 0x5bd1e995;
    }

    h ^= h >> 13;
    h *= 0x5bd1e995;
    h ^= h >> 15;

    return h;
}



