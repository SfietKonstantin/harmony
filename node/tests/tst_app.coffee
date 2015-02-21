assert = require 'assert'
app = require '../lib/app'
supertest = require 'supertest'

class DBusMock
    registerNodeOk = false
    getCertificatePathOk = false
    certificatesPath = ""
    
    constructor: (registerNodeOk, getCertificatePathOk, certificatesPath) ->
        @registerNodeOk = registerNodeOk
        @getCertificatePathOk = getCertificatePathOk
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

class AuthManagerMock
    certificatePath = ""
    
    setCertificatePath: (certificatePath) ->
        @certificatePath = certificatePath

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
            appInstance = new app new DBusMock(false, true), new AuthManagerMock
            appInstance.start (err) ->
                assert err?, "Failure to start"
                assert.equal err.name, "NodeRegistrationError"
                done()
        it "Start with DBus, getCertificatePathFail", (done) ->
            appInstance = new app new DBusMock(true, false), new AuthManagerMock
            appInstance.start (err) ->
                assert err?, "Failure to start"
                assert.equal err.name, "CertificatePathError"
                done()
        it "Start with DBus, getCertificates", (done) ->
            appInstance = new app new DBusMock(true, true), new AuthManagerMock
            appInstance.start (err) ->
                assert err?, "Failure to start"
                assert.equal err.name, "CertificatesError"
                done()
        it "Start with DBus, success", (done) ->
            certificatePath = "#{__dirname}/ssl"
            authManager = new AuthManagerMock
            appInstance = new app new DBusMock(true, true, certificatePath), authManager
            appInstance.start (err) ->
                assert not err?, "Started"
                assert.equal authManager.certificatePath, certificatePath
                done()
