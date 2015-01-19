// Go 1.2
// go run helloworld_go.go

package main

import (
    . "fmt"     
    "runtime"
    "time"
)
var g_i int64 = 0;
func thread_1( c chan int) {
    for i := 1; i < 10011; i++{
		c <- 1
		g_i++
	}
	//close(c)
}
func thread_2( c chan int) {
    for i := 1; i < 10000; i++{
		c <- 2
		g_i--
	}
	//close(c)
}
func main() {
    runtime.GOMAXPROCS(runtime.NumCPU())    // I guess this is a hint to what GOMAXPROCS does...
	c1 := make(chan int)
	c2 := make(chan int)
	timeout := time.After(1 * time.Second)
    	go thread_1(c1)                      // This spawns someGoroutine() as a goroutine
	go thread_2(c2)
    // We have no way to wait for the completion of a goroutine (without additional syncronization of some sort)
    // We'll come back to using channels in Exercise 2. For now: Sleep.
    for {
		select {
		case   c:= <-c1:
			println(c)
			
			
		case   c:= <-c2:
			println(c)
			
	
		case <- timeout:
			Println("Hello from main! The g_i: ", g_i)
			return 
		}
	}
	
    
}
