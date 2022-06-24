#!/bin/bash

docker build -t mcu-builder:latest .
docker tag mcu-builder:latest tmesw.azurecr.io/mcu-builder:latest
docker push tmesw.azurecr.io/mcu-builder:latest

