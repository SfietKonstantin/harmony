test = angular.module 'Test', ['ui.router']

test.config ($stateProvider, $urlRouterProvider) ->
    $stateProvider.state('test.main', {
        url: '',
        authenticate: true
    })
    return

test.controller 'TestController', ($scope) ->
    return
