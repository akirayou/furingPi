#!/bin/sh
ulimit -c unlimited
sudo xauth add `xauth list|tail -n 1`
#sudo DEBUG=1 LD_LIBRARY_PATH=/home/pi/work/boost/lib/ ./cap
sudo DEBUG=1  ./cap
sudo chown pi core
