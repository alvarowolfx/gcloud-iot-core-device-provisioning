package infrastructure

import (
	"context"
	"log"
	"os"

	firebase "firebase.google.com/go"
	"firebase.google.com/go/auth"
	"firebase.google.com/go/db"
)

func FirebaseApp() *firebase.App {
	projectID := os.Getenv("GCP_PROJECT")
	if projectID == "" {
		// App Engine uses another name
		projectID = os.Getenv("GOOGLE_CLOUD_PROJECT")
	}

	config := &firebase.Config{
		ProjectID:   projectID,
		DatabaseURL: "https://" + projectID + ".firebaseio.com",
	}

	ctx := context.Background()
	fbApp, err := firebase.NewApp(ctx, config)
	if err != nil {
		log.Fatalf("error initializing app: %v\n", err)
	}

	return fbApp
}

func FirebaseAuth(fbApp *firebase.App) *auth.Client {
	ctx := context.Background()
	auth, err := fbApp.Auth(ctx)
	if err != nil {
		log.Fatalln("Error initializing auth client:", err)
	}

	return auth
}

func FirebaseConnect(fbApp *firebase.App) *db.Client {
	ctx := context.Background()

	firebaseDB, err := fbApp.Database(ctx)
	if err != nil {
		log.Fatalln("Error initializing database client:", err)
	}

	return firebaseDB
}
