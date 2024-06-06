FROM alpine:3.7


RUN mkdir /app
COPY server /app

WORKDIR /app/

ENTRYPOINT ["./server"]
CMD ["8000"]
