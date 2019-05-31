#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>



#include "vf48.h"


int main(int argc, char* argv[])
{
   DWORD vf48base=0xa00000;   
   MVME_INTERFACE* myvme;

   if(argc!=2)
     {
       printf("./vf48_Status module_id\n");
       exit(0);
     }

   vf48base+=atoi(argv[1])*0x10000;
   
   // open under vmic   
   mvme_open(&myvme, 0);
   
   vf48_Status(myvme, vf48base);

   //close under vmic
   mvme_close(myvme);
  return 0;
}

