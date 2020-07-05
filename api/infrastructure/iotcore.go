package infrastructure

import (
	"context"
	"fmt"
	"log"
	"os"

	"google.golang.org/api/cloudiot/v1"
)

func IoTCoreConnect() (*cloudiot.Service, string) {
	projectID := os.Getenv("GCP_PROJECT")
	if projectID == "" {
		// App Engine uses another name
		projectID = os.Getenv("GOOGLE_CLOUD_PROJECT")
	}
	registryId := os.Getenv("REGISTRY_ID")
	region := os.Getenv("REGION")

	parent := fmt.Sprintf("projects/%s/locations/%s/registries/%s", projectID, region, registryId)

	ctx := context.Background()
	ciotService, err := cloudiot.NewService(ctx)
	if err != nil {
		log.Fatalf("Failed to connect to IoT Core: %v", err)
	}

	return ciotService, parent
}
