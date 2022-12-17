package services

import (
	"errors"
	"fmt"
	"strings"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	"github.com/oxodao/photobooth_hwhandler/config"
	"go.bug.st/serial"
)

var GET *Provider

type Provider struct {
	MqttClient *mqtt.Client
}

func (p *Provider) SendButtonPress(buttonStr string) {
	buttonStr = strings.Trim(buttonStr, " \t")

	val, ok := config.GET.Mappings[buttonStr]
	if !ok {
		fmt.Println("Unknown button: " + buttonStr)
		return
	}

	(*p.MqttClient).Publish("photobooth/button_press", 2, false, val)
}

func (p *Provider) ReadLine(port serial.Port) (string, error) {
	str := ""

	buff := make([]byte, 100)
	for {
		n, err := port.Read(buff)
		if err != nil {
			return str, err
		}

		if n == 0 {
			fmt.Println("No clue what to do here")
			return "", errors.New("byte = 0")
		}

		str += string(buff[:n])
		if strings.HasSuffix(str, "\n") || strings.HasSuffix(str, "\r") {
			break
		}
	}

	str = strings.ReplaceAll(str, "\n", "")
	str = strings.ReplaceAll(str, "\r", "")

	return str, nil
}

// @TODO: Add a delay in the arduino code so that if the button is pressed again in the last x seconds it will not send again
func (p *Provider) ProcessSerialConn(device string) {
	port, err := serial.Open(device, config.GET.SerialMode)
	if err != nil {
		fmt.Println("Failed to open device: ")
		fmt.Println(err)
		return
	}

	for {
		line, err := p.ReadLine(port)
		if err != nil {
			fmt.Println("Failed to read: ", err)
			continue
		}

		p.ProcessSerialMessage(line)
	}

}

func (p *Provider) ProcessSerialMessage(msg string) {
	if strings.HasPrefix(msg, "BTN_") {
		p.SendButtonPress(msg)
		return
	}

	data := strings.Split(msg, " ")
	if len(data) == 0 {
		fmt.Println("Bad message received")
		return
	}

	switch data[0] {
	default:
		fmt.Println("Unhandled arduino message: ", data[0])
		fmt.Println(strings.Join(append([]string{"\targs: "}, data[1:]...), " "))
	}
}

func Load() error {
	prv := Provider{}

	opts := mqtt.NewClientOptions().AddBroker(config.GET.Mosquitto.Address).SetClientID("hwhandler").SetPingTimeout(10 * time.Second).SetKeepAlive(10 * time.Second)
	opts.SetAutoReconnect(true).SetMaxReconnectInterval(10 * time.Second)
	opts.SetConnectionLostHandler(func(c mqtt.Client, err error) {
		fmt.Printf("[MQTT] Connection lost: %s\n" + err.Error())
	})
	opts.SetReconnectingHandler(func(c mqtt.Client, options *mqtt.ClientOptions) {
		fmt.Println("[MQTT] Reconnecting...")
	})

	client := mqtt.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		return token.Error()
	}
	prv.MqttClient = &client

	GET = &prv

	return nil
}
