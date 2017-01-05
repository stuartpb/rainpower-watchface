/* global Pebble navigator localStorage fetch */

// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config.js');
// Initialize Clay
var clay = new Clay(clayConfig, null, {autoHandleEvents: false});

var Promise = Promise || require('promise-polyfill');
require('whatwg-fetch');

function sendAppMessagePromised(message) {
  return new Promise(function(resolve, reject) {
    return Pebble.sendAppMessage(message, resolve, reject);
  });
}

// TODO: incorporate one-at-a-time here
// (which will probably entail controlling the first call, too)
// https://github.com/stuartpb/rainpower-watchface/issues/11
function every5Minutes(cb) {
  var timer;
  function begin() {
    timer = setInterval(cb, 3e5);
    cb();
  }
  // start at the next 5-minute interval
  timer = setTimeout(begin, 3e5 - Date.now() % 3e5);
  return function() {
    // This is valid for clearing setTimeout as well, per the HTML spec
    // http://stackoverflow.com/a/27369447/34799
    clearInterval(timer);
  };
}

// See webviewclosed handler below
var darkSkyApiKey = localStorage.getItem('DARK_SKY_API_KEY');
var darkSkyRequestUnits = localStorage.getItem('DARK_SKY_REQUEST_UNITS');

var clearWeatherInterval;

var reportPhoneBatt;

function getDarkSkyApiUrlForHere() {
  return new Promise(function(resolve, reject) {
    function geolocationSuccess(position) {
      if (darkSkyApiKey) {
        resolve('https://api.darksky.net/forecast/' + darkSkyApiKey + '/' +
          position.coords.latitude + ',' + position.coords.longitude);

      // This *shouldn't* happen, but it could theoretically happen
      // if someone reconfigured the watchface within the moment between
      // getCurrentPosition and the location response being returned
      // (see note about cancelling in-flight requests in initWeather)
      } else {
        reject(new Error('Hotshot user destroyed API access mid-positioning'));
      }
    }
    if (navigator.geolocation) {
      navigator.geolocation.getCurrentPosition(geolocationSuccess,
        // TODO: handle geolocation error with fallback
        reject,
        {
          // TODO: make maximum age configurable
          maximumAge: Infinity,
          // TODO: figure out what sensible timeout values even look like
          // https://github.com/stuartpb/rainpower-watchface/issues/11
          timeout: 3e5
        });
    } else {
      // TODO: Fallback location
      reject(new Error('no navigator.geolocation'));
    }
  });
}

function precipIntPercent(point) {
  return Math.round(point.precipProbability * 100);
}

function requestAndReportWeather() {
  return getDarkSkyApiUrlForHere().then(function(url) {
    return fetch(url).then(function(res){return res.json();});
  }).then(function(response) {
    return sendAppMessagePromised({
      'CURRENT_TEMPERATURE': Math.round(response.currently.temperature),
      'CURRENT_TEMPERATURE_IS_FAHRENHEIT': response.flags.units == 'us',
      'PRECIP_PROBABILITY_NEXT_60_MINUTES':
        response.minutely.data.map(precipIntPercent),
      'PRECIP_PROBABILITY_NEXT_48_HOURS':
        response.hourly.data.map(precipIntPercent)
    });
  }).catch(console.error.bind(console));
}

function initWeather() {
  if (darkSkyApiKey) {
    requestAndReportWeather().then(function(){
      clearWeatherInterval = every5Minutes(requestAndReportWeather);
    });

  // initWeather is also responsible for de-initting weather
  } else if (clearWeatherInterval) {
    // Any weather request in progress will just have to reject accordingly,
    // on its own timetable, due to cancellation being impossible
    clearWeatherInterval();
  }
}

Pebble.addEventListener('ready', function(e) {
  if (navigator.getBattery) {
    navigator.getBattery().then(function (battery) {
      reportPhoneBatt = function () {
        return sendAppMessagePromised({
          'PHONE_BATT_LEVEL': Math.floor(battery.level * 100),
          'PHONE_BATT_CHARGING': battery.charging ? 1 : 0
        });
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
  initWeather();
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

  darkSkyApiKey = dict.DARK_SKY_API_KEY.value;
  localStorage.setItem('DARK_SKY_API_KEY', darkSkyApiKey);
  darkSkyRequestUnits = dict.DARK_SKY_REQUEST_UNITS.value;
  localStorage.setItem('DARK_SKY_REQUEST_UNITS', darkSkyRequestUnits);

  // reflect this re-configuration
  initWeather();
});
