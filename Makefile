# Makefile

CFLAGS   =  -DOS_LINUX -g -O2 -Wall -Wuninitialized -I. -I$(MIDASSYS)/include -I$(MIDASSYS)/drivers/vme

ifdef ROOTSYS
all:: vf48_DisplayWaveforms
all:: vf48_ERaw
endif
all:: vf48_TakeData
all:: vf48_Status
all:: vf48_Reset
all:: vf48_Detect

ifdef ROOTSYS
ROOTLIBS   = -L$(ROOTSYS)/lib $(shell $(ROOTSYS)/bin/root-config --libs)
ROOTGLIBS  = -L$(ROOTSYS)/lib $(shell $(ROOTSYS)/bin/root-config --glibs)
ROOTCFLAGS = -L$(ROOTSYS)/lib $(shell $(ROOTSYS)/bin/root-config --cflags)
RPATH     += -Wl,-rpath,$(ROOTSYS)/lib
ROOTCXXFLAGS  += -DHAVE_ROOT $(ROOTCFLAGS)
ROOTLIBS      += $(ROOTGLIBS) $(RPATH)


vf48_DisplayWaveforms.o: vf48_DisplayWaveforms.cxx
	$(CXX) $(CFLAGS) -DHAVE_ROOT $(ROOTCFLAGS) -c $<
	
vf48_ERaw.o: vf48_ERaw.cxx
	$(CXX) $(CFLAGS) -DHAVE_ROOT $(ROOTCFLAGS) -c $<
endif

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cxx
	$(CXX) $(CFLAGS) -c $<

%.o: $(MIDASSYS)/drivers/vme/%.c
	$(CC) $(CFLAGS) -c -o $@ -O2 -g -Wall -Wuninitialized $<

vmicvme.o : $(MIDASSYS)/drivers/vme/vmic/vmicvme.c
	$(CC) $(CFLAGS) -c -o $@ -O2 -g -Wall -Wuninitialized $< 

vf48_TakeData: vf48_TakeData.o vf48.o vmicvme.o UnpackVF48A.o
	$(CXX) -o $@ -O2 -g -Wall $^ -lvme

ifdef ROOTSYS	
vf48_DisplayWaveforms: vf48_DisplayWaveforms.o UnpackVF48A.o
	$(CXX) -o $@ -O2 -g -Wall $^ $(ROOTGLIBS) -lvme

vf48_ERaw: vf48_ERaw.o UnpackVF48A.o
	$(CXX) -o $@ -O2 -g -Wall $^ $(ROOTGLIBS) -lvme
endif

vf48_Status: vf48_Status.o vf48.o vmicvme.o
	$(CC) -o $@ -O2 -g -Wall $^ -lvme

vf48_Reset: vf48_Reset.o vf48.o vmicvme.o
	$(CC) -o $@ -O2 -g -Wall $^ -lvme

vf48_Detect: vf48_Detect.o vf48.o vmicvme.o
	$(CC) -o $@ -O2 -g -Wall $^ -lvme

clean::
	-rm -f *.o

clean::
	-rm -f *.exe

# end
