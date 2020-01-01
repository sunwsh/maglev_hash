// Copyright 2005, Google Inc.
// All rights reserved.
//
// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//

#include <limits.h>
#include "maglevhash.h"
#include "gtest/gtest.h"
#include <string.h>
//#include <cstring>

//using std::string;

// Tests factorial of negative numbers.
TEST(FactorialTest, Negative) {
  // This test is named "Negative", and belongs to the "FactorialTest"
  // test case.
  struct MAGLEV_LOOKUP_HASH  m_maglev_hash;
  int REAL_SERVER_NUMB	= 3;
  int rs_entry_count[3] = {0};

  maglev_init( &m_maglev_hash );
  
  EXPECT_EQ(0, maglev_update_service( &m_maglev_hash , REAL_SERVER_NUMB, 313) );
  int i;
  for(i=0;i < REAL_SERVER_NUMB; i++)
  {
	char *rsdesc = (char *) malloc(2);
	snprintf(rsdesc, 2,"%d", i);
    char descname[100];
    snprintf(descname, sizeof(descname), "rs:%d", i);

    maglev_add_node(&m_maglev_hash, descname, rsdesc);
    rs_entry_count[i] = 0;
  }

  maglev_create_ht( &m_maglev_hash );

  maglev_swap_entry( &m_maglev_hash );

  // </TechnicalDetails>
  struct MAGLEV_SERVICE_PARAMS *temp_srv = &m_maglev_hash.item[ m_maglev_hash.is_use_index ];
  for(i = 0; i < temp_srv->hash_bucket_size; i++ ) {
	  if (0 == strcmp((char *)(temp_srv->hash_entry[i]), "0")) {
		  rs_entry_count[0]++;
	  }
	  if (0 == strcmp((char *)(temp_srv->hash_entry[i]), "1")) {
		  rs_entry_count[1]++;
	  }
	  if (0 == strcmp((char *)(temp_srv->hash_entry[i]), "2")) {
		  rs_entry_count[2]++;
	  }
  }

  printf("hash size:%d\n", temp_srv->hash_bucket_size);
  for(i = 0; i < REAL_SERVER_NUMB; i++) {
	  printf("node: %s, count: %d\n", (char *)temp_srv->node_info_entry[i], rs_entry_count[i]);
  }
}

int main(int argc,char **argv){
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
