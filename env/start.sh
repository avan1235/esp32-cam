#!/bin/bash

ssh-keygen -f "/home/avan1235/.ssh/known_hosts" -R "[localhost]:2222"
docker build --build-arg UID="$(id -u)" -t avan1235/espressif-idf-camera:0.1 -f Dockerfile .
docker stop espressif_idf_camera
docker rm espressif_idf_camera
docker run -d --cap-add sys_ptrace -p 127.0.0.1:2222:22 --device=/dev/ttyUSB0 --name espressif_idf_camera avan1235/espressif-idf-camera:0.1
