FROM node

WORKDIR /app

COPY . .

RUN npm --registry https://registry.npm.taobao.org install

EXPOSE 3000

# watch out environment variables
# export IP_ADDR=
# export WEB3_PROVIDER=
# export DATADIR=
# export WINFLAG=

CMD ["bin/www"]
