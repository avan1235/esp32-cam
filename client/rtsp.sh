#!/usr/bin/env bash

if [[ ( $@ == "--help") || ( $@ == "-h" ) || ( $# == "0" ) ]]
then
	echo "usage: $0 {camera_ip}:{server_port}"
	exit 0
fi

i=1;
j=$#;
while [[ $i -le $j ]]
do
  from_ip=`echo $1 | cut -d : -f 1`;
  to_port=`echo $1 | cut -d : -f 2`;
  echo "start stream from camera $from_ip to port $to_port";
  docker run -d --rm -it -v "$PWD"/rtsp-simple-server.yml:/rtsp-simple-server.yml -p ${to_port}:8554 aler9/rtsp-simple-server;
  ffmpeg \
    -f mjpeg \
    -use_wallclock_as_timestamps 1 \
    -i http://${from_ip}/stream \
    -f rtsp \
    -rtsp_transport tcp rtsp://0.0.0.0:${to_port}/stream \
    -an;
  i=$((i + 1));
  shift 1;
done

