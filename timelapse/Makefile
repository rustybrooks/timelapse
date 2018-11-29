SUFFIXES += .d

CFLAGS=-Wall 

# if using clang, which doesn't quite work right now
#CFLAGS += -Wno-c++98-compat -Wno-c++11-extensions -stdlib=libc++
#CC=g++
CC=clang++

#-fdump-class-hierarchy 
LDFLAGS=

SOURCES = 
ALL_SOURCES = timelapse.cpp dslr_capture.cpp 
ALL_SOURCES += $SOURCES

#CFLAGS += -I/usr/local/include -I /usr/X11/include  
#BOTHFLAGS += -I/Users/rbrooks/boost_1_56/include
BOTHFLAGS += -I../../lib/canon/EDSDK_Mac/EDSDK/Header/ 
BOTHFLAGS += -I/System/Library/Frameworks/Foundation.framework/Versions/C/Headers/
BOTHFLAGS += -I/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7/

BOTHFLAGS += -D__MACOS__=1

LDFLAGS += -F/Users/rbrooks/lib/canon/EDSDK_Mac/EDSDK_64/

CPPFLAGS += $(BOTHFLAGS)
CFLAGS += $(BOTHFLAGS)


LIBS = -L/usr/local/lib 
LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_video 
LIBS += -l opencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_nonfree -lopencv_contrib  
LIBS += -lboost_filesystem -lboost_system -lboost_program_options -lexiv2
LIBS += -lsqlite3


EDSDK_LIBS =

LINK=$(CC)

CFLAGS += -g
#CFLAGS += -O
#CFLAGS += -O2
CFLAGS += -O3

ALL_OBJECTS=$(ALL_SOURCES:.cpp=.o)	
OBJECTS=$(SOURCES:.cpp=.o)	



#These are the dependency files, which make will clean up after it creates them
DEPFILES=$(ALL_SOURCES:.cpp=.d)

all: timelapse dslr_capture dslr.so

clean:
	rm -f *.o *.obj *.d dslr.so timelapse dslr_capture

-include $(DEPFILES)

timelapse: timelapse.o GraphUtils.o
	$(LINK) $(LDFLAGS) $(LIBS) $(OBJECTS) timelapse.o GraphUtils.o -o $@

dslr_capture: dslr_capture.o callbacks.o GraphUtils.o
	$(LINK) $(LDFLAGS) $(LIBS) $(OBJECTS) \
	-framework EDSDK -framework Cocoa \
	dslr_capture.o callbacks.o GraphUtils.o -o $@

#	-dylib 
dslr.so: python.o
	$(LINK) -dynamiclib \
	$(LDFLAGS) $(LIBS) $(OBJECTS) \
	-framework EDSDK -framework Cocoa \
	-lboost_python \
	-F/System/Library/Frameworks/ -framework Python \
	python.o -o $@


#This is the rule for creating the dependency files
%.d: %.cpp
	$(CC) $(CFLAGS) -MM -MT '$(patsubst src/%,obj/%,$(patsubst %.cpp,%.o,$<))' $< > $@

# %.d
%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $< -o $@

