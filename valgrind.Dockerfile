FROM fedora:26
RUN dnf install -y libserialport libserialport-devel gcc make iperf3 gcc-c++ cmake libtool libtirpc sqlite sqlite-devel sigar sigar-devel iputils libuuid-devel redhat-rpm-config python3 python3-devel lapack-devel python3-scipy freetype-devel libjpeg-turbo-devel libcurl-devel
RUN dnf install -y lcov valgrind
ADD scripts /compile/scripts
WORKDIR /compile
RUN cat scripts/requirements.txt | xargs -n 1 -L 1 pip3 install
RUN cp -R ./scripts /

ADD assolo-0.9a /compile/assolo-0.9a
WORKDIR /compile/assolo-0.9a
RUN ./configure
RUN make
RUN cp $(ls ./Bin/*/*) /

ADD . /compile
WORKDIR /compile

RUN cmake . -DCMAKE_BUILD_TYPE=Debug
RUN make
RUN cp ./FogMon /
RUN cp ./libsqlitefunctions.so /
WORKDIR /

RUN rm -Rf /compile
ENTRYPOINT ["valgrind","--leak-check=full","--track-origins=yes","/FogMon"]
CMD []
