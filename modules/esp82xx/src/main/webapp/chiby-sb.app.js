function ScrollbarRaceApp(){
  var that = this;
  // Retrieve all known sensors from local storage and add them to the grid
  $.each(chibySensorManager.getSensors(), function(id, sensor){
      that.addedSensor(sensor);
  });
  
  chibySensorManager.addSensorManagementListener(this);
  chiby.addWsListener(this);
}

ScrollbarRaceApp.prototype.addedSensor = function(sensor){
    var item = $('<div></div>');
    var innerPanel = $('<div class="panel"></div>');
    // 
    var span = $('<div class="row"><div class="col-sm-1"><span>'+sensor.initials
        +'</span></div>'
        +'<div class="col-sm-11"><div class="progress"><div class="progress-bar progress-bar-success progress-bar-striped" role="progressbar" aria-valuenow="40" aria-valuemin="0" aria-valuemax="100" style="width: 40%">   <span class="sr-only">40% Complete (success)</span> </div> </div>'
        +'</div>');
    innerPanel.append(span);
    item.append(innerPanel);
    $('#scrollbar-race-grid').append(item);
}

ScrollbarRaceApp.prototype.onMessage = function(msg){
  var data = JSON.parse(msg);
  // Find the sensor ID
  m = /^\/chibit\/([0-9]+)\/sensor\/bend/.exec(data.topic);
  
  //var sensorId = m[1];
  // update the sensor box shadow with incoming value
 
}
scrollbarRaceApp = new ScrollbarRaceApp();