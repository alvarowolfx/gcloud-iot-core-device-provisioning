# API Endpoints

// Check if device is created and is reporting
GET api/devices/{deviceId}

// Delete device on IoT Core
DELETE api/devices/{deviceId}

// Create device on IoT Core
POST api/devices/{deviceId}
{
publicKey : ""
}

POST api/devices/{deviceId}/command
{

}

// Add to my account
POST api/users/me/devices/{deviceId}

// Invite another user
POST api/users/{userId}/devices/{deviceId}
