printf "GET / HTTP/1.1\r\nHost: localhost:8000\r\n\r\n"
sleep 1
printf "GET HTTP/1.1\r\nHost: localhost:8000\r\n\r\n"
sleep 1
printf "FRT / HTTP/1.1\r\nHost: localhost:8000\r\n\r\n"
sleep 1
printf "GET / HTTP/1.2\r\nHost: localhost:8000\r\n\r\n"
sleep 1
printf "GET / HTTP/\r\nHost: localhost:8000\r\n\r\n"
sleep 1
printf "GET / 1.1\r\nHost: localhost:8000\r\n\r\n"
sleep 1
printf "GET /lol HTTP/1.1\r\nHost: localhost:8000\r\n\r\n"
sleep 1
printf "GET /lol/fkf HTTP/1.1\r\nHost: localhost:8000\r\n\r\n"
sleep 1
printf "GET / HTTP/1.1\r\nHost: localhost:8000\r\n\r\n"
sleep 1
printf "GET http://localhost:8000/lol HTTP/1.1\r\n\r\n"
sleep 1
printf "GET http://localhost:8000/lol HTTP/1.1\r\n\r\n"
sleep 1
printf "GET http://localhost:8000/lol HTTP/1.1\r\nHost: localhost:8000\r\n\r\n"
sleep 1
printf "GET localhost:8000/lol HTTP/1.1\r\n\r\n"
sleep 1
printf "GET https://localhost:8000/lol HTTP/1.1\r\n\r\n"
sleep 1
printf "GET / HTTP/1.1\r\nHost: localhost:8000\r\n\r\nGET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
sleep 1
