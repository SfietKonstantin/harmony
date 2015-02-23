settings = angular.module 'Settings', ['ui.router', 'Settings.Debug']

settings.config ($stateProvider, $urlRouterProvider) ->
    $stateProvider.state('settings.main', {
        url: '',
        templateUrl: 'modules/settings/views/settings-general.html',
        authenticate: true
    }).state('settings.debug', {
        url: '/debug',
        templateUrl: 'modules/settings/views/settings-debug.html',
        controller: 'SettingsDebugController',
        authenticate: true
    }).state('settings.about', {
        url: '/about',
        templateUrl: 'modules/settings/views/settings-general.html',
        authenticate: true
    })
    return

settings.controller 'SettingsController', ($scope, $http, $state) ->
    return
