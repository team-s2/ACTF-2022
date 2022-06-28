#!/usr/local/bin/sage-entrypoint /bin/bash

pip3 config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install pycryptodome

echo deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ impish main restricted universe multiverse > /etc/apt/sources.list

apt update
apt install -y socat

chmod +x /home/sage/run_socat.sh /home/sage/run_instance.sh
