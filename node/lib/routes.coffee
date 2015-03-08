expressJwt = require 'express-jwt'

queryApi = (dbusInterface, id, name, type) ->
    return (req, res) -> 
        dbusInterface.pluginRequest id, type, name, req.query, req.body, (result) ->
            status = result.status
            type = result.type
            value = result.value

            switch type
                when 'json'
                    res.status(status).send(value)
                else
                    res.status(501).send("Type #{type} is not implemented yet")
            return
        return

setup = (app, authManager, dbusInterface, plugins) ->
    app.use '/api', expressJwt({secret: authManager.secret})

    app.get '/', (req, res) =>
        res.render "#{__dirname}/../views/index", { "plugins": plugins }
        return

    app.get '/main.js', (req, res) =>
        res.render "#{__dirname}/../views/main", { "plugins": plugins }
        return

    app.get '/api/plugins', (req, res) =>
        res.json plugins
        return

    app.post '/authenticate', (req, res) ->
        authCode = req.body["password"]
        token = authManager.generateToken authCode
        dbusInterface.identificationserviceRegisterClient token, authCode, (ok) ->
            if ok
                res.json {'token': token}
                return
            else
                res.status(401).send('Wrong authentification code')
                return
            return
        return

    # Endpoints from plugins
    for plugin in plugins
        for endpoint in plugin.endpoints
            switch endpoint.type
                when 'get'
                    app.get "/api/#{plugin.id}/#{endpoint.name}", queryApi(dbusInterface, plugin.id, endpoint.name, endpoint.type)
                when "post"
                    app.post "/api/#{plugin.id}/#{endpoint.name}", queryApi(dbusInterface, plugin.id, endpoint.name, endpoint.type)
                when "delete"
                    app.delete "/api/#{plugin.id}/#{endpoint.name}", queryApi(dbusInterface, plugin.id, endpoint.name, endpoint.type)

    app.get '*', (req, res) ->
        res.redirect '/'
        return

module.exports.setup = setup