FROM ubuntu:22.04

COPY entrypoint.sh /entrypoint.sh
COPY geth /geth
RUN chmod +x /entrypoint.sh /geth

EXPOSE 8545

ENTRYPOINT ["/entrypoint.sh"]
