FROM ubuntu:22.04 as builder

# libserialport-dev libcurl4-openssl-dev for fogmon compilation
# gcc g++ make cmake libtool automake autoconf apt-utils patch pkg-config for compilation
# lua5.3 lua5.3-dev libtirpc-dev for sigar compilation
# sqlite3 libsqlite3-dev uuid-dev liblapack-dev libfreetype6-dev libjpeg-turbo8-dev libgpg-error-dev libgpgme-dev for compilation
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libserialport-dev libcurl4-openssl-dev \
    gcc g++ make cmake libtool automake autoconf apt-utils patch pkg-config \
    lua5.3 lua5.3-dev libtirpc-dev \
    libsqlite3-dev uuid-dev liblapack-dev libfreetype6-dev libjpeg-turbo8-dev libgpg-error-dev libgpgme-dev \
    && rm -rf /var/lib/apt/lists/*

ADD ./FogMon/assolo /compile/assolo
WORKDIR /compile/assolo
RUN ./configure && make
RUN cp $(ls ./Bin/*/*) /

ADD ./FogMon/sigar /compile/sigar
WORKDIR /compile/sigar
ADD ./FogMon/patches/sigar.patch /compile/sigar/sigar.patch
ADD ./FogMon/patches/sigar2.patch /compile/sigar/sigar2.patch
RUN patch < sigar.patch src/os/linux/linux_sigar.c && patch < sigar2.patch src/Makefile.am
RUN ./autogen.sh && ./configure && make CFLAGS=-fgnu89-inline && make install 

ADD ./FogMon/ /compile
WORKDIR /compile
# RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends lcov valgrind && rm -rf /var/lib/apt/lists/*
# RUN cmake . -DCMAKE_BUILD_TYPE=Debug
RUN cmake . && make

RUN cp ./FogMon /
RUN cp ./libsqlitefunctions.so /
# WORKDIR /
# RUN rm -Rf /compile

# FROM debian:bookworm-slim as runner
FROM ubuntu:22.04 as runner

# iperf3 iputils-ping iproute2 for network tests
# sqlite3 for database access (maybe not needed)
# gcc python3 python3-pip python3-dev for python scripts and python dependencies installation
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libserialport-dev libcurl4-openssl-dev \
    iperf3 iputils-ping iproute2 \
    sqlite3 gcc g++ python3 python3-pip python3-dev \
    && rm -rf /var/lib/apt/lists/*

#python3-setuptools python3-venv python3-wheel libpq-dev
#RUN pip3 install wheel
RUN mkdir /scripts
ADD ./FogMon/scripts/requirements.txt /scripts
# --break-system-packages to install in debian image
RUN pip3 install -r scripts/requirements.txt --no-cache-dir

# copy lib.so of sigar
COPY --from=builder /usr/local/lib/libsigar.so /usr/local/lib/
COPY --from=builder /usr/local/lib/libsigar.so.0 /usr/local/lib/
# copy fogmon, sqlite functions and assolo
COPY --from=builder /FogMon /
COPY --from=builder /libsqlitefunctions.so /
COPY --from=builder /assolo* /

ADD ./FogMon/scripts /scripts
# if needed to simulate a network delay
ADD ./test/delay.py /delay.py

ENTRYPOINT ["/FogMon"]
CMD []
