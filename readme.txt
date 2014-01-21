This is built off Helco's PebbleLocalSim: https://github.com/Helco/PebbleLocalSim.  Please see the readme.txt there for additional usage information.

I have added support for the Dictionary, AppMessage, and AppSync APIs, along with support for the JavaScript API.

You must have NodeJS installed for the JavaScript API.  I have only added support for Linux.  Adding support for other platforms should be straightforward.

I make use fo the following:
* https://github.com/driverdan/node-XMLHttpRequest/blob/master/lib/XMLHttpRequest.js.
* https://github.com/ae-code/node-localStorage

Testing of my changes have been very minimal.  I was basically waiting for my Pebble watch (on backorder) to be delivered and was too excited to wait before getting started.

The weather and quotes example faces should work well.
