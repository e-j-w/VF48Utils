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
int vf48samples;
int preTrig;

TApplication *app = NULL;
TCanvas* gWindow;
TH1D* hchan;

//function to print VF48 events
void ShowVF48event(const VF48module *m)
{
  
  //debug info
  printf("--------------------------\n");
  printf("Showing VF48 waveform from channel %i (%i samples total)\n",gPlotChan, vf48samples);
  printf("VF48 group trigger #s: %i %i %i %i %i %i\n",m->trigno[0],m->trigno[1],m->trigno[2],m->trigno[3],m->trigno[4],m->trigno[5]);
  printf("VF48 group timestamps: %f %f %f %f %f %f\n",m->timestamps[0],m->timestamps[1],m->timestamps[2],m->timestamps[3],m->timestamps[4],m->timestamps[5]);
  printf("charge: %i\n",m->channels[gPlotChan].charge);
  /*printf("Waveform samples: [");
  for (int i=0; i<m->channels[gPlotChan].numSamples; i++){
    printf(" %i ",m->channels[gPlotChan].samples[i]);
  }
  printf("]\n");*/

  if (!app)
    app = new TApplication("testTTC", NULL, NULL);

  if (!gWindow) {
    gWindow = new TCanvas("Window", "window", 1000, 1000);
    gWindow->Clear();
    //gWindow->Divide(2, 4);

    hchan = new TH1D("hchan", "chan", vf48samples, 0, vf48samples-1);
    hchan->Draw();
  }

  hchan->Reset();
  for (int i=0; i<vf48samples; i++)
    hchan->SetBinContent(i+1,  m->channels[gPlotChan].samples[i]);

  gWindow->Modified();
  gWindow->Update();

  printf("Showing ROOT window...\n");
  app->Run(kTRUE);

}

int main(int argc, char* argv[])
{
  FILE * InpDataFile;
  char   fileName[132];

  if(argc!=3){
    printf("./vf48_DisplayWaveforms input_data_file channel\n");
    printf("Shows waveforms in a ROOT window.\n");
    exit(0);
  }

  gPlotChan = atoi(argv[2]);

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
  size = fread(&vf48samples,sizeof(int),1,InpDataFile);
  if((size != 1)&&(!(feof(InpDataFile)))){
    printf("File read error!\n");
    printf("Elements read: %i\n",size);
    exit(-1);
  }
  printf("Pre trigger length: %i\nSamples per waveform: %i\n",preTrig,vf48samples);

  while(!(feof(InpDataFile)))//go until the end of file is reached
    {
      VF48module* m = (VF48module*)malloc(sizeof(VF48module));
      size = fread(m,sizeof(VF48module),1,InpDataFile);
      if((size != 1)&&(!(feof(InpDataFile)))){
        printf("File read error!\n");
        printf("Elements read: %i\n",size);
        exit(-1);
      }
      ShowVF48event(m);
      delete m;
    }

  //close the data file
  fclose(InpDataFile);

  return 0;
}

