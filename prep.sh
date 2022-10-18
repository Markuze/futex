#!/bin/bash

DIR=/mnt/famfs/
SIZE='4G'

sudo mkdir -p ${DIR}
sudo mount -t rmafs -o size=${SIZE} ramfs ${DIR}
