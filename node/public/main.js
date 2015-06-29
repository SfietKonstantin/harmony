var app = angular.module('app', ['Login', 'Settings', 'Test','ui.router', 'ui.bootstrap']);
app.config(function($stateProvider, $urlRouterProvider) {
    $stateProvider.state('login', {
        url: '/login',
        templateUrl: 'modules/login/views/login.html',
        controller: 'LoginController'
    }).state('home', {
        url: '/',
        templateUrl: 'modules/home/views/home.html',
        authenticate: true
    }).state('settings', {
        url: '/settings',
        templateUrl: 'modules/settings/views/settings.html',
        controller: 'SettingsController',
        abstract: true,
        authenticate: true
    });

    $stateProvider.state('test', {
        url: '/test',
        templateUrl: 'modules/test/views/test.html',
        controller: 'TestController',
        abstract: true,
        authenticate: true
    });
}); 
