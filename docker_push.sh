#!/bin/bash
docker build -f standalone_server/Dockerfile -t spspider/device_manager:1 .
# docker tag device_manager:1 spspider/device_manager:1
docker login
docker push spspider/device_manager:1
# docker system prune -a --volumes