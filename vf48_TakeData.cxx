#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>

//VF48 driver
extern "C"
{
#include "vf48.h"
}

//Class for handling and unpacking VF48 events
#include "UnpackVF48A.h"
UnpackVF48* vfu = new UnpackVF48();

int vf48samples = 800;
int mbits1 = 0x0100;
int mbits2 = 0x0101;

double utime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  //printf("gettimeofday: %d %d\n", tv.tv_sec, tv.tv_usec);
  return tv.tv_sec + 0.000001*tv.tv_usec;
}

//function to print VF48 events
void PrintVF48event(const VF48event *e)
{
  int gPlotChan = 1; //channel to plot
  //debug info
  printf("VF48 event!\n");
  //e->PrintSummary();
  //e->PrintEvent();
  printf("Waveform samples: [");
  for (int i=0; i<vf48samples; i++)
    printf(" %i ",e->modules[0]->channels[gPlotChan].samples[i]);
  printf("] (%i samples total)\n",vf48samples);
  printf("event number: %i, event timestamp: %f\n",e->eventNo,e->timestamp);
  printf("channel: %d\n",e->modules[0]->channels[gPlotChan].channel);
  printf("time: %d\n",e->modules[0]->channels[gPlotChan].time);
  printf("charge: %d\n",e->modules[0]->channels[gPlotChan].charge);
  getc(stdin);

}

int main(int argc, char* argv[])
{
  DWORD vf48base=0xa00000;   
  MVME_INTERFACE* myvme;
  int grpEnabled = 0x3F; //what is this?
  int noreset = 0;
  FILE * DataFile;
  char   fileName[132];

  //counters
  int i,j,iloop;

  int nloops = 5000;
  int once = 0; //bool, set to true if running only once 

  int countReadErrors = 0;
  int countBabbling = 0;

  if(argc!=3){
    printf("./vf48_TakeData module_id output_data_file\n");
    exit(0);
  }

  //get the vf48 module address
  vf48base+=atoi(argv[1])*0x10000;

  // open under vmic   
  int status = mvme_open(&myvme, 0);

  // Set am to A24 non-privileged Data
  mvme_set_am(myvme, MVME_AM_A24);

  // Set dmode to D32
  mvme_set_dmode(myvme, MVME_DMODE_D32);


  int have_fifo_incomplete = 0;
  DWORD fwrev = vf48_RegisterRead(myvme, vf48base, VF48_FIRMWARE_R);
  if ((fwrev&0xFF000000)==0x07000000) 
      if (fwrev >= 0x07110726)
        have_fifo_incomplete = 1;
  fwrev = vf48_RegisterRead(myvme, vf48base, VF48_FIRMWARE_R);
  printf("\n----------------------> ");
  if (fwrev == 0xFFFFFFFF) 
    printf("VF48 at VME A24 0x%06x not detected\n", vf48base);
  else
    printf("VF48 at VME A24 0x%06x detected\n", vf48base);


  printf("Setting up the VF48 at 0x%x!\n", vf48base);
  if (!noreset)
    {
      status = vf48_Reset(myvme, vf48base);
      if (status != VF48_SUCCESS)
          {
            printf("Cannot reset VF48 at 0x%x\n", vf48base);
            exit(1);
          }
      sleep(1);
    }
  vf48_AcqStop(myvme, vf48base);
  //vf48_Status(myvme, vf48base);

  printf ("Setting up VF48 parameters...\n");
  vf48_GrpEnable(myvme, vf48base, grpEnabled);
  //write the number of samples per waveform to the VF48 module
  for (j=0; j<6; j++)
    if (grpEnabled&(1<<j))
      vf48_ParameterWrite(myvme, vf48base, j, VF48_SEGMENT_SIZE, vf48samples);

  //write the first set of mbits to the VF48 module
  if (mbits1 >= 0)
    for (j=0; j<6; j++)
      if (grpEnabled&(1<<j))
        vf48_ParameterWrite(myvme, vf48base, j, VF48_MBIT1, mbits1);

  //write the second set of mbits to the VF48 module
  if (mbits2 >= 0)
    for (j=0; j<6; j++)
      if (grpEnabled&(1<<j))
        vf48_ParameterWrite(myvme, vf48base, j, VF48_MBIT2, mbits2);

  //write the hit detection threshold to the VF48 module
  int hitThreshold = 10;
  if (hitThreshold >= 0)
    for (j=0; j<6; j++)
      if (grpEnabled&(1<<j))
        vf48_ParameterWrite(myvme, vf48base, j, VF48_HIT_THRESHOLD, hitThreshold);

  //write the trigger threshold to the VF48 module
  int trigThreshold = 500;
  if (trigThreshold >= 0)
    for (j=0; j<6; j++)
      if (grpEnabled&(1<<j))
        vf48_ParameterWrite(myvme, vf48base, j, VF48_TRIG_THRESHOLD, trigThreshold);
  
  //write the pretrigger sample count to the VF48 module
  int preTrig = 500;
  if (preTrig >= 0)
    for (j=0; j<6; j++)
      if (grpEnabled&(1<<j))
        vf48_ParameterWrite(myvme, vf48base, j, VF48_PRE_TRIGGER, preTrig);
  //read back the parameter
  preTrig = vf48_ParameterRead(myvme, vf48base, 0, VF48_PRE_TRIGGER);

  /*//write the trigger threshold to the VF48 module
  int pedestal = 100;
  if (trigThreshold >= 0)
    for (j=0; j<6; j++)
      if (grpEnabled&(1<<j))
        vf48_ParameterWrite(myvme, vf48base, j, VF48_PEDESTAL, pedestal);*/
  


  //write the K,L,M values to the VF48 module
  int kVal = 80;
  int lVal = 72;
  int mVal = 50;
  if (trigThreshold >= 0)
    for (j=0; j<6; j++)
      if (grpEnabled&(1<<j)){
        vf48_ParameterWrite(myvme, vf48base, j, VF48_K_COEF, kVal);
        vf48_ParameterWrite(myvme, vf48base, j, VF48_L_COEF, lVal);
        vf48_ParameterWrite(myvme, vf48base, j, VF48_M_COEF, mVal);
      }
        
  
  vf48_Status(myvme, vf48base);
  //getc(stdin);


  vfu->SetNumModules(1);
  vfu->SetGroupEnableMask(-1, grpEnabled);
  vfu->SetNumSamples(-1, vf48samples);
  vfu->SetCoincTime(0.000001);
  vfu->Reset();

  //external trigger
  int extTrig = 1;

  //setup the data file
  sprintf(fileName,argv[2]);
  DataFile = fopen(fileName,"wb");
  printf("\n\nWriting VF48 event data to file: %s\n",fileName);

  //write the file header
  fwrite(&preTrig,sizeof(int),1,DataFile);
  fwrite(&vf48samples,sizeof(int),1,DataFile);


  //MAIN DATA-TAKING LOOP
  vf48_ExtTrgSet(myvme, vf48base); //setup the external trigger
  vf48_AcqStart(myvme, vf48base); //start the acquisition
  double tstart = utime();
  double bstart = 0;
  int countTrigger = 0;
  while(1)
    {

      if (extTrig)
        {
            //printf("Ext Pulse!\n");
        }
      else if (1)
        {
          //usleep(100000);
          vf48_Trigger(myvme, vf48base); //trigger regardless of what the external trigger is doing
          countTrigger++;
        }
      
      // wait until the VF48 has data to read
      double tt = utime();
      while (1)
        {
          
          int csr = vf48_CsrRead(myvme, vf48base);
          int haveData = vf48_NFrameRead(myvme, vf48base);
          
          //printf("VF48 waiting for data: csr 0x%08x, nframe %i\n", csr, haveData);

          int fifo_not_empty = ((csr & VF48_CSR_FIFO_EMPTY) == 0);

          if (fifo_not_empty && haveData > 10)
            break;
          
          if (have_fifo_incomplete)
            if ((csr & VF48_CSR_FIFO_INCOMPLETE) == 0)
              if (fifo_not_empty)
                  break;

          //usleep(10);
          
          double t = utime() - tt;
          //printf("t = %f\n",t);
          if (t < 5)
            continue;
          
          printf("Timeout waiting for data, csr 0x%x, nframe %d!\n", vf48_CsrRead(myvme, vf48base), vf48_NFrameRead(myvme, vf48base));

          vf48_Status(myvme, vf48base);

          //exit(123);
          //vf48_Reset(myvme, vf48base);

          exit(-1);
        }
      
      // read the data
      const int kSize32 = 500000;
      uint32_t pdata32[kSize32];
      int iptr = 0;
      
      double t0 = utime();
      double xt = 0;
      
      //sleep(1);
      int nread = 0;
      while (1)
        {
            int csr = vf48_CsrRead(myvme, vf48base);
            int haveData = vf48_NFrameRead(myvme, vf48base);

            //printf("VF48 csr 0x%08x, has data: %6d, iptr: %6d, xt: %f\n", csr, haveData, iptr, xt);
            //vf48_Status(myvme, vf48base);

            if (haveData == 0)
              break;

            int read_last_data = 0;

            if (have_fifo_incomplete)
              if ((csr & VF48_CSR_FIFO_INCOMPLETE)==0)
                  read_last_data = 1;
      
            if (!read_last_data && haveData < 2000)
              {
                  int xframe = haveData;
                  int nframe;
                  double tt0 = utime();
                  //double tt00 = tt0;

                  // delay 10 usec
                  for (i=0; i<100; i++)
                    {
                        for (j=0; j<10; j++)
                          csr = vf48_CsrRead(myvme, vf48base);

                        if (have_fifo_incomplete)
                          if ((csr & VF48_CSR_FIFO_INCOMPLETE)==0)
                              read_last_data = 1;
      
                        nframe = vf48_NFrameRead(myvme, vf48base);

                        if (read_last_data || nframe > 2000)
                          break;

                        double now = utime();

                        if (nframe != xframe)
                          {
                              //printf("VF48 has data: %6d, new: %6d, time %f\n", haveData, nframe, now-tt0);

                              xframe = nframe;
                              tt0 = now;
                              continue;
                          }

                        if (now - tt0 > 0.000200)
                          break;
                    }
                  
                  haveData = nframe;

                  //printf("VF48 has data: %6d, wait time %f\n", haveData, utime()-tt00);
              }

            int count = haveData&~0xF;

            //if (haveData > 2000)
            //count = 10000;
    
            if (read_last_data || haveData < 2000)
              count += 16;
      
            csr = vf48_CsrRead(myvme, vf48base);

            //printf("VF48 csr 0x%08x, haveData %d, count %d\n", csr, haveData, count);

            if (count == 0)
              continue;

            if (iptr + count + 10 > kSize32)
              {
                  printf("VF48 is babbling, nframe %d!\n", vf48_NFrameRead(myvme, vf48base));
                  countBabbling++;
                  break;
              }

            while (0)
              {
                  int nf = vf48_NFrameRead(myvme, vf48base);
                  mvme_set_dmode(myvme, MVME_DMODE_D32);
                  uint32_t dd = mvme_read_value(myvme, vf48base + VF48_DATA_FIFO_R);
                  printf("nframe %d, data 0x%08x\n", nf, dd);
                  sleep(1);
              }

      
            double xt0 = utime();
      
            //printf("VF48 has data: %6d, iptr: %6d, count: %d, xt: %f\n", haveData, iptr, count, xt);

            nread++;

            int wc = 0;

            if (1) // read using DMA
              {
                  //printf("vf48_DataRead() called with iptr %d, count %d...\n", iptr, count);

                  //pdata32[iptr++] = 0xdeadbeef;
                  //pdata32[iptr++] = 0xdeadbeef;

                  while (((uint64_t)(pdata32+iptr)&0xf) != 0)
                    pdata32[iptr++] = 0xdeadbeef;

                  //printf("DMA to %p, wc %d\n", pdata32+iptr, count);

                  wc = vf48_DataRead(myvme, vf48base, pdata32+iptr, &count);
                  if (wc != count)
                    {
                        printf("Error reading VF48 data!\n");
                        countReadErrors++;
                        //exit(1);
                    }

                  //printf("vf48_DataRead() returned %d\n", wc);

                  if (0)
                    {
                        printf("read of %d words\n", count);
                        for (i=0; i<count; i++)
                          printf("count %d, word 0x%08x\n", i, pdata32[iptr+i]);
                    }
              }

            double xt1 = utime();
            double dxt = xt1 - xt0;

            //exit(123);
      
            xt += dxt;
      
            //printf("Data read: wc: %d, count: %d, %f us\n", wc, count, dxt);
      
            iptr += wc;
        }

      double t1 = utime();
  
      double t = t1 - t0;
  
      //printf("time %f %f %f\n", t0, t1, t);
  
      bstart += 4*iptr;
      double tnow = utime() - tstart;
  
      printf("Event read: %d words in %f sec in %d reads, %f sec DMA, %6.3f Mbytes/sec per event, %6.3f Mbytes/sec per DMA, %6.3f Mbytes/sec, %.0f evt/s sustained!\r", iptr, t, nread, xt, 4*iptr/t/1000000.0, 4*iptr/xt/1000000.0, bstart/tnow/1000000.0, (iloop+1)/tnow);

      int unit = 0;
      vfu->fBadDataCount = 0;
      vfu->UnpackStream(unit, pdata32, iptr);
      while (1) {
          VF48event* e = vfu->GetEvent();
          if (!e)
            break;
          //PrintVF48event(e);
          //write event data to disk
          fwrite(e->modules[0],sizeof(VF48module),1,DataFile);
          delete e;
      }
      //fwrite(vfu,sizeof(UnpackVF48),1,DataFile);

      //exit if only running once
      if (once)
          exit(0);

      //usleep(250000); //wait a bit so that we don't break anything

    }

  //close the data file
  
  fclose(DataFile);

  //close under vmic
  mvme_close(myvme);
  return 0;
}

