dbus = require 'dbus-native'

class DBusError extends Error
    constructor: (message)->
        super message
        @name = "DBusError"

class DBusInterface
    sessionBus = null

    constructor: ->
        @sessionBus = dbus.sessionBus()

    notifyDBusFailure: (error) ->
        console.trace "DBus failure: #{error}, exiting"
        process.exit 1
        return

    nodemanagerRegisterNode: (callback) ->
        @sessionBus.getService('org.sfietkonstantin.Harmony').getInterface('/nodemanager', 'org.sfietkonstantin.Harmony', (err, iface) ->
            iface.RegisterNode (err) ->
                if err?
                    throw new DBusError("Failed to register to nodemanager")
                else
                    callback()
                return
            return
        )
        return

    nodeconfigurationserviceGetCertificatePath: (callback) ->
        @sessionBus.getService('org.sfietkonstantin.Harmony').getInterface('/nodeconfigurationservice', 'org.freedesktop.DBus.Properties', (err, iface) ->
            iface.Get 'org.sfietkonstantin.Harmony', 'CertificatePath', (err, certificatePath) ->
                if err?
                    throw new DBusError("Failed to get CertificatePath from DBus")
                else
                    callback certificatePath[1][0]
                return
            return
        )
        return

    identificationserviceRegisterClient: (token, password, callback) ->
        @sessionBus.getService('org.sfietkonstantin.Harmony').getInterface('/identificationservice', 'org.sfietkonstantin.Harmony', (err, iface) ->
            iface.RegisterClient token, password, (err, result) ->
                if err?
                    throw new DBusError("Failed to register to identificationservice")
                else
                    callback result
                return
            return
        )
        return
    
    pluginserviceGetPlugins: (callback) ->
        @sessionBus.getService('org.sfietkonstantin.Harmony').getInterface('/pluginservice', 'org.freedesktop.DBus.Properties', (err, iface) =>
            iface.Get 'org.sfietkonstantin.Harmony', 'Plugins', (err, plugins) ->
                if err?
                    throw new DBusError("Failed to get Plugins from DBus")
                else
                    returned = []
                    for plugin in plugins[1][0]
                        returnedPlugin = {"id": plugin[0], "name": plugin[1], "description": plugin[2]}
                        returned.push returnedPlugin
                    callback returned
                return
            return
        )
        return

module.exports = DBusInterface