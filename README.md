<center>
<img src="https://github.com/di-unipi-socc/FogMon/blob/master/img/logofogmon.png?raw=true" alt="Home Screen" width="250" />
</center>

FogMon is described in the following paper:

> [Antonio Brogi](http://pages.di.unipi.it/brogi), [Stefano Forti](http://pages.di.unipi.it/forti), Marco Gaglianese <br>
> [**Measuring the Fog, Gently**](), <br>	
> 17th International Conference on Service-Oriented Computing (ICSOC), 2019. 

If you wish to reuse source code in this repo, please consider citing the above mentioned article.

## How To
```
git submodule init
git submodule update
```

There is a docker image for easily running the node.

```
docker build --tag fogmon .
```

Then run a leader with the command
```
docker run -it --net=host fogmon --leader
```
and the other nodes with
```
docker run -it --net=host fogmon --C ip_leader
```
All the other parameters are visible reading the main.cpp, they can regulate the timings of the program (time between tests, time between checks etc).

Some ports needs to be open for incoming connections, by default they are:

5555/TCP fogmon

5201/TCP iperf

8366/TCP assolo

8365/UDP assolo

## Example execution on 5 nodes
Let's call the 5 nodes A, B, C, D, E and their ip IP_A, IP_B,..., IP_E.

First run one of the nodes (A) as a default _Leader_
```
docker run -it --net=host fogmon --leader
```
then connect all the other nodes to A
```
docker run -it --net=host fogmon --C IP_A
```
Now after 5 times the parameter "--time-propagation" a _Leader_ selection happens, and 2 new leaders are selected. This because the number of _Leaders_ is too low to sustain 5 nodes.


## Compile outside docker

The tool can be compiled outside docker (is not advised) as follow:

```
cmake .
```
```
make
```
dependencies:

sqlite3 for development

libstdc++ for static linking

libserialport

sigar:

https://github.com/hyperic/sigar

it needs libtool for compiling

and if do not compile with
```
./autogen.sh && ./configure && make && sudo make install
```
use:
```
make clean
./autogen.sh && ./configure && make CFLAGS=-fgnu89-inline && sudo make install
```
