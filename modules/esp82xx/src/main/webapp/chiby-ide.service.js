function Chiby() {
    this.applications = [
        { "uuid": "sensors-manager"
           , "title" : "Sensors configuration"
           , "description" : "Set names and colors for all connected sensors."
           , "avatar" : "cog"
        }
    //    ,
    //    { "uuid": "scrollbar-race"
    //       , "title" : "Race other players !"
    //       , "description" : "Fill up those scrollbars to score !"
    //       , "avatar" : "th-list"
    //    }
    ];
    this.wsListeners = [];
    // Sanity check for browser support
    if ( (! ("WebSocket" in window))  || (! window.localStorage) )
    {  // The browser doesn't support WebSocket or Local Storage
        if((! ("WebSocket" in window)) ) alert("WebSocket NOT supported by your Browser! The Chiby IDE environment will not work !");
        if( ! window.localStorage ) alert("Local Storage NOT supported by your Browser! The Chiby IDE environment will not work !");
    }
    else{ 
      // Open a Websocket connection to receive message updates
      this.openWs();
    }
}
Chiby.prototype.openWs = function(){
    var that = this;
      
    this.ws = new WebSocket("ws://" + location.host + "/ws");
    this.ws.onopen = function(){
        console.log("WS Connection is open...");
     };
    
    this.ws.onmessage = function (evt){
       $.each(that.wsListeners, function(index, listener){
            if(typeof listener.onMessage === "function"){
                listener.onMessage(evt.data);
            }
       });
     };
         
     this.ws.onclose = function(){
        // websocket is closed.
        console.log("WS Connection closed...");
        // Attempt to reopen
        setTimeout(that.openWs, 1000);
     };
     
     window.onbeforeunload = function(event) {
        that.ws.close();
     };
}

Chiby.prototype.showHome = function() {
	$("#panel-sensors-manager").collapse("hide");
	$("#panel-scrollbar-race").collapse("hide");
	$("#panel-home").collapse("show");
}

Chiby.prototype.refreshHome = function() {
	var that = this;
	$("#applications-grid").empty();
	$.each(that.applications,
            function(index, app) {
                var appItem = $('<div id="'
                        + app.uuid
                        + '" class="grid-item"></div>');
                var innerPanel = $('<div class="panel panel-primary" style="height:280px"></div>');
                var innerPanelHeader = $('<div class="panel-heading"></div>');
                innerPanelHeader.text(app.title);
                innerPanel.append(innerPanelHeader);
                var avatar = 'question-sign';
                if (app.avatar != null)
                    avatar = app.avatar;
                var span = $('<div style="height:180px; padding:5px"><span style="font-size:8em; margin-top: 10px; color: #111111; display:block; text-align:center" class="glyphicon glyphicon-'
                        + avatar
                        + '"></span><div style="padding:5px">'
                        + app.description
                        + '</div></div>');
                innerPanel.append(span);
                var innerPanelFooter = $('<div class="panel-footer text-right" style="height:60px"></div>');
                var editButton = $('<button type="button" class="btn btn-primary pull-right" style="margin:3px" onclick="chiby.openApplication($(this).data(\'app\'))">Open</button>');
                editButton.data('app', app);
                innerPanelFooter.append(editButton);
                innerPanel.append(innerPanelFooter);
                appItem.append(innerPanel);
                $("#applications-grid").packery()
                        .append(appItem)
                        .packery('appended',
                                appItem);
        });
}

Chiby.prototype.openApplication = function(app) {
	$("#panel-home").collapse("hide");
	$("#panel-"+app.uuid).collapse("show");
}

Chiby.prototype.addWsListener= function(wsListener){
    this.wsListeners.push(wsListener);
}

Chiby.prototype.removeWsListener = function(wsListener){
    this.wsListeners = this.wsListeners.filter(function(e) { return e !== wsListener });
}
chiby = new Chiby();
