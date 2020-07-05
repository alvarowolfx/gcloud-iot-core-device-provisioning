package model

import (
	"context"
	"fmt"

	"firebase.google.com/go/db"
)

type UserRepository interface {
	ListDevicesByUser(ctx context.Context, userId string) ([]string, error)
	AddDeviceToUser(ctx context.Context, userId, deviceId string) error
	RemoveDeviceFromUser(ctx context.Context, userId, deviceId string) error
	RemoveDeviceFromAllUsers(ctx context.Context, deviceId string) error
}

type FirebaseUserRepository struct {
	FirebaseDB *db.Client
}

func NewFirebaseUserRepository(firebaseDB *db.Client) UserRepository {
	return &FirebaseUserRepository{
		FirebaseDB: firebaseDB,
	}
}

func (fur *FirebaseUserRepository) ListDevicesByUser(ctx context.Context, userId string) ([]string, error) {
	devicesMap := make(map[string]bool)
	err := fur.FirebaseDB.NewRef("/user_devices").Child(userId).Get(ctx, &devicesMap)
	if err != nil {
		return nil, err
	}

	var deviceIds []string
	for deviceId := range devicesMap {
		deviceIds = append(deviceIds, deviceId)
	}

	return deviceIds, nil
}

func (fur FirebaseUserRepository) AddDeviceToUser(ctx context.Context, userId, deviceId string) error {
	err := fur.FirebaseDB.NewRef("/device_users").Child(deviceId).Child(userId).Set(ctx, true)
	if err != nil {
		return err
	}

	err = fur.FirebaseDB.NewRef("/user_devices").Child(userId).Child(deviceId).Set(ctx, true)
	if err != nil {
		return err
	}

	return nil
}

func (fur FirebaseUserRepository) RemoveDeviceFromUser(ctx context.Context, userId, deviceId string) error {
	changes := map[string]interface{}{}
	changes["/device_users/"+deviceId+"/"+userId] = nil
	changes["/user_devices/"+userId+"/"+deviceId] = nil
	return fur.FirebaseDB.NewRef("/").Update(ctx, changes)
}

func (fur FirebaseUserRepository) RemoveDeviceFromAllUsers(ctx context.Context, deviceId string) error {
	usersMap := map[string]bool{}
	fmt.Print("Get Users map")
	err := fur.FirebaseDB.NewRef("/device_users").Child(deviceId).Get(ctx, &usersMap)
	if err != nil {
		return err
	}

	if len(usersMap) == 0 {
		return nil
	}

	changes := make(map[string]interface{})
	for userId := range usersMap {
		changes["/device_users/"+deviceId+"/"+userId] = nil
		changes["/user_devices/"+userId+"/"+deviceId] = nil
	}

	return fur.FirebaseDB.NewRef("/").Update(ctx, changes)
}
