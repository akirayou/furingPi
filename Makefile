CXXFLAGS=  -I ./rpi_ws281x-master  `pkg-config --cflags opencv` -O4 # -O0 -ggdb

#CXXFLAGS=  -I ./rpi_ws281x-master  `pkg-config --cflags opencv`  -O0 -ggdb
LDFLAGS=  -L ./rpi_ws281x-master  -lboost_thread -lboost_system -lboost_atomic `pkg-config --libs opencv` -lX11

all: cap

pre:
	wget https://github.com/jgarff/rpi_ws281x/archive/master.zip
	unzip master.zip
	(cd rpi_ws281x-master;scons)
	touch pre

led.o:pre

cap: wind.o cap.o led.o 
	$(CXX) $(LDFLAGS) -o cap wind.o cap.o led.o ./rpi_ws281x-master/libws2811.a

clean:
	rm *.o cap
