WX_CFLAGS := $(shell wx-config --cflags)
WX_LDFLAGS := $(shell wx-config --libs core)

# not using cv currently
CV_CFLAGS := # $(shell pkg-config --cflags opencv)
CV_LDFLAGS := # $(shell pkg-config --libs opencv)

# neither any filesystem things
BOOST_LDFLAGS := # -lboost_filesystem -lboost_system

GPHOTO2_LDFLAGS := -lgphoto2 -lgphoto2_port

CXXFLAGS := -O2 -g -std=c++11 -Wall -Wextra $(WX_CFLAGS) $(CV_CFLAGS)
LDFLAGS := -O2 -std=c++11 $(WX_LDFLAGS) $(CV_LDFLAGS) $(BOOST_LDFLAGS) $(GPHOTO2_LDFLAGS)

DEPSFLAGS := -MMD -MP

APPS := gpwraptest gphotogrid timelinetest paraphotos

SRC_GPWRAPTEST := gpwrap.cpp gptest.cpp gputil.cpp gpwrap.h signalhandler.cpp
SRC_PARAPHOTOS := paraphotos.cpp gpwrap.cpp gputil.cpp signalhandler.cpp
SRC_GPHOTOGRID := gphotogrid.cpp optionspanel.cpp imagepanel.cpp timeline.cpp previewfeed.cpp gfxui.cpp app.cpp gpwrap.cpp gputil.cpp
SRC_TIMELINETEST := timelinetest.cpp timeline.cpp
BUILDDIR = build

all: $(APPS)

gpwraptest: $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SRC_GPWRAPTEST))
	$(CXX) $(LDFLAGS) $^ -o $@

gphotogrid: $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SRC_GPHOTOGRID))
	$(CXX) $(LDFLAGS) $^ -o $@

timelinetest: $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SRC_TIMELINETEST))
	$(CXX) $(LDFLAGS) $^ -o $@

paraphotos: $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SRC_PARAPHOTOS))
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILDDIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(DEPSFLAGS) $< -o $@

-include *.d

clean:
	rm -f build/*.o build/*.d $(APPS)

.PHONY: all clean
