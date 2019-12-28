FROM fedora:26
RUN dnf install -y libserialport libserialport-devel gcc make iperf3 gcc-c++ cmake libtool libtirpc sqlite sqlite-devel sigar sigar-devel iputils libuuid-devel redhat-rpm-config python3 python3-devel lapack-devel python3-scipy freetype-devel libjpeg-turbo-devel
RUN python3 -m pip install cython
RUN python3 -m pip install pyclustering

ADD . /compile
WORKDIR /compile
RUN cmake .
RUN make
RUN ls
RUN cp ./FogMon /
RUN cp ./libsqlitefunctions.so /
RUN cp -R ./scripts /
RUN ls /
WORKDIR /compile/assolo-0.9a
RUN ./configure
RUN make
RUN ls ./Bin/*/*
RUN cp $(ls ./Bin/*/*) /
WORKDIR /

RUN rm -Rf /compile
ENTRYPOINT ["/FogMon"]
CMD []
