docker build -t server_c_src:1.0 .


docker run -d -p 8000:8000 server_c_src:1.0