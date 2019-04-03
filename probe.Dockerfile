FROM fedora
COPY ./test/* /
RUN dnf install -y libserialport libserialport-devel gcc make iperf3
COPY sigar /sigar
WORKDIR /sigar
RUN make clean
RUN ./autogen.sh && ./configure && make CFLAGS=-fgnu89-inline && sudo make install
WORKDIR /
CMD ["/FTPiDiscovererNode"]
