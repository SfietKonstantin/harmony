express = require 'express'
morgan = require 'morgan'
bodyParser = require 'body-parser'
methodOverride = require 'method-override'
http = require 'http'
https = require 'https'
fs = require 'fs'
async = require 'async'
jwt = require 'jsonwebtoken'
expressJwt = require 'express-jwt'
routes = require './routes'

class NullDBusInterfaceError extends Error
    constructor: ->
        super "DBus interface is null"
        @name = "NullDBusInterfaceError"

class NodeRegistrationError extends Error
    constructor: ->
        super "Failed to register Node"
        @name = "NodeRegistrationError"

class CertificatePathError extends Error
    constructor: ->
        super "Failed to get certificate path"
        @name = "CertificatePathError"
        
class CertificatesError extends Error
    constructor: ->
        super "Failed to open certificates"
        @name = "CertificatesError"

class App
    app = null
    dbusInterface = null
    authManager = null

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
    start: (done) ->
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
        ], (err, results) =>
            if err?
                done(err)
                return
            else
                options = results[1]
                @authManager.setCertificatePath options.certificatePath

                # Routes
                @app.use '/api', expressJwt({secret: @authManager.secret})
                @app.get '/', routes.index
                @app.get '/api/plugins', (req, res) =>
                    @dbusInterface.pluginserviceGetPlugins (plugins) ->
                        res.json plugins
                @app.get '*', routes.redirectToIndex
                @app.post '/authenticate', (req, res) =>
                    authCode = req.body["password"]
                    token = @authManager.generateToken authCode
                    @dbusInterface.identificationserviceRegisterClient token, authCode, (ok) ->
                        if ok
                            res.json {'token': token}
                            return
                        else
                            res.status(401).send('Wrong authentification code')
                            return
                        return
                    return

                # Server
                server = https.createServer(options, @app).listen @app.get('port'), =>
                    console.log("Server started on port #{@app.get('port')}")
                    done()
                    return
                return
module.exports = App