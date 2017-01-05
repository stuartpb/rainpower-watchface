/* global Pebble navigator localStorage */

// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config.js');
// Initialize Clay
var clay = new Clay(clayConfig, null, {autoHandleEvents: false});

function pebbleSuccess(e) {
  // do nothing
}
function pebbleFailure(e) {
  console.error(e);
}

// See webviewclosed handler below
var darkSkyApiKey = localStorage.getItem('DARK_SKY_API_KEY');
var darkSkyRequestUnits = localStorage.getItem('DARK_SKY_REQUEST_UNITS');

var reportPhoneBatt;

Pebble.addEventListener('ready', function(e) {
  if (navigator.getBattery) {
    navigator.getBattery().then(function (battery) {
      reportPhoneBatt = function () {
        Pebble.sendAppMessage({
          'PHONE_BATT_LEVEL': Math.floor(battery.level * 100),
          'PHONE_BATT_CHARGING': battery.charging ? 1 : 0
        }, pebbleSuccess, pebbleFailure);
      };
      battery.addEventListener('levelchange', reportPhoneBatt);
      battery.addEventListener('chargingchange', reportPhoneBatt);
      reportPhoneBatt();
    });
  } else if (navigator.userAgent) {
    console.error('No navigator.getBattery');
    console.error('User agent: '+navigator.userAgent);
  } else {
    console.log('No navigator.userAgent, probably running in emulator');
  }
});
Pebble.addEventListener('appmessage', function(e) {
  if (e.payload.QUERY_PHONE_BATT && reportPhoneBatt) {
    return reportPhoneBatt();
  }
});

// Handle Clay events manually
Pebble.addEventListener('showConfiguration', function(e) {
  // our showConfiguration looks basically normal
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {

  if (e && !e.response) {
    return;
  }

  // Get the keys and values from each config item
  var dict = clay.getSettings(e.response, false);

  // Since most (all, at time of writing) or our config options affect the
  // behavior of the JS component and not the watch, we don't send an
  // appMessage to the watch with them the way the default handler does.
  // Instead, we update our local variables reflecting them, and serialize
  // them out to their own localStorage options.
  // This is admittedly redundant to what Clay does with 'em, but /shrug

  darkSkyApiKey = dict.DARK_SKY_API_KEY;
  localStorage.setItem('DARK_SKY_API_KEY', darkSkyApiKey);
  darkSkyRequestUnits = dict.DARK_SKY_REQUEST_UNITS;
  localStorage.setItem('DARK_SKY_REQUEST_UNITS', darkSkyRequestUnits);
});
