app = angular.module('app', ['ui.router'])

app.config ($stateProvider, $urlRouterProvider) ->
    $urlRouterProvider.otherwise '/home'
    $stateProvider.state('login', {
        url: '/login',
        templateUrl: 'pages/login.html',
        authenticate: false
    }).state('home', {
        url: '/home',
        templateUrl: 'pages/home.html',
        authenticate: true
    }).state('apps', {
        url: '/apps',
        templateUrl: 'pages/home.html',
        authenticate: true
    })

app.run ($rootScope, $state)->
    $rootScope.$on "$stateChangeStart", (event, toState, toParams, fromState, fromParams) ->
        if toState.authenticate
            $state.transitionTo "login"
            event.preventDefault()
