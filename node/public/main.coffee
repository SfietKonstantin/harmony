angular.module 'Login', []
app = angular.module 'app', ['Login', 'ui.router']

app.config ($stateProvider, $urlRouterProvider) ->
    $urlRouterProvider.otherwise '/'
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
    })
    return

app.run ($rootScope, $state, LoginManager)->
    $rootScope.$on "$stateChangeStart", (event, toState, toParams, fromState, fromParams) ->
        if toState.authenticate and not LoginManager.isLoggedIn()
            $state.transitionTo "login"
            event.preventDefault()
        return
    $rootScope.$on "loggedInChanged", (event, isLoggedIn) ->
        $rootScope.loggedIn = isLoggedIn
    return