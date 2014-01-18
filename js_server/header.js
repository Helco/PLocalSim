/////////////////////////////////////////////////
// Standard header to setup Pebble environment

var Pebble = require('./simdata/server');

var window = window || {};
window.navigator = { };
window.navigator.geolocation = Pebble.geolocation;

var localStorage = Pebble.localStorage;

var XMLHttpRequest = require('./simdata/XMLHttpRequest').XMLHttpRequest;

/////////////////////////////////////////////////


