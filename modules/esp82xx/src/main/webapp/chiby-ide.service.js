function Chiby() {
    this.applications = [
        { "uuid": "hello-world"
           , "title" : "Hello World"
           , "description" : "Just a quick test application"
           , "avatar" : "asterisk"
        }
    ];
    
    this.wsListeners = [];
    // Sanity check - are websockets supported by this browser ?
    if (! ("WebSocket" in window) )
    {  // The browser doesn't support WebSocket
        alert("WebSocket NOT supported by your Browser! The Chiby IDE environment will not work !");
    }
    else{ 
      // Open a Websocket connection to receive message updates
      var that = this;
      
      this.ws = new WebSocket("ws://" + location.host + "/ws");
      this.ws.onopen = function(){
          console.log("WS Connection is open...");
       };
           
      this.ws.onmessage = function (evt){
         // console.log("Message received :", evt.data);
         $.each(that.wsListeners, function(index, listener){
              if(listener.onMessage){
                  listener.onMessage(evt.data);
              }
         });
       };
           
       this.ws.onclose = function(){
          // websocket is closed.
          console.log("WS Connection closed...");
       };
       
       window.onbeforeunload = function(event) {
          that.ws.close();
       };
    }
}

Chiby.prototype.showHome = function() {
	$("#panel-program").collapse("hide");
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
                //var deleteButton = $('<button type="button" class="btn btn-danger pull-left" style="margin:3px" onclick="chiby.deleteApplication($(this).data(\'app\'))">Delete</button>');
                //deleteButton.data('app', app);
                innerPanelFooter.append(editButton);
                //innerPanelFooter
                //        .append(deleteButton);
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
	$("#panel-program").collapse("show");
}

Chiby.prototype.addWsListener= function(wsListener){
    this.wsListeners.push(wsListener);
}

Chiby.prototype.removeWsListener = function(wsListener){
    this.wsListeners = this.wsListeners.filter(function(e) { return e !== wsListener });
}
chiby = new Chiby();