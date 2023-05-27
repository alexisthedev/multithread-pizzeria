#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Constants Definition

#define N_COOK 2
#define N_OVEN 15
#define N_PACKER 2
#define N_DELI 10
#define T_ORD_LO 1
#define T_ORD_HI 3
#define N_ORD_LO 1
#define N_ORD_HI 5
#define P_PLAIN 60
#define T_PAY_LO 1
#define T_PAY_HI 3
#define P_FAIL 10
#define C_PLAIN 10
#define C_SPECIAL 12
#define T_PREP 1
#define T_BAKE 10
#define T_PACK 1
#define T_DEL_LO 5
#define T_DEL_HI 15
