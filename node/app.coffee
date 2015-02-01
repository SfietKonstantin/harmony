express = require 'express'
morgan = require 'morgan'
bodyParser = require 'body-parser'
methodOverride = require 'method-override'

app = express()
app.use express.static("#{__dirname}/public")
app.use morgan('dev')
app.use bodyParser.urlencoded({extended: 'true'})
app.use bodyParser.json()
app.use bodyParser.json({type: 'application/vnd.api+json'})
app.use methodOverride()

app.get '*', (req, res) ->
    res.sendfile './public/index.html'

app.listen 8080
console.log "Server started"
