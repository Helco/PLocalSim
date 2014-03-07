var net = require("net");
var fs = require("fs");

// event listener callbacks
var readyFunc = null;
var messageFunc = null;

// part of Pebble API
module.exports.addEventListener = function(event, fn) {
    if (typeof(fn) !== 'function')
        return;

    if (event === "ready")
        readyFunc = fn;
    else if (event == "appmessage")
        messageFunc = fn;
    else if (event == "webviewclosed")
        ;
    else
        throw "Invalid Event";

    console.log("[INFO] Added event listener for: " + event);
};

// helper class to translate keys to friendly name and vice versa
function KeyTranslator(filename) {
    this.nameToKey = null;
    this.keyToName = null;

    // pull out friendly names from appKeys section of appinfo.json
    try {
        var contents = fs.readFileSync("appinfo.json");
        contents = JSON.parse(contents);
        if (contents.appKeys) {
            this.nameToKey = contents.appKeys;
            this.keyToName = { };
            for (var n in contents.appKeys) {
                if (contents.appKeys.hasOwnProperty(n)) {
                    this.keyToName[contents.appKeys[n]] = n;
                }
            }
        }
    }
    catch(e) {
        // appinfo.json should be there but swallow error if it isnt
        console.log("[WARN] appinfo.json could not be read");
    }
}

// name --> key (integer)
KeyTranslator.prototype.translateName = function(name) {
    if (!this.nameToKey || (this.nameToKey[name]==undefined)) {
        var ikey = parseInt(name);
        if (isNaN(ikey))
            throw "Invalid Key";
        return ikey;
    }

    return this.nameToKey[name];
};

// key --> name (string)
// non-translated name is just key cast to string
KeyTranslator.prototype.translateKey = function(key) {
    if (!this.keyToName)
        return new String(key);

    if (!this.keyToName[key])
        return new String(key);

    return this.keyToName[key];
};

// helper class for paring buffer to JSON
function MessageParser(keyTranslator) {
    this.keyTranslator = keyTranslator;
}

// type map
MessageParser.prototype.TupleTypes = {
    BYTE_ARRAY: 0,
    CSTRING: 1,
    UINT: 2,
    INT: 3
};

// buffer --> JSON
MessageParser.prototype.parse = function(buffer) {
    var msg = { };

    for (var i = 0; i < buffer.length; ) {
        var key = this.keyTranslator.translateKey(buffer.readUInt32LE(i));
        var type = buffer.readUInt8(i + 4);
        var length = buffer.readUInt16LE(i + 5);

        var value;
        if (type == this.TupleTypes.BYTE_ARRAY) {
            value = [];
            for (var x = i+7; x < (i+length+7); ++x)
                value.push(buffer.readUInt8(x));
        }
        else if (type == this.TupleTypes.CSTRING) {
            value = buffer.toString("utf8", i+7, i+length+7-1);
        }
        else if (type == this.TupleTypes.UINT) {
            if (length == 1)
                value = buffer.readUInt8(i+7);
            else if (length == 2)
                value = buffer.readUInt16LE(i+7);
            else if (length == 4)
                value = buffer.readUInt32LE(i+7);
        }
        else if (type == this.TupleTypes.INT) {
            if (length == 1)
                value = buffer.readInt8(i+7);
            else if (length == 2)
                value = buffer.readInt16LE(i+7);
            else if (length == 4)
                value = buffer.readInt32LE(i+7);
        }
        else {
            console.log("[ERROR] Corrupt Message");
            throw "Corrupt Message";
        }

        msg[key] = value;
        i += 7 + length;
    }

    return msg;
};

MessageParser.prototype.serializeMsg = function(msg) {
    // compute size
    var size = 0;
    for (var k in msg) {
        if (msg.hasOwnProperty(k)) {
            size += 7;

            if (typeof(msg[k]) == "string")
                size += msg[k].length+1;
            else if (typeof(msg[k]) == "number")
                size += 4; // assume 32-bit
            else if (typeof(msg[k]) == "array")
                size += msg[k].length;
            else
                throw "Unable to serialize message";
        }
    }

    // allocate buffer
    var buffer = new Buffer(size+2);

    // serialize
    var offset = 0;
    buffer.writeUInt16LE(size, offset);
    offset += 2;
    for (var k in msg) {
        if (msg.hasOwnProperty(k)) {
            // key
            buffer.writeUInt32LE(this.keyTranslator.translateName(k), offset);

            // type, length, and data
            var dataSize = 0
            if (typeof(msg[k]) == "string") {
                buffer.writeUInt8(this.TupleTypes.CSTRING, offset+4);
                dataSize = msg[k].length+1;
                buffer.writeUInt16LE(dataSize, offset+5);
                buffer.write(msg[k], offset+7, dataSize-1);
                buffer.writeUInt8(0, offset+7+dataSize-1);
            }
            else if (typeof(msg[k]) == "number") {
                buffer.writeUInt8(this.TupleTypes.INT, offset+4); // assume signed
                dataSize = 4; // assume 32-bit
                buffer.writeUInt16LE(dataSize, offset+5);
                buffer.writeInt32LE(msg[k], offset+7);
            }
            else if (typeof(msg[k]) == "array") {
                buffer.writeUInt8(this.TupleTypes.BYTE_ARRAY, offset+4);
                dataSize = msg[k].length;
                buffer.writeUInt16LE(dataSize, offset+5);
                for (var x = 0; x < dataSize; ++x) {
                    if (typeof(msg[k]) !== "number")
                        throw "Invalid Byte Array";
                    buffer.writeUInt8(msg[k][x], offset+7+x);
                }
            }

            offset += 7 + dataSize;
        }
    }

    return buffer;
};

// parser to be used by server
var msgParser = new MessageParser(new KeyTranslator("appinfo.json"));

// the receiving server
var server = net.createServer(function(c) {
    var States = {
        ReadingSize: 1,  // expecting size
        ReadingData: 2   // expecting data
    };

    var currentState = States.ReadingSize;
    var firstRead = true;  // no handshake yet
    var bytesToRead;

    // will be called when there is some data to read
    var attemptRead = function(c) {
        if (currentState == States.ReadingSize) {
            // attempt to read 16-bit size
            var r = c.read(2);
            if (!r) // 2-bytes not available yet
                return false;

            bytesToRead = (new Buffer(r)).readUInt16LE(0);
            currentState = States.ReadingData;
        }
        else {
            // attempt to read actual data
            // if bytesToRead isn't available null is returned
            // we want to wait until the entire message is available
            var raw = c.read(bytesToRead);
            if (!raw)
                return false;

            var buffer = new Buffer(raw);
            currentState = States.ReadingSize;
            if (firstRead) {
                // handshake
                firstRead = false;
                if (buffer.toString() === "READY") {
                    console.log("[INFO] Handshake Completed - ready event called")
                    if (readyFunc)
                        readyFunc({ ready: true, type: "ready" });
                }
                else {
                    console.log("[ERROR] Handshake Failed");
                    throw "Hanshake Failed";
                }
            }
            else {
                // have whole message, so now parse and notify on appmessage event
                var msg = msgParser.parse(buffer);
                if (messageFunc)
                    messageFunc({ type: "appmessage", payload: msg });
            }
        }

        return true;
    };

    console.log("[INFO] Server Connected");

    // called when there is data to read
    c.on("readable", function() {
        // keep trying while there is data to read
        while (attemptRead(c)) { }
    });
});

// start the actual server
server.listen(8321, function() {
    console.log("[INFO] Server Started");
})

// part of Pebble API
module.exports.sendAppMessage = function(msg, fnAck, fnNack) {
    var buffer = msgParser.serializeMsg(msg);

    var client = net.connect({ port: 8322 }, function() {
       client.write(buffer);

        if (fnAck)
            fnAck(msg);
    });
};

// part of Pebble API
module.exports.getAccountToken = function() {
    return "A1B2C3D4E5F6G7H8I9J0"; // obviously a dummy value
};

// mock Geolocation API
module.exports.geolocation = {
    watchPosition: function() {
        // do nothing
    },
    getCurrentPosition: function(fnSuccess, fnFailure, options) {
        fnSuccess({
            coords: {
                latitude: 40.75907,
                longitude: -73.98507
            }
        });
    }
};
