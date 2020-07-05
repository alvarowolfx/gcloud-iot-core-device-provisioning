package service

import (
	"encoding/base64"
	"errors"
	"fmt"

	"google.golang.org/api/cloudiot/v1"
	"google.golang.org/api/googleapi"
)

type DeviceControlService interface {
	SendCommand(id, data string) error
}

var (
	ErrDeviceOffline = errors.New("Device is Offline or not ack'ing commands")
)

type IoTCoreDeviceControlService struct {
	ciotService *cloudiot.Service
	parent      string
}

func NewIoTCoreDeviceControlService(ciotService *cloudiot.Service, parent string) DeviceControlService {
	return &IoTCoreDeviceControlService{
		ciotService: ciotService,
		parent:      parent,
	}
}

func (p *IoTCoreDeviceControlService) SendCommand(id, data string) error {
	name := fmt.Sprintf("%s/devices/%s", p.parent, id)

	encodedData := base64.StdEncoding.EncodeToString([]byte(data))
	cmdReq := &cloudiot.SendCommandToDeviceRequest{
		BinaryData: encodedData,
	}

	req := p.ciotService.Projects.Locations.Registries.Devices.SendCommandToDevice(name, cmdReq)
	_, err := req.Do()

	if err != nil {
		gapiErr := err.(*googleapi.Error)
		if len(gapiErr.Errors) > 0 {
			for _, e := range gapiErr.Errors {
				if e.Reason == "failedPrecondition" {
					return ErrDeviceOffline
				}
			}
		}
		fmt.Println("Failed to send cmd to device: %v", gapiErr.Message)
		return errors.New(gapiErr.Message)
	}

	fmt.Print("Sent cmd to ")
	fmt.Println(id)

	return nil
}
