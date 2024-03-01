# Mini-HTTP-Proxy
A mini http/https caching proxy server

## Features
1. user should be able to configure their browser to use your proxy, and browse typical webpages (e.g., perform a Google Search, view the results, etc).
2. proxy must cache responses of 200-OK responses to GET requests from the server, and should follow the rules of expiration time and/or revalidation in determining if proxy can server a request from its local cache
3. proxy must handle multiple concurrent requests effectively and should use multiple threads as part of your strategy to do so.
4. proxy will generate log in (/var/log/erss/proxy.log) which contains information of each request.

## Supported HTTP function
1. GET
2. POST
3. CONNECT

## How to run?
go to `src` and make. After that, simply run
```
./main
```

## Docker support
The structure of the current project is:
Mini-HTTP-Proxy
   |—docker-compose.yml
   |—src
       |—Dockerfile
       |—main.c
       |-other source files ...

To run the project, simply run
```
sudo docker-compose up
```
or
```
sudo docker compose up
```
if the docker compose version is larger than 3
