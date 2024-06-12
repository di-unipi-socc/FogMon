#!/bin/bash

docker buildx build --platform linux/arm64,linux/amd64 -t diunipisocc/liscio-fogmon:latest --push . -f ubuntu.Dockerfile