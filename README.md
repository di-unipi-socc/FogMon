# FTPiDiscoverer

```
git submodule init
git submodule update
```

install sqlite3 for development

install libstdc++ for static linking

install sigar:
https://github.com/hyperic/sigar

it needs to install libtool first

if not working with
```
./autogen.sh && ./configure && make && sudo make install
```
```
make clean
```
then use:
```
./autogen.sh && ./configure && make CFLAGS=-fgnu89-inline && sudo make install
```
