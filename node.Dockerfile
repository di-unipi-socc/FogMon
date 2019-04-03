FROM fedora:26
RUN dnf install -y libserialport libserialport-devel gcc make iperf3
RUN dnf install -y libtool
RUN dnf install -y libtirpc
RUN dnf install -y sqlite sqlite-devel
RUN dnf install -y sigar sigar-devel
RUN dnf install -y iputils
COPY ./test/* /
ENTRYPOINT ["/FTPiDiscovererNode"]
CMD []