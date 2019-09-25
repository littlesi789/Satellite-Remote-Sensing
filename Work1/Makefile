CXX = g++
CXXFLAGS = -g -std=c++11 -pedantic -Wno-unused-result
LDFLAGS = -lgdal -lopencv_core -lopencv_imgproc -lopencv_highgui  -lopencv_imgcodecs

all: ndvi ndwi

ndvi: ndvi.cpp
	$(CXX) $(CXXFLAGS) ndvi.cpp -o ndvi $(LDFLAGS)

ndwi: ndwi.cpp
	$(CXX) $(CXXFLAGS) ndwi.cpp -o ndwi $(LDFLAGS)

ndvi_pmg: ndvi.cpp
	$(CXX) $(CXXFLAGS) ndvi_pmg.cpp -o ndvi -lgdal

clean:
	rm -f ndvi ndwi
