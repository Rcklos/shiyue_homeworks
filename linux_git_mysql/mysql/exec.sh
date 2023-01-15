#!/bin/bash

# 构建docker image
docker build . -t mysql:5.7sy

# 运行docker容器
docker run -idt --name mysql-shiyue mysql:5.7sy

# 进入容器
docker exec -it mysql-shiyue sh
