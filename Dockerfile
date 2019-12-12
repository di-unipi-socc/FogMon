FROM fedora:26
RUN dnf install -y libserialport libserialport-devel gcc make iperf3 gcc-c++ cmake
RUN dnf install -y libtool
RUN dnf install -y libtirpc
RUN dnf install -y sqlite sqlite-devel
RUN dnf install -y sigar sigar-devel
RUN dnf install -y iputils
RUN dnf install -y python3
RUN dnf install -y libuuid-devel
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
