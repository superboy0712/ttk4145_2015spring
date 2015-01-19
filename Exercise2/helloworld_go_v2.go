// Go 1.2
// go run helloworld_go.go

package main

import (
    . "fmt"     
    "runtime"
    //"time"
)
var g_i int64 = 0;
func thread_1( c chan int) {
    for i := 1; i < 10011; i++{
		c <- 1
		g_i++
	}
	close(c)
}
func thread_2( c chan int) {
    for i := 1; i < 10000; i++{
		c <- 2
		g_i--
	}
	close(c)
}
func main() {
    runtime.GOMAXPROCS(runtime.NumCPU())    // I guess this is a hint to what GOMAXPROCS does...
	c1 := make(chan int)
	c2 := make(chan int)
	//timeout := time.After(3 * time.Second)
    	go thread_1(c1)                      // This spawns someGoroutine() as a goroutine
	go thread_2(c2)
    // We have no way to wait for the completion of a goroutine (without additional syncronization of some sort)
    // We'll come back to using channels in Exercise 2. For now: Sleep.
    ndone_1, ndone_2 := true, true
	for {
		
		
		select {
		case   c, n_1 := <-c1:
			println(c, n_1)
			ndone_1 = n_1
			
			
		case   c, n_2 :=<-c2:
			println(c, n_2)
			ndone_2 = n_2
	
		//case <- timeout:
		//	Println("Hello from timeout! The g_i: ", g_i)
		//	return 
		
		//default:
			
		}
		println("hey i am out of select :", ndone_1, ndone_2)
		if (ndone_1 == false) && (ndone_2 == false){
			println("great!, both channels are closed")
			break
		}
	}
	Println("Hello from main! The g_i: ", g_i)
    
}
