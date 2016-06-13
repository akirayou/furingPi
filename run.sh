#!/bin/sh
ulimit -c unlimited
sudo xauth add `xauth list|tail -n 1`
#sudo LD_LIBRARY_PATH=/home/pi/work/boost/lib/ ./cap
sudo  ./cap
sudo chown pi core
