package main

import (
	"errors"
	"fmt"
	"os"
	"os/signal"
	"runtime"
	"strings"
	"syscall"

	"github.com/oxodao/photobooth_hwhandler/config"
	"github.com/oxodao/photobooth_hwhandler/services"
	"go.bug.st/serial"
)

func start() error {
	err := config.Load()
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	err = services.Load()
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	ports, err := serial.GetPortsList()
	if err != nil {
		return err
	}

	if len(ports) == 0 {
		return errors.New("no arduino found")
	}

	for _, p := range ports {
		if !strings.HasPrefix(p, "/dev/ttyUSB") {
			continue
		}

		go services.GET.ProcessSerialConn(p)
	}

	return nil
}

func main() {
	if err := start(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	c := make(chan os.Signal)
	signal.Notify(c, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-c
		os.Exit(1)
	}()

	for {
		runtime.Gosched()
	}
}
