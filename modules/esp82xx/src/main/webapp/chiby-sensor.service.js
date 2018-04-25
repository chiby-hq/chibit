function ChibySensorsManager(){
  this.listeners = [];
  
  this.sensors = {};
  // Rebuild sensors list from localStorage
  if(localStorage.getItem("sensors")) {
      this.sensors = JSON.parse(localStorage.getItem("sensors"));
  }
  // Mark all previously known sensors as inactive
  $.each(this.sensors, function(id, sensor){
       sensor.active = false;
  });
  
}

ChibySensorsManager.prototype.onMessage = function(msg){
      var data = JSON.parse(msg);
      m = /^\/chibit\/([0-9]+)/.exec(data.topic);
      if(m.length > 0){
          this.addSensor(m[1]);
      }
  }

ChibySensorsManager.prototype.addSensor = function(id){
    if(this.sensors[id]){
        if(! (this.sensors[id].active)){
            this.sensors[id].active = true;
            var that = this;
            $.each(this.listeners, function(index, listener){
                    if(typeof listener.activatedSensor=== "function") listener.activatedSensor(that.sensors[id]);
            });
        }
    }else{
        this.sensors[id] = { id: id, name: id, initials: "?", color: "#999999", avatar: 'question-sign', active: true};
        var that = this;
        $.each(this.listeners, function(index, listener){
           if(typeof listener.addedSensor=== "function") listener.addedSensor(that.sensors[id]);
        });
    }
    localStorage.setItem("sensors", JSON.stringify(this.sensors));
}

ChibySensorsManager.prototype.getSensors = function(){
    return this.sensors;
}

ChibySensorsManager.prototype.updateSensor = function(id, name = null, color = null, avatar = null){
  this.addSensor(id);
  if(name != null){ this.sensors[id].name = name };
  if(color != null){ this.sensors[id].color = color };
  if(avatar != null){ this.sensors[id].avatar = avatar };
  var that = this;
  $.each(this.listeners, function(index, listener){
     if(typeof listener.updatedSensor == "function") listener.updatedSensor(that.sensors[id]);
  });
  localStorage.setItem("sensors", JSON.stringify(this.sensors));
}

ChibySensorsManager.prototype.disconnectSensor = function(id){
    if(this.sensors[id]){
        this.sensors[id].active = false;
        var that = this;
        $.each(this.listeners, function(index, listener){
           if(typeof listener.disconnectedSensor === "function") listener.disconnectedSensor(that.sensors[id]);
        });
    }
}

ChibySensorsManager.prototype.addSensorManagementListener= function(listener){
    this.listeners.push(listener);
}

ChibySensorsManager.prototype.removeSensorManagementListener = function(listener){
    this.listeners = this.listeners.filter(function(e) { return e !== listener });
}

chibySensorManager = new ChibySensorsManager();