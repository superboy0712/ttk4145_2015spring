// Go 1.2
// go run helloworld_go.go
// The ownership fashion
// pay attention, the blocking condition differs between chan and chan buffer. say make(chan int) different from make(chan int, 1)
package main

import (
    . "fmt"     
    "runtime"
    //"time"
)
func thread_1( c chan int64, done chan bool) {
    for i := 1; i < 100011; i++{
		g:= <- c
		g++
		println("1",g)
		c <- g
		 
	}
	println("done 1")
	done <- true
}
func thread_2( c chan int64, done chan bool) {
	for i := 1; i < 100000; i++{
		g:= <- c
		g--
		println("2",g)
		c <- g
	}
	println("done 2")
	done <- true
}

func main() {
    runtime.GOMAXPROCS(runtime.NumCPU())    // I guess this is a hint to what GOMAXPROCS does...
	c := make(chan int64, 1)
	done :=  make(chan bool, 2)	
	//timeout := time.After(3 * time.Second)
    go thread_1(c, done)                      // This spawns someGoroutine() as a goroutine
	go thread_2(c, done)
	
	c <- 0 // initial value of global channel
	// block to wait done channel write
	<- done
    <- done
	
	Println("Hello from main! The g_i: ", <- c)
    
}
