#!/usr/local/bin/sage-entrypoint /bin/bash

socat tcp-listen:8080,fork exec:./run_instance.sh,reuseaddr
