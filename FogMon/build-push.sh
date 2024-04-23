#!/bin/bash

#DOCKER_CLI_EXPERIMENTAL=enabled docker buildx build -t diunipisocc/liscio-fogmon:test2 --platform linux/amd64,linux/arm64,linux/arm/v7 -f ubuntu.Dockerfile --push .

docker build --tag diunipisocc/liscio-fogmon:test2 .
#docker build --tag diunipisocc/liscio-fogmon:valgrind2 -f valgrind.Dockerfile .

sudo docker push diunipisocc/liscio-fogmon:test2
#sudo docker push diunipisocc/liscio-fogmon:valgrind2