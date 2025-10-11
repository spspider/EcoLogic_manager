#!/bin/bash

docker build -t spspider/device_manager:1 .
# docker tag device_manager:1 spspider/device_manager:1
docker login
docker push spspider/device_manager:1
# docker system prune -a --volumes