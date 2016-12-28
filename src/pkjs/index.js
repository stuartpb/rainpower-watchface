function pebbleSuccess(e) {
  // do nothing
}
function pebbleFailure(e) {
  console.error(e);
}

function reportPhoneBatt() {
  if (navigator.getBattery) {
    return navigator.getBattery().then(function(battery){
      Pebble.sendAppMessage({
        'PHONE_BATT_LEVEL': Math.floor(battery.level * 100),
        'PHONE_BATT_CHARGING': battery.charging ? 1 : 0
      }, pebbleSuccess, pebbleFailure);
    });
  } else {
    console.error('No navigator.getBattery');
  }
}

Pebble.addEventListener('ready', function(e) {
  reportPhoneBatt();
});
Pebble.addEventListener('appmessage', function(e) {
  console.log(e.payload);
  if (e.payload.QUERY_PHONE_BATT) {
    return reportPhoneBatt();
  }
});