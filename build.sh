#!/bin/bash

docker build --tag diunipisocc/liscio-fogmon:test .
#docker build --tag diunipisocc/liscio-fogmon:valgrind -f valgrind.Dockerfile .

sudo docker push diunipisocc/liscio-fogmon:test
#sudo docker push diunipisocc/liscio-fogmon:valgrind