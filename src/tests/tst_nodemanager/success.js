 var DBus = require('dbus');
 var dbusInstance = new DBus();
 var bus = dbusInstance.getBus('session');

bus.getInterface('org.sfietkonstantin.Harmony', '/nodemanager', 'org.sfietkonstantin.Harmony', function(err, iface) {
    iface.RegisterNode['timeout'] = 1000;
    iface.RegisterNode();
});
