#!/bin/bash

docker build --tag diunipisocc/liscio-fogmon:test2 .
#docker build --tag diunipisocc/liscio-fogmon:test2 -f ubuntu.Dockerfile .
#docker build --tag diunipisocc/liscio-fogmon:valgrind2 -f valgrind.Dockerfile .

#sudo docker push diunipisocc/liscio-fogmon:test2
#sudo docker push diunipisocc/liscio-fogmon:valgrind2