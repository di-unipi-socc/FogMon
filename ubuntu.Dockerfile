FROM ubuntu:latest

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libserialport-dev \
    gcc g++ make cmake libtool automake autoconf apt-utils patch pkg-config \
    iperf3 iputils-ping iproute2 lua5.3 lua5.3-dev libtirpc-dev \
    sqlite3 libsqlite3-dev uuid-dev python3 python3-pip python3-dev liblapack-dev python3-scipy swig libfreetype6-dev libjpeg-turbo8-dev libcurl4-openssl-dev libgpg-error-dev libgpgme-dev \
    && rm -rf /var/lib/apt/lists/*

#python3-setuptools python3-venv python3-wheel libpq-dev
#RUN pip3 install wheel
ADD scripts /compile/scripts
WORKDIR /compile
RUN cat scripts/requirements.txt | xargs -n 1 -L 1 pip3 install
RUN cp -R ./scripts /

ADD assolo /compile/assolo
WORKDIR /compile/assolo
RUN ./configure
RUN make
RUN cp $(ls ./Bin/*/*) /

ADD sigar /compile/sigar
WORKDIR /compile/sigar
COPY sigar.patch /compile/sigar/sigar.patch
COPY sigar2.patch /compile/sigar/sigar2.patch
RUN patch < sigar.patch src/os/linux/linux_sigar.c
RUN patch < sigar2.patch src/Makefile.am
RUN ./autogen.sh && ./configure && make CFLAGS=-fgnu89-inline && make install 

# RUN cmake . && make && make install

ADD . /compile
WORKDIR /compile
#RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends lcov valgrind && rm -rf /var/lib/apt/lists/*
#RUN cmake . -DCMAKE_BUILD_TYPE=Debug
RUN cmake .
RUN make
RUN cp ./FogMon /
RUN cp ./libsqlitefunctions.so /
WORKDIR /

RUN rm -Rf /compile

#cleanup
RUN apt-get clean
RUN rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/FogMon"]
CMD []
