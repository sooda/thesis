WX_CFLAGS := $(shell wx-config --cflags)
WX_LDFLAGS := $(shell wx-config --libs core)

CV_CFLAGS := $(shell pkg-config --cflags opencv)
CV_LDFLAGS := $(shell pkg-config --libs opencv)

CXXFLAGS := -g $(WX_CFLAGS) $(CV_CFLAGS)

LDFLAGS := $(WX_LDFLAGS) $(CV_LDFLAGS)

APPS := pairbrowse

all: $(APPS)

pairbrowse: pairbrowse.o imagepanel.o
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $^

clean:
	rm -f *.o $(APPS)
