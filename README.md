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
The default used ports are:

5555/TCP fogmon

5201/TCP iperf

8366/TCP assolo

8365/UDP assolo

-----

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
