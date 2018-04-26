function SensorManagerApp(){
  var that = this;
  // Retrieve all known sensors from local storage and add them to the grid
  $.each(chibySensorManager.getSensors(), function(id, sensor){
      that.addedSensor(sensor);
  });
  
  chibySensorManager.addSensorManagementListener(this);
  chiby.addWsListener(this);
}

SensorManagerApp.prototype.addedSensor = function(sensor){
    var item = $('<div class="grid-sensor-item"></div>');
    var innerPanel = $('<div class="panel" style="height:140px"></div>');
    // 
    var span = $('<div style="height:120px; padding:5px; margin-left:15px"><div id="sensor-'+ sensor.id
                        + '" class="avatar-circle" style="background-color:'+sensor.color+'"><span class="initials"><i class="glyphicon glyphicon-'
            + sensor.avatar
            + '"></i></span></div><div style="padding:5px; text-align:center">'
            + sensor.name
            + '</div></div>');
    innerPanel.append(span);
    item.append(innerPanel);
    $("#sensors-grid").packery()
            .append(item)
            .packery('appended',
                    item);
}

SensorManagerApp.prototype.onMessage = function(msg){
  var data = JSON.parse(msg);
  // Find the sensor ID
  m = /^\/chibit\/([0-9]+)\/bend/.exec(data.topic);
  
  if(m.length > 0){
    var sensorId = m[1];
    // update the sensor box shadow with incoming value
    if(data.value < 50){
      $("#sensor-"+sensorId).css("box-shadow", "0 0 1pt 2pt green");
    }else if(data.value < 100){
      $("#sensor-"+sensorId).css("box-shadow", "0 0 1pt 4pt blue");
    }else if(data.value < 200){
      $("#sensor-"+sensorId).css("box-shadow", "0 0 1pt 6pt orange");
    }else{
      $("#sensor-"+sensorId).css("box-shadow", "0 0 1pt 8pt red");
    }
  }
  
}
sensorManagerApp = new SensorManagerApp();