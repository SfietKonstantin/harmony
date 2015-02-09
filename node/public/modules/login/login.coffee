login = angular.module 'Login', ['ui.router']

login.factory 'LoginManager', ['$rootScope', ($rootScope)->
    class LoginManager
        token = null
        deleteToken: ->
            wasLoggedIn = @isLoggedIn()
            delete @token
            isLoggedIn = @isLoggedIn()
            @notifyLogin wasLoggedIn, isLoggedIn
        setToken: (token)->
            wasLoggedIn = @isLoggedIn()
            @token = token
            isLoggedIn = @isLoggedIn()
            @notifyLogin wasLoggedIn, isLoggedIn
        isLoggedIn: ->
            return @token?
        notifyLogin: (wasLoggedIn, isLoggedIn) ->
            if wasLoggedIn != isLoggedIn
                $rootScope.$emit "loggedInChanged", isLoggedIn
    new LoginManager()
]

login.controller 'LoginController', ($scope, $http, $state, LoginManager) ->
    LoginManager.deleteToken()
    $scope.submit = ->
        $scope.loading = true
        $http.post('/authenticate', $scope.user
        ).success( (data, status, headers, config)->
            LoginManager.setToken data.token
            $scope.loading = false
            $state.go 'home'
            return
        ).error( (data, status, headers, config) ->
            LoginManager.deleteToken()
            $scope.error = data
            $scope.loading = false
            $scope.user.password = null
            return
        )
        return
    return