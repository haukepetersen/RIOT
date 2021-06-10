#!/bin/sh

PORT=22222
HOST=herbertpi
TARGET=~/services/deichbr

FILES="deichbr.sh kea.deichbr.conf deichbr.service deichbr_logger.py deichbr_logger.service"

for file in ${FILES}; do
    scp -P ${PORT} ${file} ${HOST}:${TARGET}/
done
