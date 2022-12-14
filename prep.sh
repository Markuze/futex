#!/bin/bash

DIR=/mnt/ramfs/
SIZE='32G'

sudo umount ${DIR}
sudo mkdir -p ${DIR}
sudo mount -t tmpfs -o size=${SIZE} randomname ${DIR}
sudo chown `whoami` ${DIR}
df -hk
