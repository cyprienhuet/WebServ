package main

import (
	"fmt"
	"io"

	// "io/ioutil"
	"bufio"
	"net/url"

	// "github.com/benburkert/http"
	"net/http"
	// "net/http/httputil"
	//"encoding/binary"
	"os"
)

const (
	chunk1 = "First Chunk"
	chunk2 = "Second Chunk"
)

func main() {
	rd, wr := io.Pipe()

	//u, _ := url.Parse("http://httpbin.org/post?show_env=1")
	//u, _ := url.Parse("http://requestb.in/zox5gczo")
	u, _ := url.Parse("http://localhost:8000/")

	req := &http.Request{
		Method:           "POST",
		ProtoMajor:       1,
		ProtoMinor:       1,
		URL:              u,
		TransferEncoding: []string{"chunked"},
		Body:             rd,
		Header:           make(map[string][]string),
	}
	req.Header.Set("Content-Type", "text/plain")

	client := http.DefaultClient

	go func() {
		buf := make([]byte, 300)
		f, _ := os.Open("./www/")
		for {
			n, _ := f.Read(buf)
			if 0 == n {
				break
			}
			wr.Write(buf)
		}
		wr.Close()

	}()

	resp, err := client.Do(req)
	if nil != err {
		fmt.Println("error =>", err.Error())
		return
	}

	// defer resp.Body.Close()

	reader := bufio.NewReader(resp.Body)
	for {
		line, err := reader.ReadByte()
		if err != nil {
			return
		}
		fmt.Print(string(line))
	}
	// io.Copy(os.Stdout, resp.Body)
	// ioutil.
	// resp.Body.()
	// resp.Body.Close()
	// body, err := ioutil.ReadAll(resp.Body)
	// if nil != err {
	// 	fmt.Println("error =>", err.Error())
	// } else {
	// 	fmt.Println(string(body))
	// }
}
