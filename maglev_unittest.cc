// Copyright 2005, Google Inc.
// All rights reserved.
//
// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//

#include <limits.h>
#include "maglevhash.h"
#include "gtest/gtest.h"

// Tests factorial of negative numbers.
TEST(FactorialTest, Negative) {
  // This test is named "Negative", and belongs to the "FactorialTest"
  // test case.
  struct MAGLEV_LOOKUP_HASH  m_maglev_hash;
  maglev_init( &m_maglev_hash );
  
  EXPECT_EQ(0, maglev_add_serv( &m_maglev_hash ,2,103) );
  int i;
  for(i=0;i < 2;i++)
  {
	uint64_t *rsdesc = NULL;
        char descname[100];
        
	maglev_add_rs( &m_maglev_hash ,i,descname,rsdesc );
  }

  maglev_create_ht( &m_maglev_hash );

  maglev_dump( &m_maglev_hash );

  maglev_swap_entry( &m_maglev_hash );
  // </TechnicalDetails>
}

