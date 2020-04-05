#include <stdlib.h>
#include <stdio.h>
#include <time.h>


int init=0;

int random(int imax)
{
    if (init==0)
        srand( (unsigned)time( NULL ) );
    init=1;
    return (int)((rand()*((long)imax))/RAND_MAX);

}