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
                        endpoints = []
                        for endpoint in plugin[3]
                            endpoints.push {"type": endpoint[0], "name": endpoint[1]}
                        returnedPlugin = {"id": plugin[0], "name": plugin[1], "description": plugin[2], "endpoints": endpoints}
                        returned.push returnedPlugin
                    callback returned
                return
            return
        )
        return

    pluginRequest: (id, type, name, params, body, callback) ->
        @sessionBus.getService('org.sfietkonstantin.Harmony').getInterface("/plugin/#{id}", 'org.sfietkonstantin.Harmony', (err, iface) ->
            realParams = null
            if typeof params is 'string'
                realParams = params
            if typeof params is 'object'
                realParams = JSON.stringify params
            if not params?
                realParams = ""

            realBody = null
            if typeof body is 'string'
                realBody = body
            if typeof body is 'object'
                realBody = JSON.stringify body
            if not body?
                realBody = ""

            if realParams? and realBody?
                iface.Request [type, name], realParams, realBody, (err, result) ->
                    if err?
                        throw new DBusError("Failed to perform a request")
                    else
                        callback {status: result[0], type: result[1], value: result[2]}
            else
                callback null
        )
        return

module.exports = DBusInterface