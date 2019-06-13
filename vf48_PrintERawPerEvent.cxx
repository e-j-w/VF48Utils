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

//ROOT classes for plotting
#include "TApplication.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TProfile.h"
#include "TGraph.h"

TApplication *app = NULL;
TCanvas* gWindow;
TH1D* hchan;

int preTrig;

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
  FILE * InpDataFile;
  char   fileName[132];

  if(argc!=2){
    printf("./vf48_PrintERawPerEvent input_data_file\n");
    printf("Prints raw energy per channel per event.\n");
    exit(0);
  }


  if (!app)
    app = new TApplication("testTTC", NULL, NULL);

  if (!gWindow) {
    gWindow = new TCanvas("Window", "window", 1000, 1000);
    gWindow->Clear();
    //gWindow->Divide(2, 4);

    
  }

  hchan = new TH1D("hchan", "chan", 2000, 0, 2000-1);
  hchan->Reset();

  

  //setup the input data file
  sprintf(fileName,argv[1]);
  InpDataFile = fopen(fileName,"rb");
  if(InpDataFile == NULL){
    printf("ERROR: could not read file: %s\n",fileName);
  }
  printf("Opened data file: %s\n",fileName);

  //read raw data file header
  int size = fread(&preTrig,sizeof(int),1,InpDataFile);
  if((size != 1)&&(!(feof(InpDataFile)))){
    printf("File read error!\n");
    printf("Elements read: %i\n",size);
    exit(-1);
  }
  printf("Pre trigger length: %i\n",preTrig);

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
      printf("ERaw per channel: [");
      for(int i=0;i<VF48_MAX_CHANNELS;i++){
        charge = EvalVF48WaveformE(m,i);
        printf(" %f ",charge);
      }
      printf("]\n");
      getc(stdin);
      delete m;
    }

  hchan->Draw();
  gWindow->Modified();
  gWindow->Update();

  printf("Showing ROOT window...\n");
  app->Run(kTRUE);

  //close the data file
  fclose(InpDataFile);

  return 0;
}

