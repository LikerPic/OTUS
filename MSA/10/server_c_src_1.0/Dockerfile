FROM alpine:3.7


RUN apk update \
        && apk upgrade \
        && apk add --no-cache bash \
        bash-doc \
        bash-completion \
        && rm -rf /var/cache/apk/* \
        && /bin/bash

RUN apk add gcc
RUN apk add musl-dev
RUN mkdir /app

COPY server.c /app
COPY unp_my.h /app

WORKDIR /app/

RUN gcc -o server server.c

ENTRYPOINT ["./server"]
CMD ["8000"]
