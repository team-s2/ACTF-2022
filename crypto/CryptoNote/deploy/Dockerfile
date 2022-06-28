# docker build -t aaa_crypto_note:latest .
# docker run -p 8080:8080 aaa_crypto_note:latest
FROM sagemath/sagemath:9.5

WORKDIR /home/sage

COPY src/* .
RUN chmod +x ./deploy.sh; sudo ./deploy.sh

EXPOSE 8080
CMD ["sudo", "./run_socat.sh"]
