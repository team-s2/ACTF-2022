#!/bin/bash

if [ -z "$TOKEN_SECRET" ]; then
  TOKEN_SECRET=$(openssl rand -base64 32 | tr -d /=+)
  export TOKEN_SECRET
fi

source /xinetd.sh
