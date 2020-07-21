package controllers

import (
	"context"
	"encoding/json"
	"fmt"

	"com.aviebrantz.iot-core-provisioner/model"
	"com.aviebrantz.iot-core-provisioner/service"
	"github.com/gofiber/fiber"
)

type DeviceController struct {
	deviceControlService service.DeviceControlService
	deviceProvisioner    service.DeviceProvisioner
	deviceRepository     model.DeviceRepository
	userRepository       model.UserRepository
}

func NewDeviceController(
	deviceControlService service.DeviceControlService,
	deviceProvisioner service.DeviceProvisioner,
	deviceRepository model.DeviceRepository,
	userRepository model.UserRepository) *DeviceController {
	return &DeviceController{
		deviceControlService: deviceControlService,
		deviceProvisioner:    deviceProvisioner,
		deviceRepository:     deviceRepository,
		userRepository:       userRepository,
	}
}

func (dc *DeviceController) CheckDeviceById(c *fiber.Ctx) {
	deviceID := c.Params("deviceId")
	ctx := context.Background()
	device, err := dc.deviceRepository.FindById(ctx, deviceID)

	if err != nil {
		c.Status(500).JSON(map[string]string{"message": "Internal error to fetch data"})
		return
	}

	if device == nil {
		c.Status(404).JSON(map[string]string{"message": "Device Not Found"})
		return
	}

	c.Status(200).JSON(device)
}

type CreateDeviceRequest struct {
	PublicKey string `json:"publicKey" form:"publicKey"`
}

func (dc *DeviceController) CreateDevice(c *fiber.Ctx) {
	deviceID := c.Params("deviceId")
	req := &CreateDeviceRequest{}
	if err := c.BodyParser(req); err != nil {
		c.Status(400).JSON(fiber.Map{"message": "Missing publicKey"})
		return
	}

	err := dc.deviceProvisioner.CreateDevice(deviceID, req.PublicKey)
	if err != nil {
		c.Status(400).JSON(fiber.Map{"message": err.Error()})
		return
	}

	c.Status(200).JSON(fiber.Map{
		"message":    fmt.Sprintf("%s created", deviceID),
		"projectId":  dc.deviceProvisioner.GetProjectId(),
		"registryId": dc.deviceProvisioner.GetRegistryId(),
		"region":     dc.deviceProvisioner.GetRegion(),
	})
}

type SendCommandRequest struct {
	Power      *bool `json:"power,omitempty" form:"power"`
	Brightness *int  `json:"brightness,omitempty" form:"brightness"`
}

func (dc *DeviceController) SendCommand(c *fiber.Ctx) {
	deviceID := c.Params("deviceId")
	req := &SendCommandRequest{}
	if err := c.BodyParser(req); err != nil {
		c.Status(400).JSON(fiber.Map{"message": "Missing power or brightness"})
		return
	}

	data, err := json.Marshal(req)
	if err != nil {
		c.Status(500).JSON(fiber.Map{"message": err.Error()})
		return
	}

	err = dc.deviceControlService.SendCommand(deviceID, string(data))
	if err != nil {
		c.Status(400).JSON(fiber.Map{"message": err.Error()})
		return
	}

	c.Status(200).JSON(fiber.Map{
		"message": fmt.Sprintf("cmd sent to %s", deviceID),
	})
}

func (dc *DeviceController) AddDeviceToUser(c *fiber.Ctx) {
	deviceID := c.Params("deviceId")
	userID := c.Locals("userID").(string)

	ctx := context.Background()
	err := dc.userRepository.AddDeviceToUser(ctx, userID, deviceID)
	if err != nil {
		c.Status(400).JSON(fiber.Map{"message": err.Error()})
		return
	}

	c.Status(200).JSON(fiber.Map{
		"message": fmt.Sprintf("%s added to %s", deviceID, userID),
	})
}

func (dc *DeviceController) DeleteDevice(c *fiber.Ctx) {
	deviceID := c.Params("deviceId")
	ctx := context.Background()
	var errors []string

	errDeleteDevice := dc.deviceProvisioner.DeleteDevice(deviceID)
	if errDeleteDevice != nil {
		errors = append(errors, "Error deleting device on IoT Core")
	}

	errDeleteFb := dc.deviceRepository.DeleteById(ctx, deviceID)
	if errDeleteFb != nil {
		errors = append(errors, "Error deleting device on Firebase")
	}

	errDeleteFromUsers := dc.userRepository.RemoveDeviceFromAllUsers(ctx, deviceID)
	if errDeleteFromUsers != nil {
		errors = append(errors, "Error deleting device from all users")
	}

	if len(errors) > 0 {
		c.Status(400).JSON(fiber.Map{"message": errors})
		return
	}

	c.Status(200).JSON(fiber.Map{
		"message": fmt.Sprintf("%s deleted", deviceID),
	})
}
