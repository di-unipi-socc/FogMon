<center>
<img src="https://github.com/di-unipi-socc/FogMon/blob/liscio-2.0/img/logofogmon.png?raw=true" alt="Home Screen" width="250" />
</center>

*A distributed monitoring tool for Cloud-IoT and Fog computing infrastructures*

[FogMon 2.0](https://github.com/di-unipi-socc/FogMon/tree/liscio-2.0) has been released as a result of the experimental activities of the project "*Lightweight Self-adaptive Cloud-IoT Monitoring across Fed4FIRE+ Testbeds (LiSCIo)*" funded within the [*8th Fed4FIRE+ Competitive Call – Innovative Experiments | Category “Medium Experiments”*](https://www.fed4fire.eu/news/discover-the-8th-open-call-winners/). 

FogMon 2.0 was assessed and tuned in lifelike experimental settings on Cloud-IoT resources across the [Virtual Wall](https://www.fed4fire.eu/testbeds/virtual-wall/) and [CityLab](https://www.fed4fire.eu/testbeds/citylab/) testbeds, featuring 20, 30 and 40 monitored nodes.

FogMon 1.x and the overall functioning of the tool have been fully described in the following articles:

> [Stefano Forti](http://pages.di.unipi.it/forti), Marco Gaglianese, [Antonio Brogi](http://pages.di.unipi.it/brogi) <br>
> [**Lightweight self-organising distributed monitoring of Fog infrastructures**](https://doi.org/10.1016/j.future.2020.08.011), <br>	
> Future Generation Computer Systems (2020), DOI: 10.1016/j.future.2020.08.011. 

> [Antonio Brogi](http://pages.di.unipi.it/brogi), [Stefano Forti](http://pages.di.unipi.it/forti), Marco Gaglianese <br>
> [**Measuring the Fog, Gently**](https://doi.org/10.1007/978-3-030-33702-5_40), <br>	
> 17th International Conference on Service-Oriented Computing (ICSOC), 2019. 

Experimental data collected throughout LiSCIo is publicly available at [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.4682987.svg)](https://doi.org/10.5281/zenodo.4682987).

If you wish to reuse source code in this repo, please consider citing the articles above.

## New in FogMon 2.0

Overall, the new version of FogMon substantially improves and tune the previous one. The experiments carried out in LiSCIo definitely improved on the technology readiness level (TRL) of FogMon, bringing it from TRL 4 (validation in a laboratory environment, over only 13 nodes) to TRL 5 (validation in a relevant environment, with up to 40 nodes).

Particularly, FogMon 2.0 includes the following improvements with respect to previous releases:

1.	**Multi-threaded implementation of latency tests.** In FogMon 2.0 all latency measurements from a Follower to the others in the same group are performed in parallel, by relying on different threads. In the previous version tests were performed sequentially. This improvement speeds-up the overall time to complete all measurements to be done.
2.	**Parallel implementation of latency and bandwidth tests.** A dedicated thread is used to sequentially perform bandwidth measurements from a Follower to the others in the same group. In the previous version bandwidth tests were performed only after completing all latency tests. This improvement speeds-up the overall time to complete all measurements to be done.
3.	**Improved handling of bandwidth degradation.** In case of a considerable bandwidth degradation, the previous version of FogMon was slow in identifying it as it was still averaging the whole measurements window. FogMon 2.0 overcomes this issue by emptying the window in case the degradation exceeds a certain set threshold (30 %) with respect to the current average value. 
4.	**Improved handling of latency degradation.** In case of a considerable latency degradation (which can correspond to a link failure), the previous version of FogMon was slow in identifying it as it was still averaging the whole measurements window. FogMon 2.0 overcomes this issue by emptying the window in case the degradation exceeds a certain set threshold (500%) with respect to the current average value. Besides, as a consistent latency degradation can be accompanied by a similar change in bandwidth, the affected link is given priority for a new bandwidth measurement. 
5.	**New more accurate estimate of bandwidth inter-groups.** The previous formula to estimate the bandwidth between two nodes, A and B, from different groups was refined so to also consider the bandwidth between the two Leaders of A and B. This improvement makes the estimate way more accurate as it accounts for the fact that the bandwidth between Leaders could act as a bottleneck between two nodes in different groups. This is essential when a group features good bandwidth between its members (that are close to each other) but lower bandwidth towards other groups.
6.	**Improved passive bandwidth measurements with Assolo.** FogMon interleaves active bandwidth measurements with iperf to passive (lightweight) measurements with Assolo. As we noted that Assolo is accurate only within a certain bandwidth range (i.e. from 1 to 55 Mbps), we restricted passive measurements only to links within such range. On the other hand, for values outside this range, we always employ iperf. This improvement makes overall bandwidth measurements more accurate and faster as it avoids Assolo measurements when they are known to lead to inaccurate results. 
7.	**FogMonEye GUI** FogMon 2.0 has been extended with suitable tooling and a GUI, FogMonEye, which will allow us to assess new future releases of our tool.


## Quickstart
```
git submodule init
git submodule update
```

There is a Docker image for easily running the node.

```
docker build --tag fogmon .
```

Then run a _Leader_ with the command
```
docker run -it --net=host fogmon --leader
```
and the other nodes with
```
docker run -it --net=host fogmon -C ip_leader
```
All the other parameters are visible reading the main.cpp, they can set up all FogMon parameters (time between tests, time between checks etc).

Some ports needs to be open for incoming connections, by default they are:

5555/TCP fogmon

5201/TCP iperf

8366/TCP assolo

8365/UDP assolo

## Example execution on 5 nodes
Let's call the 5 nodes A, B, C, D, E and their ip IP_A, IP_B,..., IP_E.

First run one of the nodes (A) as a default _Leader_
```
docker run -it --net=host fogmon --leader
```
then connect all the other nodes to A
```
docker run -it --net=host fogmon -C IP_A
```
After 5 rounds the parameter "--time-propagation" a _Leader_ selection happens, and 2 new leaders are selected. This is because the number of _Leaders_ is too low to sustain 5 nodes.


## Compile outside docker

The tool can be compiled outside Docker (not advised) as follows:

```
cmake .
```
```
make
```
dependencies:

sqlite3 for development

libstdc++ for static linking

libserialport

sigar:

https://github.com/hyperic/sigar

it needs libtool for compiling

and if it does not compile then try with:
```
./autogen.sh && ./configure && make && sudo make install
```
and
```
make clean
./autogen.sh && ./configure && make CFLAGS=-fgnu89-inline && sudo make install
```
