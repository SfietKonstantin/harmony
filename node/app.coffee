express = require 'express'
morgan = require 'morgan'
bodyParser = require 'body-parser'
methodOverride = require 'method-override'
http = require 'http'
https = require 'https'
fs = require 'fs'
dbus = require 'dbus'
async = require 'async'
jwt = require 'jsonwebtoken'
expressJwt = require 'express-jwt'
routes = require './routes'
authmanager = require './authmanager'
dbusinterface = require './dbusinterface'

class App
    app = null
    dbusInterface = null
    authManager = null

    constructor: ->
        @app = express()
        @app.set 'port', 8080
        @app.use express.static("#{__dirname}/public")
        @app.use morgan('dev')
        @app.use bodyParser.urlencoded({extended: 'true'})
        @app.use bodyParser.json()
        @app.use bodyParser.json({type: 'application/vnd.api+json'})
        @app.use methodOverride()
        @dbusInterface = new dbusinterface()

        # Routes
        @app.get '/', routes.index
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
        return
    start: ->
        async.series [ 
            (callback) =>
                @dbusInterface.nodemanagerRegisterNode ->
                    callback null, null
                return
            , (callback) =>
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
                        console.log error.stack
                        console.trace "Failed to get certificates, exiting"
                        process.exit 1
                    return
                return
            ], (err, results) =>
                options = results[1]
                @authManager = new authmanager options.certificatePath
                server = https.createServer(options, @app).listen @app.get('port'), =>
                    console.log("Server started on port #{@app.get('port')}")
                return
        return
    
app = new App()
app.start()
