/* global Pebble navigator */
function pebbleSuccess(e) {
  // do nothing
}
function pebbleFailure(e) {
  console.error(e);
}

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
      battery.addEventListener('levelchange', function() {
        console.log('Level change: '+ (battery.level * 100) +'%');
        reportPhoneBatt();
      });
      battery.addEventListener('chargingchange', reportPhoneBatt);
    });
  } else {
    console.error('No navigator.getBattery');
    console.error('User agent: '+navigator.userAgent);
  }
});
Pebble.addEventListener('appmessage', function(e) {
  if (e.payload.QUERY_PHONE_BATT && reportPhoneBatt) {
    return reportPhoneBatt();
  }
});