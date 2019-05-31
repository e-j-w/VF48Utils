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
   int cmode;
   DWORD fwrev;

   if(argc!=2)
     {
       printf("./vf48_isPresent module_id\n");
       exit(0);
     }

   vf48base+=atoi(argv[1])*0x10000;
   
   // open under vmic   
   mvme_open(&myvme, 0);
 
   mvme_get_dmode(myvme, &cmode);
   mvme_set_dmode(myvme, MVME_DMODE_D32);

   fwrev = vf48_RegisterRead(myvme, vf48base, VF48_FIRMWARE_R);

   printf("\n----------------------> ");
   if (fwrev == 0xFFFFFFFF) 
     printf("VF48 at VME A24 0x%06x not detected\n", vf48base);
   else
     printf("VF48 at VME A24 0x%06x detected\n", vf48base);


   //close under vmic
   mvme_close(myvme);
 

  return 0;
}

