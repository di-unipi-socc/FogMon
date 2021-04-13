FROM ubuntu:focal
RUN apt-get update -y

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends libserialport-dev gcc g++ make iperf3 iputils-ping cmake libtool sqlite3 libsqlite3-dev uuid-dev python3 python3-pip python3-dev liblapack-dev python3-scipy swig libfreetype6-dev libjpeg-turbo8-dev libcurl4-openssl-dev libgpg-error-dev libgpgme-dev automake autoconf
#python3-setuptools python3-venv python3-wheel libpq-dev
#RUN pip3 install wheel
ADD scripts /compile/scripts
WORKDIR /compile
RUN cat scripts/requirements.txt | xargs -n 1 -L 1 pip3 install
RUN cp -R ./scripts /

ADD assolo-0.9a /compile/assolo-0.9a
WORKDIR /compile/assolo-0.9a
RUN ./configure --build=unknown-unknown-linux
RUN make
RUN cp $(ls ./Bin/*/*) /

ADD sigar /compile/sigar
WORKDIR /compile/sigar
COPY sigar.patch /compile/sigar/sigar.patch
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends patch
RUN patch < sigar.patch src/os/linux/linux_sigar.c
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y lua5.3 lua5.3-dev
RUN ./autogen.sh && ./configure && make CFLAGS=-fgnu89-inline && make install

ADD . /compile
WORKDIR /compile
#RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends lcov valgrind
#RUN cmake . -DCMAKE_BUILD_TYPE=Debug
RUN cmake .
RUN make
RUN cp ./FogMon /
RUN cp ./libsqlitefunctions.so /
WORKDIR /

RUN rm -Rf /compile

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends iproute2

#cleanup
RUN apt-get clean
RUN rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/FogMon"]
CMD []
