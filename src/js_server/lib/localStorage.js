var fs = require('fs');
var _ = require('lodash');

var storage_to_save = []; // used to store info for saving later

// helper function to pull out all data keys (which happen to
// be all strings)
function getDataKeys(storage) {
    return _.filter(_.keys(storage), function(k) { 
	return (typeof(storage[k]) === 'string'); 
    });
}

// Storage class
// takes optional filename for backing store
// if left blank storage will not persist
var Storage = function(filename) {
    var that = this;

    // compute length on the fly because of direct property access
    Object.defineProperty(this, "length", {
	get: function() {
	    return getDataKeys(this).length;
	},
	set: function() { } // do nothing
    });

    // load
    if (filename) {
	try {
	    var raw = fs.readFileSync(filename);
	    if (raw)
		_.defaults(this, JSON.parse(raw));
	}
	catch(e) {
	    // swallow error if can't read
	}

	// add to list to save
	storage_to_save.push({ file: filename, storage: this });
    }
};

Storage.prototype.setItem = function(key, value) {
    this[key.toString()] = value.toString();
};

Storage.prototype.getItem = function(key) {
    var skey = key.toString();

    if (!this[skey])
	return null;

    return this[skey];
};

Storage.prototype.removeItem = function(key) {
    var skey = key.toString();

    if (!this[skey])
	return;
    if (typeof(this[skey]) !== 'string')
	return;

    delete this[skey];
};

Storage.prototype.clear = function() {
    _.forEach(getDataKeys(this), function(k) {
	delete this[k];
    }, this);
};

Storage.prototype.key = function(n) {
    if (typeof(n) !== 'number')
	return null;

    if (n < 0)
	return null;
    
    var skeys = getDataKeys(this);
    if (n >= skeys.length)
	return null;
 
    return skeys[n];
};

function saveStorageItem(sitem) {
    // prepare data
    var dataKeys = getDataKeys(sitem.storage);
    var data = { };
    _.forEach(dataKeys, function(k) {
	data[k] = sitem.storage.getItem(k);
    });

    // actual write
    try {
	fs.writeFileSync(sitem.file, JSON.stringify(data));
    }
    catch (e) {
	console.log("[ERROR] Error Saving localStorage (" + sitem.file + "): " + e);
    }
}

// function to force save
// (meant to be used for testing)
module.exports._forceSave = function(storage) {
    var sitem = _.where(storage_to_save, { 'storage': storage });
    if (!sitem || !sitem.length)
	throw "Can't find Storage in storage_to_save list";
    saveStorageItem(sitem[0]);
};

// save on exit
process.on('exit', function() {
    _.forEach(storage_to_save, saveStorageItem);
});

// save on kill
process.on('SIGTERM', function() {
    _.forEach(storage_to_save, saveStorageItem);
    process.exit(0);
});

module.exports.Storage = Storage;