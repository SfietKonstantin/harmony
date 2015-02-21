dbus = require 'dbus'

class DBusError extends Error
    constructor: (message)->
        super message
        @name = "DBusError"

class DBusInterface
    sessionBus = null

    constructor: ->
        bus = new dbus()
        @sessionBus = bus.getBus 'session'

    notifyDBusFailure: (error) ->
        console.trace "DBus failure: #{error}, exiting"
        process.exit 1
        return

    nodemanagerRegisterNode: (callback) ->
        @sessionBus.getInterface 'org.sfietkonstantin.Harmony', '/nodemanager', 'org.sfietkonstantin.Harmony', (err, iface) ->
            if iface?
                iface.RegisterNode['timeout'] = 1000
                iface.RegisterNode['finish'] = (result) ->
                    callback()
                iface.RegisterNode()
            else
                throw new DBusError("Failed to register to nodemanager")
            return
        return

    nodeconfigurationserviceGetCertificatePath: (callback) ->
        @sessionBus.getInterface 'org.sfietkonstantin.Harmony', '/nodeconfigurationservice', 'org.sfietkonstantin.Harmony', (err, iface) =>
            if iface?
                iface.getProperty 'CertificatePath', (err, certificatePath) ->
                    callback certificatePath
            else
                throw new DBusError("Failed to get CertificatePath from DBus")
            return
        return

    identificationserviceRegisterClient: (token, password, callback) ->
        @sessionBus.getInterface 'org.sfietkonstantin.Harmony', '/identificationservice', 'org.sfietkonstantin.Harmony', (err, iface) ->
            if iface?
                iface.RegisterClient['timeout'] = 1000
                iface.RegisterClient['finish'] = (result) ->
                    callback(result)
                iface.RegisterClient(token, password)
            else
                throw new DBusError("Failed to register to identificationservice")
            return
        return

module.exports = DBusInterface