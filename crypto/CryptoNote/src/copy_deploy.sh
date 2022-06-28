#!/bin/bash

rm -rf ../deploy
mkdir ../deploy
mkdir ../deploy/src

cp Dockerfile ../deploy
cp blockchain_service.py ../deploy/src
cp ring_signature.py ../deploy/src
cp secret.py ../deploy/src
cp deploy.sh ../deploy/src
cp run_instance.sh ../deploy/src
cp run_socat.sh ../deploy/src
