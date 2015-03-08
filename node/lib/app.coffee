express = require 'express'
morgan = require 'morgan'
bodyParser = require 'body-parser'
methodOverride = require 'method-override'
http = require 'http'
https = require 'https'
fs = require 'fs'
async = require 'async'
jwt = require 'jsonwebtoken'
routes = require './routes'

class NullDBusInterfaceError extends Error
    constructor: ->
        super "DBus interface is null"
        @name = "NullDBusInterfaceError"
        Error.captureStackTrace @, @

class NodeRegistrationError extends Error
    constructor: ->
        super "Failed to register Node"
        @name = "NodeRegistrationError"
        Error.captureStackTrace @, @

class CertificatePathError extends Error
    constructor: ->
        super "Failed to get certificate path"
        @name = "CertificatePathError"
        Error.captureStackTrace @, @

class CertificatesError extends Error
    constructor: ->
        super "Failed to open certificates"
        @name = "CertificatesError"
        Error.captureStackTrace @, @

class PluginsError extends Error
    constructor: ->
        super "Failed to get plugins"
        @name = "PluginsError"
        Error.captureStackTrace @, @

class App
    app = null
    dbusInterface = null
    authManager = null
    plugins = []
    enabledPlugins = []
    options = {}

    constructor: (dbusInterface, authManager) ->
        @app = express()
        @dbusInterface = dbusInterface
        @authManager = authManager

        @app.set 'port', 8080
        @app.set 'view engine', 'ejs'
        @app.use express.static("#{__dirname}/../public")
        @app.use morgan('dev')
        @app.use bodyParser.urlencoded({extended: 'true'})
        @app.use bodyParser.json()
        @app.use bodyParser.json({type: 'application/vnd.api+json'})
        @app.use methodOverride()
        return
    prepare: (done) ->
        if not @dbusInterface?
            done new NullDBusInterfaceError
            return
        async.series [
            (callback) =>
                try
                    @dbusInterface.nodemanagerRegisterNode ->
                        callback null, null
                catch error
                    callback new NodeRegistrationError
                return
            , (callback) =>
                try
                    @dbusInterface.nodeconfigurationserviceGetCertificatePath (certificatePath) ->
                        try
                            keyFile = fs.readFileSync "#{certificatePath}/harmony.key"
                            certFile = fs.readFileSync "#{certificatePath}/harmony.crt"
                            caFile = fs.readFileSync "#{certificatePath}/harmony-ca.crt"
                            options = {
                                certificatePath: certificatePath
                                key: keyFile
                                cert: certFile
                                ca: caFile
                                requestCert: true
                                rejectUnauthorized: false
                            }
                            callback null, options
                        catch error
                            callback new CertificatesError
                        return
                catch error
                    callback new CertificatePathError
                return
            , (callback) =>
                try 
                    plugins = @dbusInterface.pluginserviceGetPlugins (plugins) ->
                        callback null, plugins
                catch error
                    callback new PluginsError
        ], (err, results) =>
            if err?
                done(err)
            else
                @options = results[1]
                @plugins = results[2]
                @enabledPlugins = []

                # Get enabled plugins
                for plugin in @plugins
                    if fs.existsSync "#{__dirname}/../public/modules/#{plugin.id}/#{plugin.id}.js"
                        @enabledPlugins.push plugin
                    else
                        console.log "Plugin #{plugin.id} will not be enabled"

                @authManager.setCertificatePath options.certificatePath

                # Routes
                routes.setup @app, @authManager, @dbusInterface, @enabledPlugins
                done()
            return
        return
    start: (done) ->
        @prepare (error) =>
            if error?
                done(error)
            else
                # Server
                server = https.createServer(@options, @app).listen @app.get('port'), =>
                console.log("Server started on port #{@app.get('port')}")
                done()
            return
        return
module.exports = App