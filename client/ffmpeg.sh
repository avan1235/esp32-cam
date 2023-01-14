#!/usr/bin/env bash

docker stop "$( docker ps -aq )"

docker run -d --rm -it -v "$PWD"/rtsp-simple-server.yml:/rtsp-simple-server.yml -p 8554:8554 aler9/rtsp-simple-server
ffmpeg \
  -f mjpeg \
  -use_wallclock_as_timestamps 1 \
  -i http://192.168.0.170/stream \
  -f rtsp \
  -rtsp_transport tcp rtsp://0.0.0.0:8554/stream \
  -an
