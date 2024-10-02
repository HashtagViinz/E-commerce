#ifndef main_h
#define main_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ctime>
#include <vector>



#include "con2redis.h"
#include "Customer.h"

#define CUSTOMER_NUM 50           // ? Numero di Customer che genereremo


int micro_sleep(long usec);

#endif
