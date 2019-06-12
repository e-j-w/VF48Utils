#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>

//Class for handling and unpacking VF48 events
#include "UnpackVF48A.h"

#define S32K   32768
#define NSPECT 100

//PARAMETERS
int outHist[NSPECT][S32K];

//function to get energy from VF48 waveforms
double EvalVF48WaveformE(const VF48module *m, int chan)
{
  
  /*//debug info
  printf("VF48 group trigger #s: %i %i %i %i %i %i\n",m->trigno[0],m->trigno[1],m->trigno[2],m->trigno[3],m->trigno[4],m->trigno[5]);
  printf("VF48 group timestamps: %f %f %f %f %f %f\n",m->timestamps[0],m->timestamps[1],m->timestamps[2],m->timestamps[3],m->timestamps[4],m->timestamps[5]);
  printf("charge: %i\n",m->channels[gPlotChan].charge);
  printf("Waveform samples: [");
  for (int i=0; i<m->channels[gPlotChan].numSamples; i++){
    printf(" %i ",m->channels[gPlotChan].samples[i]);
  }
  printf("]\n");*/

  double charge = 0;
  double bgavg = 0;

  //get the baseline for background
  //may want some more robust way to get the number of baseline samples
  for(int i=0; i<30;i++){
    if(i< m->channels[chan].numSamples){
      bgavg += m->channels[chan].samples[i];
    }
  }
  bgavg = bgavg / 30.0;

  //integrate the background subtracted signal
  for(int i=0; i<m->channels[chan].numSamples; i++){
    charge += m->channels[chan].samples[i] - bgavg;
  }
  charge = charge / 100.0; //arbitrary normalization to fit in histogram
  //printf("background average: %f, charge %f\n",bgavg,charge);

  return charge;

}

int main(int argc, char* argv[])
{
  FILE * InpDataFile, *output;
  char   fileName[132], outName[132];
  int gateLow, gateHigh;

  if(argc!=5){
    printf("./vf48_ERaw input_data_file output_data_file gateLow gateHigh\n");
    printf("Saves gated raw energy spectrum to an .mca file.  Gate is in channels.\n");
    exit(0);
  }

  //setup the input data file
  sprintf(fileName,argv[1]);
  InpDataFile = fopen(fileName,"rb");
  if(InpDataFile == NULL){
    printf("ERROR: could not read file: %s\n",fileName);
  }
  printf("Opened data file: %s\n",fileName);

  gateLow = atoi(argv[3]);
  gateHigh = atoi(argv[4]);

  //initialize the output histogram
  for (int i=0;i<NSPECT;i++)
    for (int j=0;j<S32K;j++)
      outHist[i][j]=0;

  int keepEvent,gateCh;
  double charge;
  while(!(feof(InpDataFile)))//go until the end of file is reached
    {
      VF48module* m = (VF48module*)malloc(sizeof(VF48module));
      int size = fread(m,sizeof(VF48module),1,InpDataFile);
      if((size != 1)&&(!(feof(InpDataFile)))){
        printf("File read error!\n");
        printf("Elements read: %i\n",size);
        exit(-1);
      }
      keepEvent = 0;
      gateCh = 0;
      for(int i=0;i<VF48_MAX_CHANNELS;i++){
        charge = EvalVF48WaveformE(m,i);
        if((charge >= gateLow)&&(charge <= gateHigh)){
          keepEvent = 1;
          gateCh = i;
          break;
        }
      }
      if(keepEvent == 1){
        for(int i=0;i<VF48_MAX_CHANNELS;i++){
          if(i != gateCh){
            charge = EvalVF48WaveformE(m,i);
            if(charge >= 0){
              if(charge < S32K){
                outHist[i][(int)charge]++;
              }else{
                outHist[i][S32K-1000]++;
              }
            }
          }
        }
      }
      delete m;
    }
  
  //close the data file
  fclose(InpDataFile);

  //open the output file
  //setup the input data file
  sprintf(outName,argv[2]);
  if((output=fopen(outName,"w"))==NULL)
    {
      printf("ERROR: Cannot open the output file: %s!\n",outName);
      exit(-1);
    }
  
  //write the output histogram to disk
  for (int i=0;i<VF48_MAX_CHANNELS;i++)
    fwrite(outHist[i],S32K*sizeof(int),1,output);
  fclose(output);
  printf("Spectrum written to file: %s\n",outName);
  

  return 0;
}

