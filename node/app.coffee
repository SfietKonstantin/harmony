app = require './lib/app'
authmanager = require './lib/authmanager'
dbusinterface = require './dbusinterface'

harmonyApp = new app(new dbusinterface, new authmanager)
harmonyApp.start (err) ->
    if err?
        console.log err.stack
        process.exit 1
