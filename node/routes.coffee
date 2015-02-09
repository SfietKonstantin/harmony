module.exports.index = (req, res) ->
    res.sendFile "#{__dirname}/public/index.html"

module.exports.redirectToIndex = (req, res) ->
        res.redirect '/'
