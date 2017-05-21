#ifndef TEST_H
#define TEST_H


#include <stdio.h>


//
// Some helper macros
//


#define TEST(expr) ((expr) ? fprintf(stderr, #expr " PASSED\n") : \
        fprintf(stderr, #expr " FAILED at line %d\n", __LINE__));


#endif // TEST_H
