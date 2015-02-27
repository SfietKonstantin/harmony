assert = require 'assert'
app = require '../lib/app'
supertest = require 'supertest'

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
            callback []

class AuthManagerMock
    certificatePath = ""
    secret = null
    
    setCertificatePath: (certificatePath) ->
        @certificatePath = certificatePath
        @secret = certificatePath

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
                done()
