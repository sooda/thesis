WX_CFLAGS := $(shell wx-config --cflags)
WX_LDFLAGS := $(shell wx-config --libs core)

CV_CFLAGS := # $(shell pkg-config --cflags opencv)
CV_LDFLAGS := # $(shell pkg-config --libs opencv)

BOOST_LDFLAGS := -lboost_filesystem -lboost_system

GPHOTO2_LDFLAGS := -lgphoto2 -lgphoto2_port

CXXFLAGS := -g -std=c++0x $(WX_CFLAGS) $(CV_CFLAGS)
LDFLAGS := -std=c++0x $(WX_LDFLAGS) $(CV_LDFLAGS) $(BOOST_LDFLAGS) $(GPHOTO2_LDFLAGS)

DEPSFLAGS := -MMD -MP

APPS := gphotogrid

all: $(APPS)

gpwraptest: gpwrap.cpp gptest.cpp gputil.cpp gpwrap.h
	clang++ -g -std=c++0x gpwrap.cpp gptest.cpp gputil.cpp -lgphoto2 -lgphoto2_port -Wall -Wextra -o gpwraptest

gphotogrid: gphotogrid.o optionspanel.o imagepanel.o app.o gpwrap.o gputil.o
	$(CXX) $(LDFLAGS) $^ -o $@

timelinetest: timelinetest.o timeline.o
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(DEPSFLAGS) $<

-include *.d

clean:
	rm -f *.o $(APPS)

.PHONY: all clean
