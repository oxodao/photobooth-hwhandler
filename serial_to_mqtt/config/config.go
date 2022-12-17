package config

import (
	"io/ioutil"
	"os"

	"go.bug.st/serial"
	"gopkg.in/yaml.v3"
)

var GET *Config = nil

type SerialConfig struct {
	BaudRate int `yaml:"baud_rate"`
}

type MosquittoConfig struct {
	Address string `json:"address"`
}

type Config struct {
	Mappings     map[string]string `yaml:"mappings"`
	Mosquitto    MosquittoConfig   `yaml:"mosquitto"`
	SerialConfig SerialConfig      `yaml:"serial_config"`

	SerialMode *serial.Mode `yaml:"-"`
}

func Load() error {
	cfg := Config{}

	configPath := os.Getenv("PHOTOBOOTH_HANDLER_CONFIG_PATH")
	if len(configPath) == 0 {
		configPath = "/etc/photobooth_hwhandler.yaml"
	}

	data, err := ioutil.ReadFile(configPath)
	if err != nil {
		return err
	}

	err = yaml.Unmarshal(data, &cfg)
	if err != nil {
		return err
	}

	cfg.SerialMode = &serial.Mode{
		BaudRate: cfg.SerialConfig.BaudRate,
		Parity:   serial.NoParity,
		DataBits: 8,
		StopBits: serial.OneStopBit,
	}

	GET = &cfg

	return nil
}
