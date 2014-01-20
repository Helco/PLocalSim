/////////////////////////////////////////////////
// Standard header to setup Pebble environment

var Pebble = require('./simdata/server');

var window = window || {};
window.navigator = { };
window.navigator.geolocation = Pebble.geolocation;

var localStorage = Pebble.localStorage;

var XMLHttpRequest = require('./simdata/XMLHttpRequest').XMLHttpRequest;

console.log = ((function() {
    var orig_log = console.log;
    return function() {
        if ((arguments.length > 0) &&
            (typeof(arguments[0]) === 'string'))
            arguments[0] = "[JS App] " + arguments[0];

        orig_log.apply(console, arguments);
    };
})());

/////////////////////////////////////////////////


