WX_CFLAGS := $(shell wx-config --cflags)
WX_LDFLAGS := $(shell wx-config --libs core)

CV_CFLAGS := # $(shell pkg-config --cflags opencv)
CV_LDFLAGS := # $(shell pkg-config --libs opencv)

BOOST_LDFLAGS := -lboost_filesystem -lboost_system

GPHOTO2_LDFLAGS := -lgphoto2 -lgphoto2_port

CXXFLAGS := -g -std=c++0x -Wall -Wextra $(WX_CFLAGS) $(CV_CFLAGS)
LDFLAGS := -std=c++0x $(WX_LDFLAGS) $(CV_LDFLAGS) $(BOOST_LDFLAGS) $(GPHOTO2_LDFLAGS)

DEPSFLAGS := -MMD -MP

APPS := gphotogrid gpwraptest paraphotos

all: $(APPS)

gpwraptest: gpwrap.cpp gptest.cpp gputil.cpp gpwrap.h signalhandler.cpp
	clang++ -g -std=c++0x gpwrap.cpp gptest.cpp gputil.cpp signalhandler.cpp -lgphoto2 -lgphoto2_port -Wall -Wextra -o gpwraptest

paraphotos: paraphotos.o gpwrap.o gputil.o signalhandler.o
	$(CXX) $(LDFLAGS) $^ -o $@

gphotogrid: gphotogrid.o optionspanel.o imagepanel.o timeline.o previewfeed.o gfxui.o app.o gpwrap.o gputil.o
	$(CXX) $(LDFLAGS) $^ -o $@

timelinetest: timelinetest.o timeline.o
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(DEPSFLAGS) $<

-include *.d

clean:
	rm -f *.o *.d $(APPS)

.PHONY: all clean
