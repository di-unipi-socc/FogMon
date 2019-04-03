FROM fedora:26
RUN dnf install -y libserialport libserialport-devel gcc make iperf3
RUN dnf install -y libtool
RUN dnf install -y libtirpc
RUN dnf install -y sqlite
COPY sigar /sigar
WORKDIR /sigar
RUN ./autogen.sh && ./configure && make CFLAGS=-fgnu89-inline && make install
WORKDIR /
RUN dnf install -y valgrind
RUN dnf install -y sqlite-devel
COPY ./test/* /
CMD ["/FTPiDiscovererServer"]
