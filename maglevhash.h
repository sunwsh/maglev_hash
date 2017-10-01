/*
 * maglevhash.h
 *
 *  Created on: 2016-9-13
 */

#ifndef MAGLEV_HASH_H_
#define MAGLEV_HASH_H_
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <mm_malloc.h>

// maglev entry size (太小不好，不能保证均衡性， 太大每次计算量大， google 论文上建议设置成  真实节点*100 附近的素数)
#define  MAGLEV_HASH_SIZE_MIN   211
#define  MAGLEV_HASH_SIZE_MAX	40009

struct MAGLEV_SERVICE_PARAMS {
	int			m_hash_size;
	int			m_rs_size;

	int			*m_permutation;
	int			*m_next;

	void		**m_rs_info;
	void		**m_entry;

	char		**m_rs_name;
};

struct MAGLEV_LOOKUP_HASH {
	volatile int		is_use_index;
	struct MAGLEV_SERVICE_PARAMS item[2];

	volatile int		is_modify_lock;		// 是否在修改
	struct MAGLEV_SERVICE_PARAMS *p_temp;
};

/*
 * maglev_init()
 * */
void  maglev_init(struct MAGLEV_LOOKUP_HASH *psrv );


/*
 *  maglev_add_serv 初始化一个 meglev 一致性hash缓存 ，
 *     vip      =
 *     rs_size 	= real server number
 *     hash_size = 质数  > rs_size * 100;
 *  return
 *     0 	 success
 *     -1 	== false
 *     -2   == false
 * */
int  maglev_add_serv(struct MAGLEV_LOOKUP_HASH *psrv, int rs_size ,int hash_size );

/*
 *  maglev_add_rs 增加一个 rs服务器的配置， 其中的服务器名称必须是不能重复的
 *  return
 *  	0	== success
 *  	-1	== rs_order 越界
 * */
int  maglev_add_rs(struct MAGLEV_LOOKUP_HASH *psrv,int rs_order,char *p_rs_srv_name ,void  *rs_info );

/*
 *  maglev_create_ht 创建hash 映射表
 * */
void maglev_create_ht(struct MAGLEV_LOOKUP_HASH *psrv);

/*
 *  maglev_dump 打印生成的 hash 表信息
 * */
void maglev_dump(struct MAGLEV_LOOKUP_HASH *psrv);


/*
 *  maglev_swap_entry  移动数据 到 线上 业务
 * */
void maglev_swap_entry(struct MAGLEV_LOOKUP_HASH *psrv);


/*
 *  maglev_lookup_rs 查询一个 key 对应的real server
 * */
void *  maglev_lookup_rs(struct MAGLEV_LOOKUP_HASH *psrv,char *key ,int key_size );


// 用maglev hash时，需要两个不同的hash key 生成算法 ，所以就选择了比较流行的 两个生成算法
/* the famous DJB Hash Function for strings */
unsigned int DJBHash(char *str);
unsigned int  ngx_murmur_hash2(char *data, int len);


#endif /* MAGLEV_HASH_H_ */
