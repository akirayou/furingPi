CXXFLAGS=  -I ~/work/rpi_ws281x-master_new  `pkg-config --cflags opencv` -O4 # -O0 -ggdb

#CXXFLAGS=  -I ~/work/rpi_ws281x-master_new  `pkg-config --cflags opencv`  -O0 -ggdb
LDFLAGS=  -L ~/work/rpi_ws281x-master_new  -lboost_thread -lboost_system -lboost_atomic `pkg-config --libs opencv` -lX11

all: cap

cap: wind.o cap.o led.o
	$(CXX) $(LDFLAGS) -o cap wind.o cap.o led.o ~/work/rpi_ws281x-master_new/libws2811.a

clean:
	rm *.o cap
