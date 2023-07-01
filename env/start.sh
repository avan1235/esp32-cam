#!/bin/bash

CONTAINER_NAME="espressif_idf_camera"
IMAGE_TAG="avan1235/espressif-idf-camera:sha-f2d84aef94cb5f00701332841b7cf1766811d791"

ssh-keygen -f "$HOME/.ssh/known_hosts" -R "[localhost]:2222"
ssh-keygen -f "$HOME/.ssh/known_hosts" -R "[127.0.0.1]:2222"

docker stop "${CONTAINER_NAME}"
docker rm "${CONTAINER_NAME}"
if [[ "$*" == *"--no-device"* ]]
then
    docker run -d --cap-add sys_ptrace -p 127.0.0.1:2222:22 --name "${CONTAINER_NAME}" "${IMAGE_TAG}"
else
    docker run -d --cap-add sys_ptrace -p 127.0.0.1:2222:22 --device=/dev/ttyUSB0 --name "${CONTAINER_NAME}" "${IMAGE_TAG}"
fi
