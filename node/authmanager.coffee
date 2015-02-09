fs = require 'fs'
jwt = require 'jsonwebtoken'

class AuthManager
    secret = null

    constructor: (certificatePath) ->
        @secret = fs.readFileSync "#{certificatePath}/harmony-key.pub"

    generateToken: (authCode) =>
        token = jwt.sign({'auth_code': authCode}, @secret, { expiresInMinutes: 60 * 24 })
        return token

module.exports = AuthManager