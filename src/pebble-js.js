Pebble.addEventListener("ready",
  function(e) {
    console.log("PebbleKit JS ready!");
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
    Pebble.openURL("https://dl.dropboxusercontent.com/u/46109611/Pebble/binary_circuit_color/binary_config_v1.html");
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(configuration));

    //Send to Pebble, persist there
    Pebble.sendAppMessage(
      {
        "KEY_MODE"			: configuration.mode,
				"KEY_COLOR"			: configuration.color,
				"KEY_NUMBER"		: configuration.number,
				"KEY_VIBE_H"		: configuration.vibe_h,
				"KEY_VIBE_BT"		: configuration.vibe_bt,
				"KEY_SHOW_BAT"	: configuration.show_bat
      },
      function(e) {
        console.log("Sending settings data...");
      },
      function(e) {
        console.log("Settings feedback failed!");
      }
    );
  }
);
