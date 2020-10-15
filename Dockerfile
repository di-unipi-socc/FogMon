FROM fedora:26
RUN dnf install -y libserialport libserialport-devel gcc make iperf3 gcc-c++ cmake libtool libtirpc sqlite sqlite-devel sigar sigar-devel iputils libuuid-devel redhat-rpm-config python3 python3-devel lapack-devel python3-scipy freetype-devel libjpeg-turbo-devel

ADD . /compile
WORKDIR /compile
RUN cp -R ./scripts /
RUN cat scripts/requirements.txt | xargs -n 1 -L 1 pip3 install
RUN cmake .
RUN make
RUN cp ./FogMon /
RUN cp ./libsqlitefunctions.so /
WORKDIR /compile/assolo-0.9a
RUN ./configure
RUN make
RUN cp $(ls ./Bin/*/*) /
WORKDIR /

RUN rm -Rf /compile
ENTRYPOINT ["/FogMon"]
CMD []
