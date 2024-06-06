docker build -t server_c:1.0 .

docker run -d -p 8000:8000 server_c:1.0
