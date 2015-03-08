assert = require 'assert'
app = require '../lib/app'
authManager = require '../lib/authmanager'
supertest = require 'supertest'

testPlugin = {
    id: "test"
    name: "Test"
    description: ""
    endpoints: [
        {
            type: "get"
            name: "test_get"
        },
        {
            type: "post"
            name: "test_post"
        },
        {
            type: "delete"
            name: "test_delete"
        }
    ]
}

invalidPlugin = {
    id: "invalid"
    name: "Invalid"
    description: ""
    endpoints: []
}


class DBusMock
    registerNodeOk = false
    getCertificatePathOk = false
    getPluginsOk = false
    certificatesPath = ""

    constructor: (registerNodeOk, getCertificatePathOk, getPluginsOk, certificatesPath) ->
        @registerNodeOk = registerNodeOk
        @getCertificatePathOk = getCertificatePathOk
        @getPluginsOk = getPluginsOk
        @certificatesPath = certificatesPath

    nodemanagerRegisterNode: (callback)->
        if not @registerNodeOk
            throw new Error("nodeManagerRegisterNode error")
        else
            callback()

    nodeconfigurationserviceGetCertificatePath: (callback) ->
        if not @getCertificatePathOk
            throw new Error("nodeconfigurationserviceGetCertificatePath error")
        else
            callback @certificatesPath
    pluginserviceGetPlugins: (callback) ->
        if not @getPluginsOk
            throw new Error("pluginserviceGetPlugins error")
        else
            callback [testPlugin, invalidPlugin]
    identificationserviceRegisterClient: (token, authCode, callback) ->
        if authCode == "pass"
            callback true
        else
            callback false

class AuthManagerMock extends authManager
    setCertificatePath: (certificatePath) ->
        @certificatePath = certificatePath
        @secret = certificatePath

runRequest = (callback) ->
    certificatePath = "#{__dirname}/ssl"
    appInstance = new app new DBusMock(true, true, true, certificatePath), new AuthManagerMock
    appInstance.prepare (err) ->
        assert not err?, "Prepared"
        assert appInstance.app, "Express app is available"
        callback(appInstance.app)


describe "App", ->
    describe "init", ->
        it "App creation", ->
            appInstance = new app null, null
            assert appInstance?, "App created"
    describe "start", ->
        it "Start failure without DBus", (done)->
            appInstance = new app null, null
            appInstance.start (err) ->
                assert err?, "Failure to start"
                assert.equal err.name, "NullDBusInterfaceError"
                done()
        it "Start with DBus, registerNodeFail", (done) ->
            appInstance = new app new DBusMock(false, true, true), new AuthManagerMock
            appInstance.start (err) ->
                assert err?, "Failure to start"
                assert.equal err.name, "NodeRegistrationError"
                done()
        it "Start with DBus, getCertificatePathFail", (done) ->
            appInstance = new app new DBusMock(true, false, true), new AuthManagerMock
            appInstance.start (err) ->
                assert err?, "Failure to start"
                assert.equal err.name, "CertificatePathError"
                done()
        it "Start with DBus, getCertificates", (done) ->
            appInstance = new app new DBusMock(true, true, true), new AuthManagerMock
            appInstance.start (err) ->
                assert err?, "Failure to start"
                assert.equal err.name, "CertificatesError"
                done()
        it "Start with DBus, pluginserviceGetPlugins", (done) ->
            certificatePath = "#{__dirname}/ssl"
            appInstance = new app new DBusMock(true, true, false, certificatePath), new AuthManagerMock
            appInstance.start (err) ->
                assert err?, "Failure to start"
                assert.equal err.name, "PluginsError"
                done()
        it "Start with DBus, success", (done) ->
            certificatePath = "#{__dirname}/ssl"
            authManager = new AuthManagerMock
            appInstance = new app new DBusMock(true, true, true, certificatePath), authManager
            appInstance.start (err) ->
                assert not err?, "Started"
                assert.equal authManager.certificatePath, certificatePath
                assert.deepEqual appInstance.plugins, [testPlugin, invalidPlugin]
                assert.deepEqual appInstance.enabledPlugins, [testPlugin]
                done()

describe "API", ->
    server = null
    token = ""
    before (done) ->
        certificatePath = "#{__dirname}/ssl"
        appInstance = new app new DBusMock(true, true, true, certificatePath), new AuthManagerMock
        appInstance.prepare (err) ->
            assert not err?, "Prepared"
            assert appInstance.app, "Express app is available"
            server = appInstance.app
            done()
    describe "Login", ->
        it "GET /", (done) ->
            supertest(server)
            .get '/' 
            .expect 200
            .expect 'Content-Type', /html/
            .end (err, res) ->
                assert not err?, "Request succesful"
                done()
        it "Redirect GET /", (done) ->
            supertest(server)
            .get '/redirect' 
            .expect 302
            .expect 'Location', "/"
            .end (err, res) ->
                assert not err?, "Request succesful"
                done()
        it "GET /main.js", (done) ->
            supertest(server)
            .get '/main.js' 
            .expect 200
            .end (err, res) ->
                assert not err?, "Request succesful"
                done()
        it "POST /authenticate failed", (done) ->
            supertest(server)
            .post '/authenticate'
            .send {password: 'INVALID'}
            .expect 401
            .end (err, res) ->
                assert not err?, "Request succesful"
                done()
        it "POST /authenticate success", (done) ->
            supertest(server)
            .post '/authenticate'
            .send {password: 'pass'}
            .expect 200
            .end (err, res) ->
                assert not err?, "Request succesful"
                assert res.body.token?
                token = res.body.token
                done()
        it "GET /api/plugins", (done) ->
            supertest(server)
            .get '/api/plugins' 
            .set 'Authorization', "Bearer #{token}"
            .expect 200
            .expect 'Content-Type', /json/
            .end (err, res) ->
                assert not err?, "Request succesful"
                assert.deepEqual res.body, [testPlugin]
                done()