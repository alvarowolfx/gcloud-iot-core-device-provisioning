package model

import (
	"context"

	"firebase.google.com/go/db"
)

type DeviceData struct {
	Power      bool  `json:"power"`
	Brightness int   `json:"brightness"`
	Time       int64 `json:"time"`
}

type Device struct {
	ID   string     `json:"id"`
	Data DeviceData `json:"data"`
}

type DeviceRepository interface {
	FindById(ctx context.Context, id string) (*Device, error)
	DeleteById(ctx context.Context, id string) error
}

type FirebaseDeviceRepository struct {
	FirebaseDB *db.Client
}

func NewFirebaseDeviceRepository(firebaseDB *db.Client) DeviceRepository {
	return &FirebaseDeviceRepository{
		FirebaseDB: firebaseDB,
	}
}

func (fdr *FirebaseDeviceRepository) FindById(ctx context.Context, id string) (*Device, error) {
	device := &Device{}
	err := fdr.FirebaseDB.NewRef("/devices").Child(id).Get(ctx, device)
	if err != nil {
		return nil, err
	}

	if device.Data.Time == 0 {
		return nil, err
	}

	device.ID = id
	return device, err
}

func (fdr FirebaseDeviceRepository) DeleteById(ctx context.Context, id string) error {
	return fdr.FirebaseDB.NewRef("/devices").Child(id).Delete(ctx)
}
