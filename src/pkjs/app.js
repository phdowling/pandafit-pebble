var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function scoreToImageId(score){
  var id = null;
  if (80 < score){ // case ecstatic
    id = 0;
  } else if (60 < score < 80){ // case happy
    id = 1;
  }else if (40 < score < 60){ // case content
    id = 2;
  }else if (20 < score < 40){ // case angry
    id = 3;
  }else if (score < 20){ // case dying
    id = 4;
  }
}

 
function getUserScore(pos) {
  // Construct URL
  var url = "http://our-server"; // TODO
 
  // Send request to Backend
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      var score = json.score;
      // calc image ID from score
      var imageId = scoreToImageId(score);
      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_IMAGE": imageId,
      };
 
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function selectRandomImage(){
  var seconds = Math.round(new Date().getTime() / 1000);
  var imageId = seconds % 3;
  Pebble.sendAppMessage({
    "KEY_IMAGE": imageId
  }, function(e) {
    console.log("Image ID " + imageId+ " sent to Pebble!");
  },function(e) {
    console.log("Error sending imageId to Pebble!");
  });
  
}
 
// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
 
    // Get the initial weather
    selectRandomImage();
  }
);
 
// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    selectRandomImage();
  }                     
);
