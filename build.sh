#!/bin/bash

rm -fr build
rm -fr spim

mkdir spim && cd spim
svn co http://svn.code.sf.net/p/spimsimulator/code/spim/
svn co http://svn.code.sf.net/p/spimsimulator/code/CPU/
cd spim && make

export PATH=`pwd`:$PATH
which spim

cd ../../ && mkdir build && cd build/ && cmake .. && make cooltest
