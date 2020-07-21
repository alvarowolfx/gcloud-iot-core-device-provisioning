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

	firebaseApp := infrastructure.FirebaseApp()
	firebaseAuth := infrastructure.FirebaseAuth(firebaseApp)
	firebaseDB := infrastructure.FirebaseConnect(firebaseApp)
	ciotService, parent, projectID, region, registryID := infrastructure.IoTCoreConnect()
	provisioner := service.NewIoTCoreDeviceProvisioner(ciotService, parent, projectID, region, registryID)
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
	checkUserMiddleware := func(c *fiber.Ctx) {
		ctx := c.Context()
		authHeader := c.Get(fiber.HeaderAuthorization)
		idToken := ""
		authScheme := "Bearer:"
		l := len(authScheme)
		if len(authHeader) > l+1 && authHeader[:l] == authScheme {
			idToken = authHeader[l+1:]
		} else {
			c.Status(401).JSON(fiber.Map{"message": "Missing or malformed JWT token"})
			return
		}
		token, err := firebaseAuth.VerifyIDToken(ctx, idToken)
		if err != nil {
			c.Status(401).JSON(fiber.Map{"message": err.Error()})
			return
		}
		c.Locals("userID", token.UID)
		c.Next()
	}

	app.Get("/", func(c *fiber.Ctx) {
		c.JSON(fiber.Map{
			"healthCheck": "ok",
		})
	})

	app.Get("/api/devices/:deviceId", checkUserMiddleware, deviceController.CheckDeviceById)
	app.Post("/api/devices/:deviceId", checkUserMiddleware, deviceController.CreateDevice)
	app.Delete("/api/devices/:deviceId", checkUserMiddleware, deviceController.DeleteDevice)
	app.Post("/api/devices/:deviceId/command", checkUserMiddleware, deviceController.SendCommand)
	app.Post("/api/users/devices/:deviceId", checkUserMiddleware, deviceController.AddDeviceToUser)
	//app.Get("/api/devices", heroController.GetHeroesRecommendations)

	err = app.Listen(port)
	if err != nil {
		log.Fatalf("error initializing app: %v\n", err)
	}
}
