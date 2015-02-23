angular.module 'Login', []
app = angular.module 'app', ['Login', 'Settings', 'ui.router', 'ui.bootstrap']

app.config ($stateProvider, $urlRouterProvider) ->
    $stateProvider.state('login', {
        url: '/login',
        templateUrl: 'modules/login/views/login.html',
        controller: 'LoginController'
    }).state('home', {
        url: '/',
        templateUrl: 'pages/home.html',
        authenticate: true
    }).state('apps', {
        url: '/apps',
        templateUrl: 'pages/home.html',
        authenticate: true
    }).state('settings', {
        url: '/settings',
        templateUrl: 'modules/settings/views/settings.html',
        controller: 'SettingsController',
        abstract: true,
        authenticate: true
    })
    $urlRouterProvider.otherwise ($injector, $location) ->
        $state = $injector.get "$state"
        $state.go "home"
    return

app.controller 'CollapseController', ($scope)->
    $scope.isCollapsed = true

app.factory 'authInterceptor', ($rootScope, LoginManager) ->
    return {
        request: (config) ->
            config.headers = config.headers || {}
            if LoginManager.isLoggedIn()
                config.headers.Authorization = 'Bearer ' + LoginManager.token
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