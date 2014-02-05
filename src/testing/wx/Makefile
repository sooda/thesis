WX_CFLAGS := $(shell wx-config --cflags)
WX_LDFLAGS := $(shell wx-config --libs core)

CV_CFLAGS := $(shell pkg-config --cflags opencv)
CV_LDFLAGS := $(shell pkg-config --libs opencv)

BOOST_LDFLAGS := -lboost_filesystem -lboost_system

CXXFLAGS := -g -std=c++0x $(WX_CFLAGS) $(CV_CFLAGS)
LDFLAGS := -std=c++0x $(WX_LDFLAGS) $(CV_LDFLAGS) $(BOOST_LDFLAGS)

APPS := pairbrowse

all: $(APPS)

pairbrowse: pairbrowse.o imagepanel.o
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $^

clean:
	rm -f *.o $(APPS)
