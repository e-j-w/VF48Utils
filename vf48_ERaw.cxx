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

int preTrig,vf48samples;
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
  for(int i=0; i<preTrig;i++){
    if(i< m->channels[chan].numSamples){
      bgavg += m->channels[chan].samples[i];
    }
  }
  bgavg = bgavg / (preTrig*1.0);

  //find the maximum value in the waveform
  int maxVal = 0;
  int maxChan = 0;
  for(int i=0; i<m->channels[chan].numSamples; i++){
    if(m->channels[chan].samples[i] > maxVal){
      maxChan = i;
      maxVal = m->channels[chan].samples[i];
    }
  }
  
  int upperLim = 0;
  int lowerLim = 0;
  //find the integration limits
  for(int i=maxChan; i>0; i--){
    if(m->channels[chan].samples[i] < bgavg){
      lowerLim = i+1;
      break;
    }
  }
  if(lowerLim < preTrig){
    lowerLim = preTrig;
  }
  for(int i=maxChan; i<m->channels[chan].numSamples; i++){
    if(m->channels[chan].samples[i] < bgavg){
      upperLim = i-1;
      break;
    }
  }

  //integrate the background subtracted signal
  for(int i=lowerLim; i<=upperLim; i++){
    //don't integrate below the baseline
    if(m->channels[chan].samples[i] > bgavg){
      charge += m->channels[chan].samples[i] - bgavg;
    }
  }
  //charge = charge / 100.0; //arbitrary normalization to fit in histogram
  
  //printf("background average: %f, lower limit: %i, upper limit: %i, charge: %f\n",bgavg,lowerLim,upperLim,charge);
  //getc(stdin);

  return charge;

}

int main(int argc, char* argv[])
{
  FILE * InpDataFile, *output;
  char   fileName[132], outName[132];

  if(argc!=3){
    printf("./vf48_ERaw input_data_file output_data_file\n");
    printf("Saves raw energy spectrum to an .mca file.\n");
    exit(0);
  }

  //setup the input data file
  sprintf(fileName,argv[1]);
  InpDataFile = fopen(fileName,"rb");
  if(InpDataFile == NULL){
    printf("ERROR: could not read file: %s\n",fileName);
  }
  printf("Opened data file: %s\n",fileName);

  //initialize the output histogram
  for (int i=0;i<NSPECT;i++)
    for (int j=0;j<S32K;j++)
      outHist[i][j]=0;

  //read raw data file header
  int size = fread(&preTrig,sizeof(int),1,InpDataFile);
  if((size != 1)&&(!(feof(InpDataFile)))){
    printf("File read error!\n");
    printf("Elements read: %i\n",size);
    exit(-1);
  }
  size = fread(&vf48samples,sizeof(int),1,InpDataFile);
  if((size != 1)&&(!(feof(InpDataFile)))){
    printf("File read error!\n");
    printf("Elements read: %i\n",size);
    exit(-1);
  }
  printf("Pre trigger length: %i\nSamples per waveform: %i\n",preTrig,vf48samples);

  double charge;
  while(!(feof(InpDataFile)))//go until the end of file is reached
    {
      VF48module* m = (VF48module*)malloc(sizeof(VF48module));
      size = fread(m,sizeof(VF48module),1,InpDataFile);
      if((size != 1)&&(!(feof(InpDataFile)))){
        printf("File read error!\n");
        printf("Elements read: %i\n",size);
        exit(-1);
      }
      for(int i=0;i<VF48_MAX_CHANNELS;i++){
        charge = EvalVF48WaveformE(m,i);
        if(charge >= 0){
          if(charge < S32K){
            outHist[i][(int)charge]++;
          }else{
            outHist[i][S32K-1000]++;
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

