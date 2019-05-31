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

int gPlotChan = 1; //channel to plot

TApplication *app = NULL;
TCanvas* gWindow;
TH1D* hchan;

//function to print VF48 events
void ShowVF48event(const VF48module *m)
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
    if(i< m->channels[gPlotChan].numSamples){
      bgavg += m->channels[gPlotChan].samples[i];
    }
  }
  bgavg = bgavg / 30.0;

  //integrate the background subtracted signal
  for(int i=0; i<m->channels[gPlotChan].numSamples; i++){
    charge += m->channels[gPlotChan].samples[i] - bgavg;
  }
  charge = charge / 100.0; //arbitrary normalization to fit in histogram
  printf("background average: %f, charge %f\n",bgavg,charge);

  hchan->Fill(charge);

}

int main(int argc, char* argv[])
{
  FILE * InpDataFile;
  char   fileName[132];

  if(argc!=3){
    printf("./vf48_ERaw input_data_file channel\n");
    printf("Shows raw energy spectrum in a ROOT window.");
    exit(0);
  }

  gPlotChan = atoi(argv[2]);


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


  while(!(feof(InpDataFile)))//go until the end of file is reached
    {
      VF48module* m = (VF48module*)malloc(sizeof(VF48module));
      int size = fread(m,sizeof(VF48module),1,InpDataFile);
      if((size != 1)&&(!(feof(InpDataFile)))){
        printf("File read error!\n");
        printf("Elements read: %i\n",size);
        exit(-1);
      }
      ShowVF48event(m);
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

