const functions = require('firebase-functions')
const admin = require('firebase-admin')

admin.initializeApp()
const db = admin.database()
const devicesRef = db.ref('devices')

exports.processDeviceData = functions.pubsub
  .topic('devices-inbound')
  .onPublish( (message) => {
  const attributes = message.attributes
  const data = message.json
  const { deviceId } = attributes

  return devicesRef.child(deviceId).child('data').update({
    ...data,
    time : Date.now()
  })
})
