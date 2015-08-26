//=====================================================================================
//
//	Created Dec, 2013
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	2dViewer.js
//
//	requires an omni div, that will be made full window size (window.innerWidth and 
//		window.innerHeight) at position 0,0
//
//	public function list:
//		2dViewer.launch()
//
//=====================================================================================

var 2dViewer = 2dViewer || {}; //define namespace


////////////////////////////////////////////////////////////
//public function definitions

// 2dViewer.launch ~~
//		called to start 2d viewer
2dViewer.launch = function() {
	
	Debug.log("2dViewer.launch");
	
	2dViewer.omni = document.getElementById("omni");
	2dViewer.omni.innerHTML = ""; //clear all content
	
	//initialize with loading message in center
    var w = 2dViewer.w = window.innerWidth;
    var h = 2dViewer.h = window.innerHeight;

	2dViewer.omni.style.position = "absolute";
    2dViewer.omni.style.left = 0  + "px";
    2dViewer.omni.style.top = 0  + "px";
	2dViewer.omni.style.width = w + "px";
	2dViewer.omni.style.height = h + "px";
	2dViewer.omni.style.backgroundColor = "rgb(30,30,30)";
	
	2dViewer.omni.innerHTML = "<center><div style='margin-top:" + (h/2-8) + "px'>Loading 2-D...</div></center>";
	
	
	//load/insert libraries and setup canvas
	// steps: 
	//		- setup canvas behind "loading..."
	//		- setup hud
	//		- get default configuration from server
	//		- get default geometry from server
	//		- bring canvas in front of "loading..."
	
	2dViewer.initCanvas();
}

//end public function definitions
////////////////////////////////////////////////////////////


2dViewer.CANVAS_COLOR = "rgb(102,0,51)";
2dViewer.CANVAS_MIN_SIZE = 300; //dont allow w or h to go less than this
2dViewer.HUD_WIDTH = 200;
2dViewer.HUD_MARGIN_RIGHT = 10;
2dViewer.HUD_DROP_DOWN_SPEED = 10;
2dViewer.TICK_REFRESH_PERIOD = 30; //ms

2dViewer.omni;
2dViewer.hud;
2dViewer.canvas;
2dViewer.context;
2dViewer.w;
2dViewer.h;
2dViewer.tickTimer;

2dViewer.drawGridFlag = true;
2dViewer.globalAccumulate = true;



//station consists one or two planes, only can be two planes if they are strip planes
//	[station 0, (optional station 1)]

//a plane consists of strips or pixels
//	[cols, rows, cell width, cell height]  //microns

2dViewer.stations = [];


//"private" function list
//	2dViewer.initCanvas
//	2dViewer.handleWindowResize
//	2dViewer.createHud
//	2dViewer.redraw
//	2dViewer.getConfiguration
//	2dViewer.getConfigurationHandler


2dViewer.initCanvas = function() {
	
	2dViewer.canvas = document.createElement('canvas');  
	2dViewer.canvas.style.position = "absolute";
    2dViewer.canvas.style.left = 0  + "px";
    2dViewer.canvas.style.top = 0  + "px";
	2dViewer.canvas.style.zIndex = -1;  //-1 is not visible.. change to 0 to bring to front	
	2dViewer.canvas.style.backgroundColor = 2dViewer.CANVAS_COLOR;
	2dViewer.context = 2dViewer.canvas.getContext('2d');
	
	2dViewer.omni.appendChild(2dViewer.canvas);
	
	2dViewer.hud = new 2dViewer.createHud();
	
	window.onresize = 2dViewer.handleWindowResize;
	2dViewer.handleWindowResize();
	
	2dViewer.getConfiguration();
	
	//if(2dViewer.tickTimer) clearInterval(2dViewer.tickTimer);
	//2dViewer.tickTimer = setInterval(2dViewer.tick, 2dViewer.TICK_REFRESH_PERIOD);
}

//2dViewer.tick ~~
//	periodically check for data
2dViewer.tick = function() {					 
	2dViewer.redraw();
}

2dViewer.handleWindowResize = function() {
	
	var w = window.innerWidth < 2dViewer.CANVAS_MIN_SIZE? 2dViewer.CANVAS_MIN_SIZE:window.innerWidth;
	var h = window.innerHeight < 2dViewer.CANVAS_MIN_SIZE? 2dViewer.CANVAS_MIN_SIZE:window.innerHeight;

	Debug.log("2dViewer.handleWindowResize " + w + "-" + h);
	
	2dViewer.omni.style.width = w + "px";
	2dViewer.omni.style.height = h + "px";
	2dViewer.canvas.style.width = w + "px";
	2dViewer.canvas.style.height = h + "px";
	2dViewer.canvas.width = w;
	2dViewer.canvas.height = h;

	//reposition HUD
	2dViewer.hud.handleWindowResize();
	
	//redraw 2d world
	2dViewer.redraw();
}

2dViewer.createHud = function() {
		
	var hudMouseOverDiv;
	var animationTargetTop, isDropDownAnimating, isDropDownDown;
	var controlMouseIsDown = false;
	var controlMouseTimeout = 0;
	var getEventsTimeout = 0;
	
	this.isInMotion = function() { return isDropDownAnimating; }
	
	this.handleWindowResize = function() {
		Debug.log("2dViewer Hud handleWindowResize");
		this.hudMouseOverDiv.style.left = window.innerWidth - this.hudMouseOverDiv.offsetWidth - 2dViewer.HUD_MARGIN_RIGHT + "px";
		
		this.hudNavSpeedDiv.style.left = 5 + "px";
		this.hudNavSpeedDiv.style.top = window.innerHeight - 95 + "px";			
	}
	
	this.checkboxUpdate = function(i) {
		var chk = document.getElementById("hudCheckbox" + i);
		Debug.log("2dViewer Hud checkboxUpdate " + i + "=" + chk.checked);
		
		if(i==0) 2dViewer.drawGridFlag = chk.checked; //grid 
		else if(i==1) 2dViewer.globalAccumulate = chk.checked; //globalAccumulate 
				
		2dViewer.redraw();
	}	
	
	// animateDropDown ~~
	var animateDropDown = function() {
		var dir = (animationTargetTop - hudMouseOverDiv.offsetTop > 0)? 1: -1;
		
		var tmpTop = hudMouseOverDiv.offsetTop + dir*2dViewer.HUD_DROP_DOWN_SPEED;
		if(Math.abs(tmpTop - animationTargetTop) <= 2dViewer.HUD_DROP_DOWN_SPEED) //done
		{
			hudMouseOverDiv.style.top = animationTargetTop + "px";
			isDropDownAnimating = false;
			return;
		}
		//else still going towards target
		hudMouseOverDiv.style.top = tmpTop + "px";
		window.setTimeout(animateDropDown,30);
	}

	// mouseOverDropDown ~~
	var mouseOverDropDown = this.mouseOverDropDown = function() {
		if(isDropDownAnimating) return; //do nothing if animating currently
		
		if(!isDropDownDown) //start animation
		{
			isDropDownDown = true;
			isDropDownAnimating = true;
			animationTargetTop = -15;
			window.setTimeout(animateDropDown,30);
		}		
	}
	
	// mouseOutDropDown ~~
	var mouseOutDropDown = this.mouseOutDropDown = function(event) {
		if(isDropDownAnimating) return; //do nothing if animating currently
		
		if(event)
		{
			var e = event.toElement || event.relatedTarget;			
			while(e)	//if moving within drop down menu ignore
			{
				if(e == this) return;
				e = e.parentNode;
			} 
		}
		
		if(isDropDownDown) //start animation
		{
			isDropDownDown = false;
			isDropDownAnimating = true;
			animationTargetTop = 15 - hudMouseOverDiv.offsetHeight;
			window.setTimeout(animateDropDown,30);
		}
	}
	
	//for event and run number buttons
	this.handleControlDown = function(i,evt) {		
		//Debug.log("hud control down " + i);
		
		var to = 500;
		if(controlMouseIsDown) //already mouse down, so do action
		{
			to = 10;
			doControlAction(i);			
		}
		else if(evt)
			controlMouseIsDown = true;
		
		if(controlMouseIsDown)
		{
			if(controlMouseTimeout) window.clearTimeout(controlMouseTimeout);
			controlMouseTimeout = window.setTimeout(function() {2dViewer.hud.handleControlDown(i)},to);
		}
	}

	//for event and run number buttons
	this.handleControlUp = function(i) {		
		if(!controlMouseIsDown) return;
		
		//Debug.log("hud control up " + i);
		
		if(controlMouseTimeout) window.clearTimeout(controlMouseTimeout);
		controlMouseIsDown = false;
		doControlAction(i);		//do action on mouse up
	}

	//for event and run number buttons
	var doControlAction = function(i) {
		switch(i)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			if(i!=1 && i!=2)
				2dViewer.eventToDisplay += (i==0?-1:1)*parseInt(2dViewer.events.length/5+1);
			else
				2dViewer.eventToDisplay += (i==1?-1:1);	
			
			if(2dViewer.eventToDisplay < 0)
				2dViewer.eventToDisplay = 0;
			if(2dViewer.eventToDisplay >= 2dViewer.events.length)
				2dViewer.eventToDisplay = 2dViewer.events.length - 1;
			break;
		case 4: //dec run #
		case 5: //inc run #
			2dViewer.hud.hudRunNumberInput.value = parseInt(2dViewer.hud.hudRunNumberInput.value) + (i*2-9);
			if(2dViewer.hud.hudRunNumberInput.value < 0) 2dViewer.hud.hudRunNumberInput.value = 0;
			if(getEventsTimeout) window.clearTimeout(getEventsTimeout);
			getEventsTimeout = window.setTimeout(2dViewer.getEvents,1000);
			break;
		default:
			Debug.log("hud doControlAction unknown action" + i);	
			return;
		}
				
		2dViewer.hud.update();	
	}

	//only allow positive integers, remove all non numbers and parse as int
	this.handleRunChange = function() {	
		//Debug.log("hud handleRunChange " );
		var s = this.hudRunNumberInput.value;
		for(var i=0;i<s.length;++i)
		{
			if(!(parseInt(s[i])+1)) //remove non numbers
			{
				s = s.substr(0,i) + s.substr(i+1);
				--i; //rewind
			}
		}
		s = parseInt(s); 
		if(s+"" != this.hudRunNumberInput.value) //string comparison
		{
			this.hudRunNumberInput.value = s;
			Debug.log("hud handleRunChange " + s);
		}
	}
	
	
	this.hudNavSpeedDiv = document.createElement('div');	
	this.hudNavSpeedDiv.setAttribute("id", "2dViewer-hudNavSpeedOverlay");
	this.hudNavSpeedDiv.style.position = "absolute";	
	this.hudNavSpeedDiv.style.zIndex = 100;
	2dViewer.omni.appendChild(this.hudNavSpeedDiv);
	
	
	hudMouseOverDiv = this.hudMouseOverDiv = document.createElement('div');	
	hudMouseOverDiv.setAttribute("id", "2dViewer-hudMouseOver");
	hudMouseOverDiv.style.position = "absolute";	
    hudMouseOverDiv.style.zIndex = 100;
	
	this.hudDiv = document.createElement('div');	
	this.hudDiv.setAttribute("id", "2dViewer-hud");
	var chkLabels = ["Show Axes","Show Grid","Show Tracks", "Mouse Nav","Fly-By","Accumulated"];
	var chkDefaults = ["checked","checked","checked","checked",2dViewer.enableFlyBy?"checked":"",2dViewer.accumulateEvents?"checked":""];
	var str = "";
	for(var i=0;i<chkLabels.length;++i)
		str += "<input type='checkbox' id='hudCheckbox" + i + "' onchange='2dViewer.hud.checkboxUpdate(" + i + 
					");' " + chkDefaults[i] + "><label for='hudCheckbox" + i + "'>" + chkLabels[i] + "</label><br>";
	
	//add event controls
	str += "<center><div id='2dViewer-hudEventNumberControls'><div id='2dViewer-hudEventNumber'></div>";
	var evtNumBtns = ["<<","<",">",">>"];
	for(var i=0;i<evtNumBtns.length;++i)		
		str += "<input type='button' onmousedown='2dViewer.hud.handleControlDown(" + i + ",event);' " +
			"onmouseup='2dViewer.hud.handleControlUp(" + i + ");' " +
			"onmouseout='2dViewer.hud.handleControlUp(" + i + ");' " +
			"value='" + evtNumBtns[i] + "' />";	
	str += "</div></center>";
	
	//add run controls
	if(DesktopContent._needToLoginMailbox) //only add if successful login
	{
		str += "<div id='2dViewer-hudRunNumberControls'>Run #: " +
				"<input id='2dViewer-hudRunNumberControls-textInput' oninput='2dViewer.hud.handleRunChange();' type='text' value='40' > ";
		evtNumBtns = ["<",">"];
		for(var i=0;i<evtNumBtns.length;++i)		
			str += "<input type='button' onmousedown='2dViewer.hud.handleControlDown(" + (i+4) + ",event);' " +
				"onmouseup='2dViewer.hud.handleControlUp(" + (i+4) + ");' " +
				"onmouseout='2dViewer.hud.handleControlUp(" + (i+4) + ");' " +
				"value='" + evtNumBtns[i] + "' />";	
		str += "</div>";
	}
	else
	{
		str += "<input id='2dViewer-hudRunNumberControls-textInput' type='hidden' value='40' >";
		
		this.hudUrlOverlay = document.createElement('div');	
		this.hudUrlOverlay.setAttribute("id", "2dViewer-hudUrlOverlay");
		this.hudUrlOverlay.style.position = "absolute";	
		this.hudUrlOverlay.style.zIndex = 100;
		this.hudUrlOverlay.style.left = 5 + "px";
		this.hudUrlOverlay.style.top = 5 + "px";
		this.hudUrlOverlay.innerHTML = "Try on your own mobile device!<br>http://tinyurl.com/q6lhdrm";
		2dViewer.omni.appendChild(this.hudUrlOverlay);
	}
	
	this.hudDiv.innerHTML = str;
	
//	_historyEl.onmousewheel = handleUserInputScroll;
//	_historyEl.onmousedown = handleUserInputScroll;
//	this.hudEventScrollbar.onscroll = this.handleEventScroll;
//	_historyEl.onmousemove = handleMouseMoveScroll;
   
    hudMouseOverDiv.appendChild(this.hudDiv);  
    
    hudMouseOverDiv.style.top = hudMouseOverDiv.offsetHeight - 15 + "px";
    hudMouseOverDiv.style.width = 2dViewer.HUD_WIDTH + "px";
    hudMouseOverDiv.onmouseover = mouseOverDropDown;
    hudMouseOverDiv.onmouseout = mouseOutDropDown;
	2dViewer.omni.appendChild(hudMouseOverDiv);	
	
	this.hudEventNumber = document.getElementById('2dViewer-hudEventNumber'); //get reference
    this.hudEventNumberControls = document.getElementById('2dViewer-hudEventNumberControls'); //get reference
    this.hudRunNumberInput = document.getElementById('2dViewer-hudRunNumberControls-textInput'); //get reference
    
	//setup dropdown effect
	isDropDownDown = false;
	isDropDownAnimating = true;
	animationTargetTop = 15 - hudMouseOverDiv.offsetHeight;
	window.setTimeout(animateDropDown,30);
	this.handleWindowResize();
}


2dViewer.getConfiguration = function() {
	Debug.log("2dViewer getConfiguration ");
	//DesktopContent.XMLHttpRequest("request?RequestType=get2dConfiguration", "", 2dViewer.getConfigurationHandler);
	2dViewer.getConfigurationHandler();
}

// 2dViewer.getConfigurationHandler ~~
2dViewer.getConfigurationHandler = function (req) {
	Debug.log("2dViewer getConfigurationHandler " );
	

	2dViewer.stations= [];
	
	var cnt = 0;
	stations[cnt++] = [[1024, 1, 60, 90000]];
	stations[cnt++] = [[1024, 1, 60, 90000],[512, 1, 60, 90000]];
	                   
	//var objects = req.responseXML.getElementsByTagName("object");
	
//	2dViewer.geometry = [];
//	var gi;
//	var gl = 2dViewer.gl;
//	var locPoints, xyzPoints, objectType;
//	for(var i=0;i<objects.length;++i)
//	{
//		//TODO if different geometries, use object type
//		//objectType = objects[i].getElementsByTagName("object_type");
//	    //Debug.log("Rotating3d getGeometry objectType " + objectType[0].getAttribute("value"));
//		
//		xyzPoints = objects[i].getElementsByTagName("xyz_point");
//	
//		locPoints = [];		
//	    for(var j=0;j<xyzPoints.length;++j)
//	    	locPoints[locPoints.length] = xyzPoints[j].getAttribute("value")/(j%3==2?2dViewer.Z_SQUASH_FACTOR:1.0);
//	    	
//	     
//	    gi = 2dViewer.geometry.length;
//	    2dViewer.geometry[gi] = gl.createBuffer();
//	    gl.bindBuffer(gl.ARRAY_BUFFER, 2dViewer.geometry[gi]);
//	    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(locPoints), gl.STATIC_DRAW);
//	    2dViewer.geometry[gi].itemSize = 3;
//	    2dViewer.geometry[gi].numItems = locPoints.length/3;
//	}
//	
//	Debug.log("2dViewer getGeometryHandler  geometry objects " + 2dViewer.geometry.length);  
	2dViewer.canvas.style.zIndex = 0; //Bring canvas to front, above "Loading..." if not already there
//	
//	2dViewer.cameraAction = true;
	
	2dViewer.redraw();
}

//2dViewer.redraw ~~
//	called to redraw 2d objects
2dViewer.redraw = function () {
	Debug.log("2dViewer redraw " );

	
	2dViewer.context.save();

	2dViewer.context.clearRect(0, 0, 2dViewer.canvas.width, 2dViewer.canvas.height);
	
	//
	

	2dViewer.context.fillStyle="RGBA(255,0,0,.2)";
	2dViewer.context.strokeStyle = 'RGBA(255,0,0,.1)';
	2dViewer.context.lineWidth = 100;

	2dViewer.context.translate(300,300);
	2dViewer.context.rotate(45*Math.PI/180);
	
	2dViewer.context.beginPath();
	2dViewer.context.moveTo(-200,0);
	2dViewer.context.lineTo(200,0);
	2dViewer.context.stroke();


	//2dViewer.context.fillRect(-100,-50,200,100);
	
	2dViewer.context.restore();
	

	2dViewer.context.save();
	
	2dViewer.context.fillStyle="RGBA(255,0,0,.2)";
	2dViewer.context.strokeStyle = 'RGBA(255,0,0,.1)';
	2dViewer.context.lineWidth = 100;	

	2dViewer.context.translate(300,300);
	2dViewer.context.rotate(-45*Math.PI/180);
	
	2dViewer.context.beginPath();
	2dViewer.context.moveTo(-200,0);
	2dViewer.context.lineTo(200,0);
	2dViewer.context.stroke();
	//2dViewer.context.fillRect(-100,-50,200,100);
	
	2dViewer.context.restore();

//	if(2dViewer.hud.isInMotion()) return;
//	
//	2dViewer.initForDraw();
//	if(2dViewer.drawAxesFlag) 2dViewer.drawAxes();
//	if(2dViewer.drawGridFlag) 2dViewer.drawGrid();
//	2dViewer.drawGeometry();
//
//    //drawScene();
//
//	2dViewer.drawClusters((2dViewer.accumulateEvents && !2dViewer.enableFlyBy)?-1:2dViewer.eventToDisplay);	
//	if(2dViewer.drawTracksFlag) 2dViewer.drawTracks((2dViewer.accumulateEvents && !2dViewer.enableFlyBy)?-1:2dViewer.eventToDisplay);	
//	2dViewer.hud.update(); //update hud
}

2dViewer.initForDraw = function() {
 
}

2dViewer.drawGrid = function() {

}




