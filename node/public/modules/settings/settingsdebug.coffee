settingsDebug = angular.module 'Settings.Debug', []

settingsDebug.controller 'SettingsDebugController', ($scope, $http) ->
    $scope.havePlugins = false
    $http({
        url: '/api/list'
        method: 'GET'
    }).success((data, status, header, config) ->
        $scope.plugins = data
        $scope.havePlugins = data.length > 0
    ).error((data, status, header, config)->
        $scope.plugins = []
        $scope.havePlugins = false
    )
    return
