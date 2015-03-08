test = angular.module 'Test', ['ui.router']

test.config ($stateProvider, $urlRouterProvider) ->
    $stateProvider.state('test.main', {
        url: '',
        authenticate: true
    })
    return

test.controller 'TestController', ($scope, $http) ->
    $scope.sendRequest = (type)->
        console.log "Sending #{type.toUpperCase()} to /api/test/test_#{type}"
        request = $http {
            method: type.toUpperCase()
            url: "/api/test/test_#{type}"
            params: {str: "value", double: 123.456}
            data: {str: "test", int: 3}
        }
        request.success( (data, status, headers, config)->
            console.log "#{type.toUpperCase()} success: status #{status}, data #{JSON.stringify data}"
            return
        ).error( (data, status, headers, config) ->
            console.log "#{type.toUpperCase()} error: status #{status}, data #{JSON.stringify data}"
            return
        )
        return
