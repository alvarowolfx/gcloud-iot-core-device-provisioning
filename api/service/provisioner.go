package service

import (
	"errors"
	"fmt"
	"net/http"

	"google.golang.org/api/cloudiot/v1"
	"google.golang.org/api/googleapi"
)

type DeviceProvisioner interface {
	CreateDevice(id, publicKey string) error
	DeleteDevice(id string) error
}

var (
	ErrDeviceAlreadyExists = errors.New("Device Already Exists")
)

type IoTCoreDeviceProvisioner struct {
	ciotService *cloudiot.Service
	parent      string
}

func NewIoTCoreDeviceProvisioner(ciotService *cloudiot.Service, parent string) DeviceProvisioner {
	return &IoTCoreDeviceProvisioner{
		ciotService: ciotService,
		parent:      parent,
	}
}

func (p *IoTCoreDeviceProvisioner) CreateDevice(id, publicKey string) error {
	device := &cloudiot.Device{
		Id: id,
		Credentials: []*cloudiot.DeviceCredential{
			{
				PublicKey: &cloudiot.PublicKeyCredential{
					Format: "ES256_PEM",
					Key:    publicKey,
				},
			},
		},
	}

	req := p.ciotService.Projects.Locations.Registries.Devices.Create(p.parent, device)
	_, err := req.Do()

	if err != nil {
		gapiErr := err.(*googleapi.Error)
		alreadyExists := gapiErr.Code == http.StatusConflict
		if alreadyExists {
			fmt.Print(device.Id)
			fmt.Println(" already exists")
			return ErrDeviceAlreadyExists
		} else {
			fmt.Println("Failed to create device: %v", gapiErr.Message)
			return errors.New(gapiErr.Message)
		}
	}

	fmt.Print("Created ")
	fmt.Println(device.Id)

	return nil
}

func (p *IoTCoreDeviceProvisioner) DeleteDevice(id string) error {
	name := fmt.Sprintf("%s/devices/%s", p.parent, id)
	req := p.ciotService.Projects.Locations.Registries.Devices.Delete(name)
	_, err := req.Do()

	if err != nil {
		gapiErr := err.(*googleapi.Error)
		fmt.Println("Failed to delete device: %v", gapiErr.Message)
		return errors.New(gapiErr.Message)
	}

	fmt.Print("Deleted ")
	fmt.Println(name)

	return nil
}
