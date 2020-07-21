# Tests

## Siege

```bash
# 10 concurrent requests on any available URL
siege -c10 -f urls.txt
```

## sender

> Send packets of packetSize to port on localhost while there is content in stdin.

```bash
clang++ -std=c++11 sender.cpp -o sender
# Use sender withtout options to see usage
cat file | sender port
echo "Request" | sender port
```

## ``test.go``

> Set parameters in ``http.Request`` L31

```bash
go run test.go
```

## netcat

```
# Multiple requests in one buffer
bash test_nc.sh | netcat localhost 8000
```
