# FogMon

```
git submodule init
git submodule update
```

Two docker image representing the two nodes.

```
docker build --tag node . -f node.Dockerfile
```

```
docker build --tag server . -f server.Dockerfile
```

The tool can be compiled outside docker (is not advised) as follow:

install sqlite3 for development

install libstdc++ for static linking

install libserialport

install sigar:
https://github.com/hyperic/sigar

it needs to install libtool first

if not working with
```
./autogen.sh && ./configure && make && sudo make install
```
use:
```
make clean
./autogen.sh && ./configure && make CFLAGS=-fgnu89-inline && sudo make install
```
