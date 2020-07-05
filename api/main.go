package main

import (
	"log"
	"os"

	controllers "com.aviebrantz.iot-core-provisioner/controller"
	"com.aviebrantz.iot-core-provisioner/infrastructure"
	"com.aviebrantz.iot-core-provisioner/logger"
	"com.aviebrantz.iot-core-provisioner/model"
	"com.aviebrantz.iot-core-provisioner/service"
	"github.com/gofiber/cors"
	"github.com/gofiber/fiber"
	"github.com/gofiber/recover"
	"github.com/joho/godotenv"
)

func main() {
	err := godotenv.Load()
	if err != nil {
		log.Println("Error loading .env file")
	}

	port := "8000"
	if envPort := os.Getenv("PORT"); envPort != "" {
		port = envPort
	}

	log.Println(port)

	firebaseDB := infrastructure.FirebaseConnect()
	ciotService, parent := infrastructure.IoTCoreConnect()
	provisioner := service.NewIoTCoreDeviceProvisioner(ciotService, parent)
	deviceControlService := service.NewIoTCoreDeviceControlService(ciotService, parent)
	deviceRepository := model.NewFirebaseDeviceRepository(firebaseDB)
	userRepository := model.NewFirebaseUserRepository(firebaseDB)

	deviceController := controllers.NewDeviceController(deviceControlService, provisioner, deviceRepository, userRepository)

	app := fiber.New()

	app.Use(cors.New())
	app.Use(logger.NewZapLogger())
	cfg := recover.Config{
		Handler: func(c *fiber.Ctx, err error) {
			c.Status(500).JSON(fiber.Map{"message": err.Error()})
		},
	}
	app.Use(recover.New(cfg))

	app.Get("/", func(c *fiber.Ctx) {
		c.JSON(fiber.Map{
			"healthCheck": "ok",
		})
	})

	app.Get("/api/devices/:deviceId", deviceController.CheckDeviceById)
	app.Post("/api/devices/:deviceId", deviceController.CreateDevice)
	app.Delete("/api/devices/:deviceId", deviceController.DeleteDevice)
	app.Post("/api/devices/:deviceId/command", deviceController.SendCommand)
	app.Post("/api/users/:userId/devices/:deviceId", deviceController.AddDeviceToUser)
	//app.Get("/api/devices", heroController.GetHeroesRecommendations)

	err = app.Listen(port)
	if err != nil {
		log.Fatalf("error initializing app: %v\n", err)
	}
	/*
			publicKey := `-----BEGIN PUBLIC KEY-----
		MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE8XLBISPU19FFENarEzHjIrS8xz6c
		xsYlIz3GMhQfqRHvT1zJ0nsMK7x3iXnzPORh6wzcax/cb1ZNgvgYOnuz9w==
		-----END PUBLIC KEY-----`
			deviceId := "flutter-esp32-mark-one"
			err = provisioner.CreateDevice(deviceId, publicKey)
			if err != nil && err != service.ErrDeviceAlreadyExists {
				log.Fatalf("Error provisioning device: %v", err)
			}

			cmd := struct {
				Power      bool `json:"power,omitempty"`
				Brightness int  `json:"brightness,omitempty"`
			}{
				Power:      true,
				Brightness: 50,
			}

			data, err := json.Marshal(cmd)
			if err != nil {
				log.Fatalf("Failed to encode cmd", err)
			}

			err = deviceControllerSrv.SendCommand(deviceId, string(data))
			if err != nil {
				log.Fatalf("Error sending cmd to device: %v", err)
			}
			err = provisioner.DeleteDevice(deviceId)
			if err != nil {
				log.Fatalf("Error deleting device: %v", err)
			}
	*/
}
