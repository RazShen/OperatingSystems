// Raz Shenkman
// 311130777
#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include "threadPool.h"


void hello (void* a)
{
   printf("hello\n");
    sleep(3);
}


void test_thread_pool_sanity()
{
   int i;

   ThreadPool* tp = tpCreate(5);
   
   for(i=0; i<10; ++i)
   {
      tpInsertTask(tp,hello,NULL);
   }
    sleep(5);
   tpDestroy(tp,0);
}


int main()
{
    test_thread_pool_sanity();
   return 0;
}
