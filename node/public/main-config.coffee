angular.module 'Login', []
app = angular.module 'app'

app.config ($stateProvider, $urlRouterProvider) ->
    # State management done in index.ejs
    $urlRouterProvider.otherwise ($injector, $location) ->
        $state = $injector.get "$state"
        $state.go "home"
    return

app.controller 'ToolbarController', ($rootScope, $scope, $http)->
    $scope.isCollapsed = true
    $scope.isAppsOpened = false

    $rootScope.$on "loggedInChanged", (event, isLoggedIn) ->
        $http({
            url: '/api/plugins'
            method: 'GET'
        }).success((data, status, header, config) ->
            $scope.plugins = data
        ).error((data, status, header, config)->
            $scope.plugins = []
        )
        return

app.factory 'authInterceptor', ($rootScope, LoginManager) ->
    return {
        request: (config) ->
            config.headers = config.headers || {}
            if LoginManager.isLoggedIn()
                config.headers.Authorization = "Bearer #{LoginManager.token}"
            return config
    }
app.config ($httpProvider) ->
    $httpProvider.interceptors.push 'authInterceptor'

app.run ($rootScope, $state, LoginManager)->
    $rootScope.$on "$stateChangeStart", (event, toState, toParams, fromState, fromParams) ->
        if toState.authenticate and not LoginManager.isLoggedIn()
            $state.go "login"
            event.preventDefault()
        return
    $rootScope.$on "loggedInChanged", (event, isLoggedIn) ->
        $rootScope.loggedIn = isLoggedIn
    return