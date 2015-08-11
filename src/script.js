var dict = {};

Pebble.addEventListener('ready',
  function(e) {
    console.log('JavaScript app ready and running!');
  }
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('http://albertyau.github.io/pebble-classy-colors-config');
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  
  dict = {};
  var backgroundColor = configData.bgColor;
  dict.KEY_BGCOLOR_R = parseInt(backgroundColor.substring(2, 4), 16);
  dict.KEY_BGCOLOR_G = parseInt(backgroundColor.substring(4, 6), 16);
  dict.KEY_BGCOLOR_B = parseInt(backgroundColor.substring(6), 16);

  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
  
});