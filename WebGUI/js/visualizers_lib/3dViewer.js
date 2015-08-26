//=====================================================================================
//
//	Created Aug, 2013
//	by Ryan Rivera ((rrivera at fnal.gov)) and Dan Parilla
//
//	3dViewer.js
//
//	requires an omni div, that will be made full window size (window.innerWidth and 
//		window.innerHeight) at position 0,0
//
//	note: good tutorial http://learningwebgl.com/blog/?p=28
//		expanded glMatrix library: http://code.google.com/p/glmatrix/source/browse/glMatrix.js?spec=svne5ad8f6975eef038de668914a44ed36e2c611966&r=e5ad8f6975eef038de668914a44ed36e2c611966
//		reference card: http://www.khronos.org/files/webgl/webgl-reference-card-1_0.pdf
//	public function list:
//		3dViewer.launch()
//
//=====================================================================================

var 3dViewer = 3dViewer || {}; //define namespace


////////////////////////////////////////////////////////////
//public function definitions

// 3dViewer.launch ~~
//		called to start 3d viewer
3dViewer.launch = function() {
	
	Debug.log("3dViewer.launch");
	
	3dViewer.omni = document.getElementById("omni");
	3dViewer.omni.innerHTML = ""; //clear all content
	
	//initialize with loading message in center
    var w = 3dViewer.w = window.innerWidth;
    var h = 3dViewer.h = window.innerHeight;

	3dViewer.omni.style.position = "absolute";
    3dViewer.omni.style.left = 0  + "px";
    3dViewer.omni.style.top = 0  + "px";
	3dViewer.omni.style.width = w + "px";
	3dViewer.omni.style.height = h + "px";
	3dViewer.omni.style.backgroundColor = "rgb(30,30,30)";
	
	3dViewer.omni.innerHTML = "<center><div style='margin-top:" + (h/2-8) + "px'>Loading 3-D...</div></center>";
	
	
	//load/insert libraries and shaders into header and setup webgl
	// steps: 
	//		- load WebGL libraries
	//		- setup canvas behind "loading..."
	//		- init WebGL
	//		- load WebGL shaders
	//		- setup hud
	//		- get default geometry from server
	//		- bring canvas in front of "loading..."
	
	3dViewer.loadScripts();
}

//end public function definitions
////////////////////////////////////////////////////////////


3dViewer.CANVAS_MIN_SIZE = 300; //dont allow w or h to go less than this
3dViewer.HUD_WIDTH = 200;
3dViewer.HUD_MARGIN_RIGHT = 10;
3dViewer.HUD_DROP_DOWN_SPEED = 10;
3dViewer.WEBGL_NEAR_CLIPPING_DEPTH = 1000.0;
3dViewer.WEBGL_FAR_CLIPPING_DEPTH = 10000000.0; 
3dViewer.WEBGL_RESET_CAMERA_POS = [-300000, 50000, -300000];
3dViewer.WEBGL_RESET_CAMERA_FOCUS = [0, 50000, 0];
3dViewer.WEBGL_RESET_CAMERA_UP = [0, 1, 0];
3dViewer.WEBGL_AXES_LENGTH = 1000000; //microns
3dViewer.WEBGL_GRID_SPACING = 50000; //microns
3dViewer.WEBGL_GRID_EXPANSE = 1000000; //microns
3dViewer.WEBGL_GRID_YOFF = -100000; //microns
3dViewer.TICK_REFRESH_PERIOD = 30; //ms
3dViewer.KEY_ROT_SPEED_FAST = 0.06; 
3dViewer.KEY_ROT_SPEED_SLOW = 0.01;
3dViewer.KEY_MOVE_SPEED_FAST = 10000; //incremental size of key navigation in fast mode 
3dViewer.KEY_MOVE_SPEED_SLOW = 1000; //incremental size of key navigation in slow mode 
3dViewer.MOUSE_NAV_SPEED = 0.005; 
3dViewer.MOUSE_WHEEL_SPEED_SLOW = 10000; 
3dViewer.MOUSE_WHEEL_SPEED_FAST = 100000; 
3dViewer.TOUCH_NAV_SPEED = 0.001; 
3dViewer.TOUCH_ZOOM_SPEED = 10000; 
3dViewer.DOUBLE_TAP_RESET_TIME = 600; //ms 
3dViewer.DOUBLE_TAP_HUD_Y_ACCEPT = 300; 
3dViewer.Z_SQUASH_FACTOR = 10.0; 

3dViewer.FLY_BY_ROT_RATE = 0.003; 
3dViewer.FLY_BY_HEIGHT_MAX = 10000; //0
3dViewer.FLY_BY_HEIGHT_RATE = 0.001; 
3dViewer.FLY_BY_RADIUS_RATE = 0.002; 
3dViewer.FLY_BY_RADIUS_SWING = 100000; //0
3dViewer.FLY_BY_RADIUS_MIN = 10000;  //0
3dViewer.FLY_BY_FOCUS = [0,3dViewer.FLY_BY_HEIGHT_MAX/10,0];
3dViewer.FLY_BY_EVENT_TIME = 500; //ms for each event
3dViewer.FLY_BY_ACCUM_TIME = 5000; //ms for alternating between accumulated time individual events


3dViewer.omni;
3dViewer.hud;
3dViewer.canvas;
3dViewer.textureCanvas;
3dViewer.textureContext;
3dViewer.cTexture;
3dViewer.gl;
3dViewer.w;
3dViewer.h;
3dViewer.tickTimer;

3dViewer.lastMousePos = [-1,-1];
3dViewer.lastTwoTouchMag = 0;		//used for pinch zooming
3dViewer.lastTouchTime = 0;		//used for double tap recognition
3dViewer.cameraAction = false; //used to redraw if mouse nav occurs
3dViewer.keysDown = []; //12 keys. 1 pressed, 0 unpressed: Q, W, E, R, A, S, D, F, Up, Left, Down, Right
3dViewer.cameraPos; //position of camera
3dViewer.cameraFocus; //3d point for camera focus
3dViewer.cameraUp; //unit direction vector for camera up
3dViewer.cameraKeyNavFast = false;

3dViewer.drawAxesFlag = true;
3dViewer.drawGridFlag = true;
3dViewer.drawTracksFlag = true;
3dViewer.enableMouseNav = true;
3dViewer.enableFlyBy = true;
3dViewer.accumulateEvents = true;
3dViewer.enableFlyByParameter = 0;
3dViewer.flyByEvent = 0;
3dViewer.flyByModeAlarm = 0; //to change fly-by mode, 0 is unset
3dViewer.flyByEventAlarm = 0; //to change fly-by event, 0 is unset
3dViewer.axes;
3dViewer.grid;
3dViewer.geometry;
3dViewer.events; //array of events, where an event has glBuffer .clusters and .tracks 
3dViewer.eventToDisplay = 0; //index of event to display, -1 is all events
3dViewer.runNumber = -1; //index of run to display, -1 is no run loaded

3dViewer.scriptLoadedCount;
3dViewer.SCRIPTS_TO_LOAD = 2;

3dViewer.fragmentShader;
3dViewer.vertexShader;
3dViewer.shaderProgram;
3dViewer.perspectiveMatrix;
3dViewer.modelViewMatrix;
3dViewer.modelViewMatrixStack = [];

//"private" function list
//3dViewer.handleScriptLoaded
//3dViewer.initCanvas
//3dViewer.tick
//3dViewer.handleCanvasKeyDown
//3dViewer.handleCanvasKeyUp
//3dViewer.checkKeyAction
//3dViewer.handleCanvasMouseWheel
//3dViewer.zoomCameraByDelta
//3dViewer.handleCanvasTouchMove
//3dViewer.handleCanvasMouseMove
//3dViewer.handleCanvasTouchStart
//3dViewer.handleCanvasMouseDown
//3dViewer.panCameraByDelta
//3dViewer.handleWindowResize
//3dViewer.initWebGL
//3dViewer.resetCamera
//3dViewer.initShaders
//3dViewer.createHud
//	this.handleWindowResize
//	this.checkboxUpdate
//	this.update
//	this.handleControlDown
//	this.handleControlUp
//	this.handleRunChange
//3dViewer.getEvents
//3dViewer.getEventsHandler
//3dViewer.getGeometry
//3dViewer.getGeometryHandler 
//3dViewer.mvPushMatrix
//3dViewer.mvPopMatrix
//3dViewer.setMatrixUniforms
//3dViewer.redraw
//3dViewer.initForDraw
//3dViewer.initAxesAndGridBuffers
//3dViewer.drawAxes
//3dViewer.drawGrid
//3dViewer.drawGeometry
//3dViewer.drawClusters
//3dViewer.drawTracks

//first step is to load webGl libraries
3dViewer.loadScripts = function() {

	3dViewer.scriptLoadedCount = 0;
	
	var head = document.getElementsByTagName('head')[0];
	var script;
	
	//webgl libraries
	script = document.createElement('script');   
	script.type = 'text/javascript';   
	script.src = '/WebPath/js/js_lib/glMatrix-0.9.5.min.js';	
	script.onload = 3dViewer.handleScriptLoaded;	
	head.appendChild(script);
	
	script = document.createElement('script');   
	script.type = 'text/javascript';   
	script.src = '/WebPath/js/js_lib/webgl-utils.js';	
	script.onload = 3dViewer.handleScriptLoaded;	
	head.appendChild(script);
}

3dViewer.handleScriptLoaded = function() {
	++3dViewer.scriptLoadedCount;
	Debug.log("3dViewer.handleScriptLoaded " + 3dViewer.scriptLoadedCount);
	if(3dViewer.SCRIPTS_TO_LOAD == 3dViewer.scriptLoadedCount) //done, goto next step
		3dViewer.initCanvas();
}

3dViewer.initCanvas = function() {
	
	3dViewer.canvas = document.createElement('canvas');  
	3dViewer.canvas.style.position = "absolute";
    3dViewer.canvas.style.left = 0  + "px";
    3dViewer.canvas.style.top = 0  + "px";
	3dViewer.canvas.style.zIndex = -1;  //-1 is not visible.. change to 0 to bring to front	
	3dViewer.omni.appendChild(3dViewer.canvas);

	3dViewer.textureCanvas = document.createElement('canvas');
	3dViewer.textureContext = 3dViewer.textureCanvas.getContext('2d');
	
	3dViewer.textureCanvas.width = 128;	//must be power of 2 (?)
	3dViewer.textureCanvas.height = 128;
	3dViewer.textureContext.fillStyle = "#FF33FF"; 	// This determines the text colour, it can take a hex value or rgba value (e.g. rgba(255,0,0,0.5))
	3dViewer.textureContext.textAlign = "center";	// This determines the alignment of text, e.g. left, center, right
	3dViewer.textureContext.textBaseline = "middle";	// This determines the baseline of the text, e.g. top, middle, bottom
	3dViewer.textureContext.font = "12px monospace";	// This determines the size of the text and the font family used
	3dViewer.textureContext.fillText("3D Pixel Sensor",3dViewer.textureCanvas.width/2, 3dViewer.textureCanvas.height/2);
	
	3dViewer.initWebGL();

    initBuffers();

	3dViewer.initTexture();
	
	
	3dViewer.hud = new 3dViewer.createHud();
	
	window.onresize = 3dViewer.handleWindowResize;
	3dViewer.handleWindowResize();
	
	3dViewer.getGeometry();	
	3dViewer.getEvents();
	
	if(3dViewer.tickTimer) clearInterval(3dViewer.tickTimer);
	3dViewer.tickTimer = setInterval(3dViewer.tick, 3dViewer.TICK_REFRESH_PERIOD);
		
	3dViewer.canvas.onmousedown = 3dViewer.handleCanvasMouseDown;
	3dViewer.canvas.onmousemove = 3dViewer.handleCanvasMouseMove;
	3dViewer.canvas.onmousewheel = 3dViewer.handleCanvasMouseWheel;
	3dViewer.canvas.addEventListener('touchstart',3dViewer.handleCanvasTouchStart);
	3dViewer.canvas.addEventListener('touchmove',3dViewer.handleCanvasTouchMove);
	window.onkeydown = 3dViewer.handleCanvasKeyDown;
	window.onkeyup = 3dViewer.handleCanvasKeyUp;	
	3dViewer.redraw();
}

function handleLoadedTexture(texture, textureCanvas) {
	Debug.log("3dViewer handleLoadedTexture");
	var gl = 3dViewer.gl;
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);

    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, textureCanvas);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
    gl.generateMipmap(gl.TEXTURE_2D);

    gl.bindTexture(gl.TEXTURE_2D, null);
}

//3dViewer.tick ~~
//	always check keys and periodically check for data
//	and animate camera for fly by
3dViewer.initTexture = function() {
	Debug.log("3dViewer initTexture");
	3dViewer.cTexture = 3dViewer.gl.createTexture();
    handleLoadedTexture(3dViewer.cTexture, 3dViewer.textureCanvas);
}


//3dViewer.tick ~~
//	always check keys and periodically check for data
// 	and animate camera for fly by
3dViewer.tick = function() {
	
	if(3dViewer.enableFlyBy)
	{
		var radius = Math.sin(3dViewer.enableFlyByParameter*3dViewer.FLY_BY_RADIUS_RATE)*3dViewer.FLY_BY_RADIUS_SWING/2 + 
			3dViewer.FLY_BY_RADIUS_SWING/2 + 3dViewer.FLY_BY_RADIUS_MIN;
		var height = Math.cos(3dViewer.enableFlyByParameter*3dViewer.FLY_BY_HEIGHT_RATE)*3dViewer.FLY_BY_HEIGHT_MAX;// + 3dViewer.FLY_BY_HEIGHT_MAX;

		vec3.set([Math.cos(3dViewer.enableFlyByParameter*3dViewer.FLY_BY_ROT_RATE)*radius, 
		          height, 
		          Math.sin(3dViewer.enableFlyByParameter*3dViewer.FLY_BY_ROT_RATE)*radius],
				3dViewer.cameraPos);
		vec3.set(3dViewer.FLY_BY_FOCUS, 3dViewer.cameraFocus);
		
		
		
		//select event to display
		if(!3dViewer.flyByModeAlarm || //reset alarm and switch mode
				(new Date()).getTime() > 3dViewer.flyByModeAlarm)
		{
			if(3dViewer.eventToDisplay < 0) //now in accum mode, switch to individual event
			{
				3dViewer.eventToDisplay = 3dViewer.flyByEvent;
				//reset event timer
				3dViewer.flyByEventAlarm = (new Date()).getTime() + 3dViewer.FLY_BY_EVENT_TIME;	
				
				Debug.log("3dViewer tick Fly-by Mode: Individual");			
			}
			else	//now in individual event mode, switch to accum mode
			{
				3dViewer.eventToDisplay = -1;
				3dViewer.drawTracksFlag = !3dViewer.drawTracksFlag; //alternate with/without tracks

				Debug.log("3dViewer tick Fly-by Mode: Accum");
			}			
			
			//reset mode timer
			3dViewer.flyByModeAlarm = (new Date()).getTime() + 3dViewer.FLY_BY_ACCUM_TIME;
		}
		
		if(3dViewer.eventToDisplay >= 0 &&  //then in individual event mode, so check for timer update
				(new Date()).getTime() > 3dViewer.flyByEventAlarm)
		{
			++3dViewer.flyByEvent;
			if(!3dViewer.events || 3dViewer.flyByEvent >= 3dViewer.events.length)
				3dViewer.flyByEvent = 0;
			3dViewer.eventToDisplay = 3dViewer.flyByEvent;

			//reset event timer
			3dViewer.flyByEventAlarm = (new Date()).getTime() + 3dViewer.FLY_BY_EVENT_TIME;	
			
			Debug.log("3dViewer tick Fly-by Event: " + 3dViewer.eventToDisplay);
		}
		
		//select event to display DONE
		
		
		
		
		
	    3dViewer.redraw();
	    
		++3dViewer.enableFlyByParameter;		
	}
	else 	//keys deactivated during fly by
	{
		if(3dViewer.checkKeyAction() || 3dViewer.cameraAction) //if active key, redraw scene			 
		    3dViewer.redraw();
	}
}

3dViewer.handleCanvasKeyDown = function(event) {
	var c = String.fromCharCode(event.keyCode);
    //Debug.log(event.keyCode + " handleKeyDown " + c);
    if((
    		c == "A" ||	c == "S" || c == "D" ||	c == "W" ||	
    		c == "Q" ||	c == "E" || c == "R" ||	c == "F" ||	
    		c == "%" ||	c == "(" || c == "'" ||	c == "&")	//Left, Down, Right, Up
    		&& !3dViewer.keysDown[c])
        3dViewer.keysDown[c] = 1;
}

3dViewer.handleCanvasKeyUp = function(event) {
	var c = String.fromCharCode(event.keyCode);
    if(
    		c == "A" ||	c == "S" || c == "D" ||	c == "W" ||	
    		c == "Q" ||	c == "E" || c == "R" ||	c == "F" ||	
    		c == "%" ||	c == "(" || c == "'" ||	c == "&")	//Left, Down, Right, Up
        3dViewer.keysDown[c] = 0;   
    else if(c == " " || event.keyCode == 16) //space or shift change speed mode
    {
    	3dViewer.cameraKeyNavFast = !3dViewer.cameraKeyNavFast;
    	3dViewer.hud.update(); //update hud
    }
    else if(c == "Z") //Z reset camera
    {
		3dViewer.resetCamera();
	    3dViewer.redraw();
    }
}

3dViewer.checkKeyAction = function(event) {

   // Debug.log("3dViewer.checkKeyAction ");
	var actionTaken = false;	
	
	if(3dViewer.keysDown["Q"] || 3dViewer.keysDown["E"]) //rotate left or right
	{		
		var dir = [];
		vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
	    
	    var left = [];
	    vec3.cross(3dViewer.cameraUp,dir,left);
	    vec3.normalize(left);	    
	    vec3.scale(left,3dViewer.cameraKeyNavFast?3dViewer.KEY_ROT_SPEED_FAST:3dViewer.KEY_ROT_SPEED_SLOW);	
	    
	    if(3dViewer.keysDown["Q"]) vec3.add(3dViewer.cameraFocus, left, 3dViewer.cameraFocus);
	    else vec3.subtract(3dViewer.cameraFocus, left, 3dViewer.cameraFocus);	    

		vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
	    vec3.normalize(dir);
	    vec3.add(3dViewer.cameraPos, dir, 3dViewer.cameraFocus);
	    
	    actionTaken = true;	    
	}
	
	if(3dViewer.keysDown["R"] || 3dViewer.keysDown["F"]) //rotate down or up
	{		
		var dir = [];
		vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
		
	    var up = [];
	    vec3.set(3dViewer.cameraUp,up);
	    vec3.normalize(up);	    
	    vec3.scale(up,3dViewer.cameraKeyNavFast?3dViewer.KEY_ROT_SPEED_FAST:3dViewer.KEY_ROT_SPEED_SLOW);
	    
	    if(3dViewer.keysDown["R"]) vec3.add(3dViewer.cameraFocus, up, 3dViewer.cameraFocus);
	    else vec3.subtract(3dViewer.cameraFocus, up, 3dViewer.cameraFocus);	  	      
	    
		vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
	    vec3.normalize(dir);
	    vec3.add(3dViewer.cameraPos, dir, 3dViewer.cameraFocus);	        

	    actionTaken = true;	    
	}

	if(3dViewer.keysDown["A"] || 3dViewer.keysDown["D"] ||
			3dViewer.keysDown["%"] || 3dViewer.keysDown["'"]) //slide left or right
	{		
		var dir = [];
		vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
	    
	    var left = [];
	    vec3.cross(3dViewer.cameraUp,dir,left);
	    vec3.normalize(left);	
	    vec3.scale(left,3dViewer.cameraKeyNavFast?3dViewer.KEY_MOVE_SPEED_FAST:3dViewer.KEY_MOVE_SPEED_SLOW);
	    
	    if(3dViewer.keysDown["%"] || 3dViewer.keysDown["A"]) vec3.add(3dViewer.cameraPos, left);
	    else vec3.subtract(3dViewer.cameraPos, left);	
	    
	    vec3.normalize(dir);
	    vec3.add(3dViewer.cameraPos, dir, 3dViewer.cameraFocus);
	    actionTaken = true;	    
	}
	
	if(3dViewer.keysDown["("] || 3dViewer.keysDown["&"]) //slide up or down
	{
		var dir = [];
		vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
		
		var up = [];
		vec3.set(3dViewer.cameraUp,up);
	    vec3.normalize(up);	
	    vec3.scale(up,3dViewer.cameraKeyNavFast?3dViewer.KEY_MOVE_SPEED_FAST:3dViewer.KEY_MOVE_SPEED_SLOW);

	    if(3dViewer.keysDown["&"]) vec3.add(3dViewer.cameraPos, up);
	    else vec3.subtract(3dViewer.cameraPos, up);	
	    
	    vec3.normalize(dir);
	    vec3.add(3dViewer.cameraPos, dir, 3dViewer.cameraFocus);
	    actionTaken = true;	    
	}
	
	if(3dViewer.keysDown["W"] || 3dViewer.keysDown["S"]) //move forward or backward
	{
		var dir = [];
		vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
	    vec3.normalize(dir);		
	    vec3.scale(dir,3dViewer.cameraKeyNavFast?3dViewer.KEY_MOVE_SPEED_FAST:3dViewer.KEY_MOVE_SPEED_SLOW);
	    
	    if(3dViewer.keysDown["W"]) vec3.add(3dViewer.cameraPos, dir);
	    else vec3.subtract(3dViewer.cameraPos, dir);	
	    
	    vec3.normalize(dir);
	    vec3.add(3dViewer.cameraPos, dir, 3dViewer.cameraFocus);
	    actionTaken = true;	    
	}

	if(3dViewer.keysDown["W"] || 3dViewer.keysDown["S"]) //move forward or backward
	{
	    actionTaken = true;	    
	}
	
	return actionTaken;
}

3dViewer.handleCanvasMouseWheel = function(event) {
	var delta = event.wheelDelta/120;	     
	3dViewer.zoomCameraByDelta(delta, 
			3dViewer.cameraKeyNavFast?3dViewer.MOUSE_WHEEL_SPEED_FAST:3dViewer.MOUSE_WHEEL_SPEED_SLOW);
}

3dViewer.zoomCameraByDelta = function(delta, speed) {
	if(!delta) return; //0 delta do nothing
	
	var dir = [];
	vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
    vec3.normalize(dir);		
    vec3.scale(dir,speed);
    
    if(delta > 0) vec3.add(3dViewer.cameraPos, dir);
    else vec3.subtract(3dViewer.cameraPos, dir);
    
    vec3.normalize(dir);
    vec3.add(3dViewer.cameraPos, dir, 3dViewer.cameraFocus);  
    
    3dViewer.cameraAction = true;	 	
}

3dViewer.handleCanvasTouchMove = function(touchEvent) {

	touchEvent.preventDefault(); //fix chrome issue of only 2 fires
	touchEvent.cancelBubble=true; //eat event away so scrolling doesnt happen
	3dViewer.lastTouchTime = 0; //prevent a moving finger from firing double tap reset camera view
	
    var touch = touchEvent.targetTouches[0];		
    var newMousePos = [touch.pageX, touch.pageY];
    var touch2 = touchEvent.targetTouches[1];	
    if(touch2) //second finger detected, so do zoom action
    {
        var mag = Math.sqrt((touch2.pageX-touch.pageX)*(touch2.pageX-touch.pageX) +
        	(touch2.pageY-touch.pageY)*(touch2.pageY-touch.pageY));
    	//Debug.log("3dViewer touch2 " + touch2.pageX + "-" + touch2.pageY + "=" + mag);
    	if(3dViewer.lastTwoTouchMag)    	
    		3dViewer.zoomCameraByDelta(mag-3dViewer.lastTwoTouchMag, 3dViewer.TOUCH_ZOOM_SPEED);
    	3dViewer.lastTwoTouchMag = mag;	
        3dViewer.lastMousePos = [-1,-1];	 //prevent weird jumps in transition from zoom to move 	
    	return;
    }
	
    var delta = [3dViewer.lastMousePos[0] - newMousePos[0],
                  3dViewer.lastMousePos[1] - newMousePos[1]];
    if(3dViewer.lastMousePos[0] > 0) //prevent weird jumps in transition from zoom to move 
    	3dViewer.panCameraByDelta(delta,3dViewer.TOUCH_NAV_SPEED);
    3dViewer.lastMousePos = newMousePos;
}

3dViewer.handleCanvasMouseMove = function(event) {

	if(3dViewer.enableMouseNav && event.which == 1) //left mouse button
	{
	    var newMousePos = [event.clientX, event.clientY];
	
	    var delta = [newMousePos[0] - 3dViewer.lastMousePos[0],
	                 newMousePos[1] - 3dViewer.lastMousePos[1]];

	    3dViewer.panCameraByDelta(delta,3dViewer.MOUSE_NAV_SPEED);
	    3dViewer.lastMousePos = newMousePos;
	}
}

3dViewer.handleCanvasTouchStart = function(touchEvent) {
	var touchTime = new Date().getTime();
    var touch = touchEvent.targetTouches[0];	
	3dViewer.lastMousePos = [touch.pageX, touch.pageY];	
	if(3dViewer.lastTouchTime && !touchEvent.targetTouches[1] && //test if single-finger double tap occured
			touchTime - 3dViewer.lastTouchTime < 3dViewer.DOUBLE_TAP_RESET_TIME)
	{	//double tap detected!! 
		
		touchEvent.preventDefault(); //prevent native zoom action
		
		if(touch.pageY < 3dViewer.DOUBLE_TAP_HUD_Y_ACCEPT && 	//expand 3d hud! 
				touch.pageX > window.innerWidth - 3dViewer.hud.hudMouseOverDiv.offsetWidth - 2*3dViewer.HUD_MARGIN_RIGHT)
		{		
			3dViewer.hud.mouseOverDropDown();	
			return;
		}
		
		if(touch.pageY < 3dViewer.DOUBLE_TAP_HUD_Y_ACCEPT && 	//expand top level visualization hud! 
				touch.pageX < 3dViewer.hud.hudMouseOverDiv.offsetWidth + 2*3dViewer.HUD_MARGIN_RIGHT)
		{		
			mouseOverDropDown();
			return;
		}
		
		//else reset camera view		
		Debug.log("3dViewer handleCanvasTouchStart Double Tap Reset Camera! " + (touchTime - 3dViewer.lastTouchTime));
		3dViewer.resetCamera();
	    3dViewer.redraw();
	    return;	    
	}
	
	3dViewer.lastTouchTime = touchTime;
	3dViewer.hud.mouseOutDropDown();	//elevate drop downs if touches going on
	mouseOutDropDown(); //top level visualization drop down hud
}

3dViewer.handleCanvasMouseDown = function(event) {
	Debug.log("3dViewer handleCanvasMouseDown ");
	3dViewer.lastMousePos = [event.clientX, event.clientY];		
	window.focus(); //manual focus since return false
	if(3dViewer.hud)	//manual remove focus from any text inputs!
		3dViewer.hud.hudRunNumberInput.blur();
	return false; //return false to cancel drag and drop action  
}
	
3dViewer.panCameraByDelta = function(delta, speed) {	
	if(!delta[0] && !delta[1]) return;
	
	//rotate left/right
    var dir = [];
	vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
    
    var left = [];
    vec3.cross(3dViewer.cameraUp,dir,left);
    vec3.normalize(left);	    
    vec3.scale(left,-delta[0]*speed);	
    
    vec3.add(3dViewer.cameraFocus, left, 3dViewer.cameraFocus);   

	vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
    vec3.normalize(dir);
    vec3.add(3dViewer.cameraPos, dir, 3dViewer.cameraFocus);

    //rotate up/down
    var up = [];
    vec3.set(3dViewer.cameraUp,up);
    vec3.normalize(up);	    
    vec3.scale(up,-delta[1]*speed);	
    	    
    vec3.add(3dViewer.cameraFocus, up, 3dViewer.cameraFocus);  
	vec3.subtract(3dViewer.cameraFocus,3dViewer.cameraPos,dir);
    vec3.normalize(dir);
    vec3.add(3dViewer.cameraPos, dir, 3dViewer.cameraFocus);	        

    3dViewer.cameraAction = true;	
}

3dViewer.handleWindowResize = function() {
	
	var w = window.innerWidth < 3dViewer.CANVAS_MIN_SIZE? 3dViewer.CANVAS_MIN_SIZE:window.innerWidth;
	var h = window.innerHeight < 3dViewer.CANVAS_MIN_SIZE? 3dViewer.CANVAS_MIN_SIZE:window.innerHeight;

	Debug.log("3dViewer.handleWindowResize " + w + "-" + h);
	
	3dViewer.omni.style.width = w + "px";
	3dViewer.omni.style.height = h + "px";
	3dViewer.canvas.style.width = w + "px";
	3dViewer.canvas.style.height = h + "px";
	3dViewer.canvas.width = w;
	3dViewer.canvas.height = h;

	//resize webGL
	var gl = 3dViewer.gl;
	gl.viewportWidth = 3dViewer.canvas.width;
	gl.viewportHeight = 3dViewer.canvas.height;
	gl.clearColor(0.0, 0.0, 0.0, 1.0);  // Clear to black, fully opaque
	gl.enable(3dViewer.gl.DEPTH_TEST);           // Enable depth testing
	gl.depthFunc(3dViewer.gl.LEQUAL);            // Near things obscure far things
	gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
    mat4.perspective(45, gl.viewportWidth / gl.viewportHeight, 3dViewer.WEBGL_NEAR_CLIPPING_DEPTH, 3dViewer.WEBGL_FAR_CLIPPING_DEPTH, 3dViewer.perspectiveMatrix);
    
	//reposition HUD
	3dViewer.hud.handleWindowResize();
	
	//redraw 3d world
	3dViewer.redraw();
}

// 3dViewer.initWebGL ~~
//		warns user that their browser is incompatible 
3dViewer.initWebGL = function() {
	try {
		3dViewer.gl = 3dViewer.canvas.getContext("webgl") || 3dViewer.canvas.getContext("experimental-webgl");
    } 	        	
	catch (e) {   alert("Could not initialize WebGL, sorry :-("); return;		}
    
	if (!3dViewer.gl) {   alert("Could not initialize WebGL! sorry :-("); return;		}	

	3dViewer.initShaders();	

	3dViewer.perspectiveMatrix = mat4.create();
	3dViewer.modelViewMatrix = mat4.create();
	
	3dViewer.resetCamera();
	3dViewer.initAxesAndGridBuffers();
}

3dViewer.resetCamera = function() {	
	3dViewer.cameraPos = [];
	3dViewer.cameraUp = [];
	3dViewer.cameraFocus = [];
	vec3.set(3dViewer.WEBGL_RESET_CAMERA_POS, 3dViewer.cameraPos);
	vec3.set(3dViewer.WEBGL_RESET_CAMERA_UP, 3dViewer.cameraUp);	
	vec3.set(3dViewer.WEBGL_RESET_CAMERA_FOCUS, 3dViewer.cameraFocus);	
	
	//gaurantee vectors are normalized
	vec3.normalize(3dViewer.cameraUp);

	Debug.log("3dViewer.resetCamera to pos: " + 3dViewer.cameraPos + " focus: " + 3dViewer.cameraFocus + " up: " + 3dViewer.cameraUp);
		
}

3dViewer.initShaders = function() {	

	//get fragment shader
	var req = new XMLHttpRequest();  
	req.open("GET",'/WebPath/js/js_lib/webgl_shader-fs.shader', false);  
	req.send(null);  
	var fsCode = (req.status == 200) ? req.responseText : null;
	if(!fsCode) {   alert("Could not initialize WebGL fs shaders, sorry :-("); return;		}
	3dViewer.fragmentShader = 3dViewer.gl.createShader(3dViewer.gl.FRAGMENT_SHADER);	
	//Debug.log("fragmentShader" + fsCode);
	3dViewer.gl.shaderSource(3dViewer.fragmentShader, fsCode);
	3dViewer.gl.compileShader(3dViewer.fragmentShader);  
	if (!3dViewer.gl.getShaderParameter(3dViewer.fragmentShader, 3dViewer.gl.COMPILE_STATUS)) 
	{	 
		alert("An error occurred compiling the fragment shader: " + 3dViewer.gl.getShaderInfoLog(3dViewer.fragmentShader));   
		return; 
	}
	
	//get vertex shader
	req = new XMLHttpRequest();  
	req.open("GET",'/WebPath/js/js_lib/webgl_shader-vs.shader', false);  
	req.send(null);  
	var vsCode = (req.status == 200) ? req.responseText : null;
	if(!vsCode) {   alert("Could not initialize WebGL vs shaders, sorry :-("); return;		}
	3dViewer.vertexShader = 3dViewer.gl.createShader(3dViewer.gl.VERTEX_SHADER);
	//Debug.log("vertexShader" + vsCode);
	3dViewer.gl.shaderSource(3dViewer.vertexShader, vsCode);
	3dViewer.gl.compileShader(3dViewer.vertexShader);
	if (!3dViewer.gl.getShaderParameter(3dViewer.vertexShader, 3dViewer.gl.COMPILE_STATUS)) 
	{	 
		alert("An error occurred compiling the vertex shader: " + 3dViewer.gl.getShaderInfoLog(3dViewer.vertexShader));   
		return; 
	}
	
	//setup shader progam
	3dViewer.shaderProgram = 3dViewer.gl.createProgram();
	3dViewer.gl.attachShader(3dViewer.shaderProgram, 3dViewer.vertexShader);
	3dViewer.gl.attachShader(3dViewer.shaderProgram, 3dViewer.fragmentShader);
	3dViewer.gl.linkProgram(3dViewer.shaderProgram);
	
	if (!3dViewer.gl.getProgramParameter(3dViewer.shaderProgram, 3dViewer.gl.LINK_STATUS)) 
	{    alert("Could not initialize shader program"); return;	}
	
	3dViewer.gl.useProgram(3dViewer.shaderProgram);
	
	3dViewer.shaderProgram.vertexPositionAttribute = 3dViewer.gl.getAttribLocation(3dViewer.shaderProgram, "aVertexPosition");
	3dViewer.gl.enableVertexAttribArray(3dViewer.shaderProgram.vertexPositionAttribute);
    
	3dViewer.shaderProgram.vertexColorAttribute = 3dViewer.gl.getAttribLocation(3dViewer.shaderProgram, "aVertexColor");
	3dViewer.gl.enableVertexAttribArray(3dViewer.shaderProgram.vertexColorAttribute);
	
	3dViewer.shaderProgram.pMatrixUniform = 3dViewer.gl.getUniformLocation(3dViewer.shaderProgram, "uPMatrix");
	3dViewer.shaderProgram.mvMatrixUniform = 3dViewer.gl.getUniformLocation(3dViewer.shaderProgram, "uMVMatrix");
}

3dViewer.createHud = function() {
		
	var hudMouseOverDiv;
	var animationTargetTop, isDropDownAnimating, isDropDownDown;
	var controlMouseIsDown = false;
	var controlMouseTimeout = 0;
	var getEventsTimeout = 0;
	
	this.isInMotion = function() { return isDropDownAnimating; }
	
	this.handleWindowResize = function() {
		Debug.log("3dViewer Hud handleWindowResize");
		this.hudMouseOverDiv.style.left = window.innerWidth - this.hudMouseOverDiv.offsetWidth - 3dViewer.HUD_MARGIN_RIGHT + "px";
		
		this.hudNavSpeedDiv.style.left = 5 + "px";
		this.hudNavSpeedDiv.style.top = window.innerHeight - 95 + "px";	
		
		this.update();
	}
	
	this.checkboxUpdate = function(i) {
		var chk = document.getElementById("hudCheckbox" + i);
		Debug.log("3dViewer Hud checkboxUpdate " + i + "=" + chk.checked);
		
		if(i==0) 3dViewer.drawAxesFlag = chk.checked; //axes 
		else if(i==1) 3dViewer.drawGridFlag = chk.checked; //grid 
		else if(i==2) 3dViewer.drawTracksFlag = chk.checked; //tracks 
		else if(i==3) 3dViewer.enableMouseNav = chk.checked; //mouse nav 
		else if(i==4) 3dViewer.enableFlyBy = chk.checked; //fly-by 
		else if(i==5) 3dViewer.accumulateEvents = chk.checked; //accumulate
				
		3dViewer.redraw();	
		this.update();	
	}
	
	this.update = function() {		
		
		this.hudNavSpeedDiv.innerHTML = "Tests of radiation-hard sensors for the SLHC<br>";
		this.hudNavSpeedDiv.innerHTML += "at the Fermi Test Beam Facility.<br>";
		this.hudNavSpeedDiv.innerHTML += "Devices under test for this run were 3D Pixel Sensors.<br>";
		if(3dViewer.enableFlyBy)
			this.hudNavSpeedDiv.innerHTML += "Fly-by Mode";	
		else
			this.hudNavSpeedDiv.innerHTML += "Nav Speed: " + (3dViewer.cameraKeyNavFast?"Fast":"Slow");	
				
		var str = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" + ((3dViewer.accumulateEvents && 3dViewer.eventToDisplay<0)?"Events Accumulated":("Event: " + 
				(3dViewer.eventToDisplay<0?"All":3dViewer.eventToDisplay)));
		str += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" + "Run: " + (3dViewer.runNumber < 0?"Not found":3dViewer.runNumber);
		this.hudNavSpeedDiv.innerHTML += str;
		
		this.hudEventNumber.innerHTML = "Event #: " + (3dViewer.eventToDisplay<0?"All":3dViewer.eventToDisplay);
		
		if(3dViewer.accumulateEvents)
			this.hudEventNumberControls.style.display = "none";
		else
			this.hudEventNumberControls.style.display = "block";
	}
	
	// animateDropDown ~~
	var animateDropDown = function() {
		var dir = (animationTargetTop - hudMouseOverDiv.offsetTop > 0)? 1: -1;
		
		var tmpTop = hudMouseOverDiv.offsetTop + dir*3dViewer.HUD_DROP_DOWN_SPEED;
		if(Math.abs(tmpTop - animationTargetTop) <= 3dViewer.HUD_DROP_DOWN_SPEED) //done
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
			controlMouseTimeout = window.setTimeout(function() {3dViewer.hud.handleControlDown(i)},to);
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
				3dViewer.eventToDisplay += (i==0?-1:1)*parseInt(3dViewer.events.length/5+1);
			else
				3dViewer.eventToDisplay += (i==1?-1:1);	
			
			if(3dViewer.eventToDisplay < 0)
				3dViewer.eventToDisplay = 0;
			if(3dViewer.eventToDisplay >= 3dViewer.events.length)
				3dViewer.eventToDisplay = 3dViewer.events.length - 1;
			break;
		case 4: //dec run #
		case 5: //inc run #
			3dViewer.hud.hudRunNumberInput.value = parseInt(3dViewer.hud.hudRunNumberInput.value) + (i*2-9);
			if(3dViewer.hud.hudRunNumberInput.value < 0) 3dViewer.hud.hudRunNumberInput.value = 0;
			if(getEventsTimeout) window.clearTimeout(getEventsTimeout);
			getEventsTimeout = window.setTimeout(3dViewer.getEvents,1000);
			break;
		default:
			Debug.log("hud doControlAction unknown action" + i);	
			return;
		}
				
		3dViewer.hud.update();	
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
	this.hudNavSpeedDiv.setAttribute("id", "3dViewer-hudNavSpeedOverlay");
	this.hudNavSpeedDiv.style.position = "absolute";	
	this.hudNavSpeedDiv.style.zIndex = 100;
	3dViewer.omni.appendChild(this.hudNavSpeedDiv);
	
	
	hudMouseOverDiv = this.hudMouseOverDiv = document.createElement('div');	
	hudMouseOverDiv.setAttribute("id", "3dViewer-hudMouseOver");
	hudMouseOverDiv.style.position = "absolute";	
    hudMouseOverDiv.style.zIndex = 100;
	
	this.hudDiv = document.createElement('div');	
	this.hudDiv.setAttribute("id", "3dViewer-hud");
	var chkLabels = ["Show Axes","Show Grid","Show Tracks", "Mouse Nav","Fly-By","Accumulated"];
	var chkDefaults = ["checked","checked","checked","checked",3dViewer.enableFlyBy?"checked":"",3dViewer.accumulateEvents?"checked":""];
	var str = "";
	for(var i=0;i<chkLabels.length;++i)
		str += "<input type='checkbox' id='hudCheckbox" + i + "' onchange='3dViewer.hud.checkboxUpdate(" + i + 
					");' " + chkDefaults[i] + "><label for='hudCheckbox" + i + "'>" + chkLabels[i] + "</label><br>";
	
	//add event controls
	str += "<center><div id='3dViewer-hudEventNumberControls'><div id='3dViewer-hudEventNumber'></div>";
	var evtNumBtns = ["<<","<",">",">>"];
	for(var i=0;i<evtNumBtns.length;++i)		
		str += "<input type='button' onmousedown='3dViewer.hud.handleControlDown(" + i + ",event);' " +
			"onmouseup='3dViewer.hud.handleControlUp(" + i + ");' " +
			"onmouseout='3dViewer.hud.handleControlUp(" + i + ");' " +
			"value='" + evtNumBtns[i] + "' />";	
	str += "</div></center>";
	
	//add run controls
	if(DesktopContent._needToLoginMailbox) //only add if successful login
	{
		str += "<div id='3dViewer-hudRunNumberControls'>Run #: " +
				"<input id='3dViewer-hudRunNumberControls-textInput' oninput='3dViewer.hud.handleRunChange();' type='text' value='40' > ";
		evtNumBtns = ["<",">"];
		for(var i=0;i<evtNumBtns.length;++i)		
			str += "<input type='button' onmousedown='3dViewer.hud.handleControlDown(" + (i+4) + ",event);' " +
				"onmouseup='3dViewer.hud.handleControlUp(" + (i+4) + ");' " +
				"onmouseout='3dViewer.hud.handleControlUp(" + (i+4) + ");' " +
				"value='" + evtNumBtns[i] + "' />";	
		str += "</div>";
	}
	else
	{
		str += "<input id='3dViewer-hudRunNumberControls-textInput' type='hidden' value='40' >";
		
		this.hudUrlOverlay = document.createElement('div');	
		this.hudUrlOverlay.setAttribute("id", "3dViewer-hudUrlOverlay");
		this.hudUrlOverlay.style.position = "absolute";	
		this.hudUrlOverlay.style.zIndex = 100;
		this.hudUrlOverlay.style.left = 5 + "px";
		this.hudUrlOverlay.style.top = 5 + "px";
		this.hudUrlOverlay.innerHTML = "Try on your own mobile device!<br>http://tinyurl.com/q6lhdrm";
		3dViewer.omni.appendChild(this.hudUrlOverlay);
	}
	
	this.hudDiv.innerHTML = str;
	
//	_historyEl.onmousewheel = handleUserInputScroll;
//	_historyEl.onmousedown = handleUserInputScroll;
//	this.hudEventScrollbar.onscroll = this.handleEventScroll;
//	_historyEl.onmousemove = handleMouseMoveScroll;
   
    hudMouseOverDiv.appendChild(this.hudDiv);  
    
    hudMouseOverDiv.style.top = hudMouseOverDiv.offsetHeight - 15 + "px";
    hudMouseOverDiv.style.width = 3dViewer.HUD_WIDTH + "px";
    hudMouseOverDiv.onmouseover = mouseOverDropDown;
    hudMouseOverDiv.onmouseout = mouseOutDropDown;
	3dViewer.omni.appendChild(hudMouseOverDiv);	
	
	this.hudEventNumber = document.getElementById('3dViewer-hudEventNumber'); //get reference
    this.hudEventNumberControls = document.getElementById('3dViewer-hudEventNumberControls'); //get reference
    this.hudRunNumberInput = document.getElementById('3dViewer-hudRunNumberControls-textInput'); //get reference
    
	//setup dropdown effect
	isDropDownDown = false;
	isDropDownAnimating = true;
	animationTargetTop = 15 - hudMouseOverDiv.offsetHeight;
	window.setTimeout(animateDropDown,30);
	this.handleWindowResize();
}


3dViewer.getEvents = function() {
	Debug.log("3dViewer getEvents for run " + 3dViewer.hud.hudRunNumberInput.value);
	DesktopContent.XMLHttpRequest("request?RequestType=getEvents&run=" +
			parseInt(3dViewer.hud.hudRunNumberInput.value), "", 3dViewer.getEventsHandler);
}

//3dViewer.getEventsHandler ~~
3dViewer.getEventsHandler = function (req) {
	//Debug.log("3dViewer getEventsHandler " + req.responseText.substr(1000));
	3dViewer.runNumber = 3dViewer.hud.hudRunNumberInput.value; //update loaded run number
	var events = req.responseXML.getElementsByTagName("event");
	
	3dViewer.events = [];
	var evt, cnt;
	var gl = 3dViewer.gl;
	var locPoints, xyzPoints, slopes, intercepts;
	var locSlope = [];
	var locIntcpt = [];
	var pixelSz = 200;
	var trackSz = 1000000;	
	for(var i=0;i<events.length;++i)
	{		
		xyzPoints = events[i].getElementsByTagName("xyz_point");
	
		//create a new event to hold clusters and tracks
	    evt = 3dViewer.events.length;
	    3dViewer.events[evt] = 3dViewer.events[evt] || {};
	    3dViewer.events[evt].clusters = [];
		
	    //clusters
		locPoints = [];		
	    for(var j=0;j<xyzPoints.length;++j) //for each cluster point, make a square
	    {
	    	if(j%3==2) //have the x,y of cluster point started, finish it
	    	{
	    		locPoints[locPoints.length] = xyzPoints[j].getAttribute("value")/3dViewer.Z_SQUASH_FACTOR;
	    		
				for(var k=0;k<3;++k) //make the 3 other points of square, original point is top left, and then two adj, and then opposite for TRIANGLE_STRIP
				{	    			 
					locPoints[locPoints.length] = locPoints[locPoints.length-3*(k+1)] + (k==0?pixelSz:(k==1?0:pixelSz));
					locPoints[locPoints.length] = locPoints[locPoints.length-3*(k+1)] + (k==0?0:(k==1?pixelSz:pixelSz));
					locPoints[locPoints.length] = locPoints[locPoints.length-3*(k+1)];
				}
	    		
				//a new cluster is complete

				cnt = 3dViewer.events[evt].clusters.length;
				3dViewer.events[evt].clusters[cnt] = gl.createBuffer();
			    gl.bindBuffer(gl.ARRAY_BUFFER, 3dViewer.events[evt].clusters[cnt]);
			    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(locPoints), gl.STATIC_DRAW);
			    3dViewer.events[evt].clusters[cnt].itemSize = 3;
			    3dViewer.events[evt].clusters[cnt].numItems = locPoints.length/3;
				locPoints = [];	
	    	}
	    	else //get x and y
		    	locPoints[locPoints.length] = xyzPoints[j].getAttribute("value") - pixelSz/2;
	    }
	    
	    //tracks
	    slopes = events[i].getElementsByTagName("slope");
	    intercepts = events[i].getElementsByTagName("intcpt");
	    locPoints = [];		
	    for(var j=0;j<slopes.length;j+=2) //for each track, make a line
	    {
	    	locSlope[0] = parseInt(slopes[j].getAttribute("value"))/3dViewer.Z_SQUASH_FACTOR;
	    	locSlope[1] = parseInt(slopes[j+1].getAttribute("value"))/3dViewer.Z_SQUASH_FACTOR;
	    	locIntcpt[0] = parseInt(intercepts[j].getAttribute("value"));
	    	locIntcpt[1] = parseInt(intercepts[j+1].getAttribute("value"));
	    	locPoints[locPoints.length] = locSlope[0]*-trackSz + locIntcpt[0]; 		//x
	    	locPoints[locPoints.length] = locSlope[1]*-trackSz + locIntcpt[1]; 		//y
	    	locPoints[locPoints.length] = -trackSz; 								//z	
	    	locPoints[locPoints.length] = locSlope[0]*trackSz + locIntcpt[0]; 		//x
	    	locPoints[locPoints.length] = locSlope[1]*trackSz + locIntcpt[1]; 		//y
	    	locPoints[locPoints.length] = trackSz; 									//z	  	
	    }
		//Debug.log("3dViewer getEventsHandler event locPoints " + locPoints);
	    
	    3dViewer.events[evt].tracks = gl.createBuffer();
	    gl.bindBuffer(gl.ARRAY_BUFFER, 3dViewer.events[evt].tracks);
	    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(locPoints), gl.STATIC_DRAW);
	    3dViewer.events[evt].tracks.itemSize = 3;
	    3dViewer.events[evt].tracks.numItems = locPoints.length/3; 
	}

	Debug.log("3dViewer getEventsHandler event count " + 3dViewer.events.length); 
	3dViewer.cameraAction = true;
}

3dViewer.getGeometry = function() {
	Debug.log("3dViewer.getGeometry ");
	DesktopContent.XMLHttpRequest("request?RequestType=getGeometry", "", 3dViewer.getGeometryHandler);
}

// 3dViewer.getGeometryHandler ~~
3dViewer.getGeometryHandler = function (req) {
	//Debug.log("3dViewer getGeometryHandler " + req.responseText);
	
	var objects = req.responseXML.getElementsByTagName("object");
	
	3dViewer.geometry = [];
	var gi;
	var gl = 3dViewer.gl;
	var locPoints, xyzPoints, objectType;
	for(var i=0;i<objects.length;++i)
	{
		//TODO if different geometries, use object type
		//objectType = objects[i].getElementsByTagName("object_type");
	    //Debug.log("Rotating3d getGeometry objectType " + objectType[0].getAttribute("value"));
		
		xyzPoints = objects[i].getElementsByTagName("xyz_point");
	
		locPoints = [];		
	    for(var j=0;j<xyzPoints.length;++j)
	    	locPoints[locPoints.length] = xyzPoints[j].getAttribute("value")/(j%3==2?3dViewer.Z_SQUASH_FACTOR:1.0);
	    	
	     
	    gi = 3dViewer.geometry.length;
	    3dViewer.geometry[gi] = gl.createBuffer();
	    gl.bindBuffer(gl.ARRAY_BUFFER, 3dViewer.geometry[gi]);
	    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(locPoints), gl.STATIC_DRAW);
	    3dViewer.geometry[gi].itemSize = 3;
	    3dViewer.geometry[gi].numItems = locPoints.length/3;
	}
	
	Debug.log("3dViewer getGeometryHandler  geometry objects " + 3dViewer.geometry.length);  
	3dViewer.canvas.style.zIndex = 0; //Bring canvas to front, above "Loading..." if not already there
	
	3dViewer.cameraAction = true;
}

3dViewer.mvPushMatrix = function() {
    copy = mat4.create();
    mat4.set(3dViewer.modelViewMatrix, copy);
    3dViewer.modelViewMatrixStack.push(copy);
}

3dViewer.mvPopMatrix = function() {
    if (3dViewer.modelViewMatrixStack.length == 0) {
        throw "Invalid popMatrix!";
    }
    3dViewer.modelViewMatrix = 3dViewer.modelViewMatrixStack.pop();
}

//3dViewer.setMatrixUniforms ~~
//	pushes the shader matrices to the from js to webgl world
//	need to call any time matrices are modified
3dViewer.setMatrixUniforms = function() {
	3dViewer.gl.uniformMatrix4fv(3dViewer.shaderProgram.pMatrixUniform, false, 3dViewer.perspectiveMatrix);
	3dViewer.gl.uniformMatrix4fv(3dViewer.shaderProgram.mvMatrixUniform, false, 3dViewer.modelViewMatrix);
}


//3dViewer.redraw ~~
//	called to redraw 3d objects
3dViewer.redraw = function () {

	if(3dViewer.hud.isInMotion()) return;
	
	3dViewer.initForDraw();
	if(3dViewer.drawAxesFlag) 3dViewer.drawAxes();
	if(3dViewer.drawGridFlag) 3dViewer.drawGrid();
	3dViewer.drawGeometry();

    //drawScene();

	3dViewer.drawClusters((3dViewer.accumulateEvents && !3dViewer.enableFlyBy)?-1:3dViewer.eventToDisplay);	
	if(3dViewer.drawTracksFlag) 3dViewer.drawTracks((3dViewer.accumulateEvents && !3dViewer.enableFlyBy)?-1:3dViewer.eventToDisplay);	
	3dViewer.hud.update(); //update hud
}

3dViewer.initForDraw = function() {

	var gl = 3dViewer.gl;
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    mat4.identity(3dViewer.modelViewMatrix);   
    
    //generate camera lookAt matrix and apply
    var tmpLookAtMatrix = [];
    mat4.lookAt(3dViewer.cameraPos, 3dViewer.cameraFocus, 3dViewer.cameraUp, tmpLookAtMatrix);
    mat4.multiply(3dViewer.modelViewMatrix,tmpLookAtMatrix);   //apply camera rotation
    3dViewer.setMatrixUniforms();    
}

3dViewer.initAxesAndGridBuffers = function() {
	var gl = 3dViewer.gl;
	
	//for axes
	var tmpArray = [ //for gl.LINE
		0,0,0,
		3dViewer.WEBGL_AXES_LENGTH,0,0,
		0,0,0,
		0,3dViewer.WEBGL_AXES_LENGTH,0,
		0,0,0,
		0,0,3dViewer.WEBGL_AXES_LENGTH            ]
      
	3dViewer.axes = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, 3dViewer.axes);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(tmpArray), gl.STATIC_DRAW);
	3dViewer.axes.itemSize = 3;
	3dViewer.axes.numItems = tmpArray.length/3;
	
	//for grid
	tmpArray = [];
	var ti = 0;
	for(var x = 0; x <= 2; x+=2) //lines parallel to x axis and then to z axis
		for(var i = -3dViewer.WEBGL_GRID_EXPANSE; i <= 3dViewer.WEBGL_GRID_EXPANSE; i += 3dViewer.WEBGL_GRID_SPACING)
			for(var j = -3dViewer.WEBGL_GRID_EXPANSE; j <= 3dViewer.WEBGL_GRID_EXPANSE; j += 3dViewer.WEBGL_GRID_EXPANSE*2, ++ti)
			{
				tmpArray[ti*3 + x] = j; //first time this is x coord, second time z
				tmpArray[ti*3 + 2 - x] = i; //first time this is z coord, second time x
				tmpArray[ti*3 + 1] = 3dViewer.WEBGL_GRID_YOFF;
			}	
	
	3dViewer.grid = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, 3dViewer.grid);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(tmpArray), gl.STATIC_DRAW);
	3dViewer.grid.itemSize = 3;
	3dViewer.grid.numItems = tmpArray.length/3;
	
	//Debug.log(tmpArray);
}

3dViewer.drawAxes = function() {	
	var gl = 3dViewer.gl;

	var geometryColorBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, geometryColorBuffer);
	var colors = [];
    for (var i=0; i < 3dViewer.axes.numItems; i++)
		 colors = colors.concat([1.0, 1.0, 1.0, 1.0]);    
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, geometryColorBuffer);
    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexColorAttribute, 4, gl.FLOAT, false, 0, 0);
    
    gl.lineWidth(2.0);
    gl.bindBuffer(gl.ARRAY_BUFFER, 3dViewer.axes);
    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexPositionAttribute, 3dViewer.axes.itemSize, gl.FLOAT, false, 0, 0);    
    gl.drawArrays(gl.LINES, 0, 3dViewer.axes.numItems);
}

3dViewer.drawGrid = function() {
	var gl = 3dViewer.gl;
	

	var geometryColorBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, geometryColorBuffer);
	var colors = [];
    for (var i=0; i < 3dViewer.grid.numItems; i++)
		 colors = colors.concat([0.5, 0.5, 0.5, 1.0]);    
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, geometryColorBuffer);
    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexColorAttribute, 4, gl.FLOAT, false, 0, 0);
    
    gl.lineWidth(1.0);
    gl.bindBuffer(gl.ARRAY_BUFFER, 3dViewer.grid);
    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexPositionAttribute, 3dViewer.grid.itemSize, gl.FLOAT, false, 0, 0);    
    gl.drawArrays(gl.LINES, 0, 3dViewer.grid.numItems);
}

3dViewer.drawGeometry = function() {
	
	if(!3dViewer.geometry) return;
	
	var gl = 3dViewer.gl;
    gl.lineWidth(4.0);
    
	var geometryColorBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, geometryColorBuffer);
	var colors = [];
    for (var i=0; i < 4; i++)
		 colors = colors.concat([0.0, 0.0, 1.0, 1.0]);    
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, geometryColorBuffer);
    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexColorAttribute, 4, gl.FLOAT, false, 0, 0);
    
	for(var i=0;i<3dViewer.geometry.length;++i)
	{
	    gl.bindBuffer(gl.ARRAY_BUFFER, 3dViewer.geometry[i]);
	    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexPositionAttribute, 3dViewer.geometry[i].itemSize, gl.FLOAT, false, 0, 0);   

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, 3dViewer.cTexture);

	    //gl.drawArrays(gl.TRIANGLE_STRIP, 0, 3dViewer.geometry[i].numItems);	
	    gl.drawArrays(gl.LINE_LOOP, 0, 3dViewer.geometry[i].numItems);	    
	}
        
}


3dViewer.drawClusters = function(evt) {
	
	if(!3dViewer.events) return;
	
	var gl = 3dViewer.gl;
    gl.lineWidth(2.0);
    
	var clusterColorBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, clusterColorBuffer);
	var colors = [];
    for (var i=0; i < 4; i++)
		 colors = colors.concat([1.0, 0.0, 0.0, 1.0]);    
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, clusterColorBuffer);
    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexColorAttribute, 4, gl.FLOAT, false, 0, 0);
    
    var lo = evt < 0?0:evt;
    var hi = evt < 0?3dViewer.events.length:(evt+1);
	for(var i=lo;i<hi && i<3dViewer.events.length;++i)
	{
		for(var c=0;c<3dViewer.events[i].clusters.length;++c) //clusters
		{
		    gl.bindBuffer(gl.ARRAY_BUFFER, 3dViewer.events[i].clusters[c]);
		    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexPositionAttribute, 
		    		3dViewer.events[i].clusters[c].itemSize, gl.FLOAT, false, 0, 0);    
		    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 3dViewer.events[i].clusters[c].numItems);	 
		}   
	}        	
}


3dViewer.drawTracks = function(evt) {
	
	if(!3dViewer.events) return;
	
	var gl = 3dViewer.gl;
    gl.lineWidth(2.0);
    
	var clusterColorBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, clusterColorBuffer);
	var colors = [];
    for (var i=0; i < 4; i++)
		 colors = colors.concat([1%2, 1.0, 0.0, 1.0]);    
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, clusterColorBuffer);
    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexColorAttribute, 2, gl.FLOAT, false, 0, 0);
    
    var lo = evt < 0?0:evt;
    var hi = evt < 0?3dViewer.events.length:(evt+1);
	for(var i=lo;i<hi && i<3dViewer.events.length;++i)
	{
	    gl.bindBuffer(gl.ARRAY_BUFFER, 3dViewer.events[i].tracks);
	    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexPositionAttribute, 
	    		3dViewer.events[i].tracks.itemSize, gl.FLOAT, false, 0, 0);    
	    gl.drawArrays(gl.LINES, 0, 3dViewer.events[i].tracks.numItems);	 
	}        	
}

var cubeVertexPositionBuffer;
var cubeVertexNormalBuffer;
var cubeVertexTextureCoordBuffer;
var cubeVertexIndexBuffer;

function initBuffers() {
	var gl = 3dViewer.gl;
    cubeVertexPositionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexPositionBuffer);
    vertices = [
        // Front face
        -1.0, -1.0,  1.0,
         1.0, -1.0,  1.0,
         1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,

        // Back face
        -1.0, -1.0, -1.0,
        -1.0,  1.0, -1.0,
         1.0,  1.0, -1.0,
         1.0, -1.0, -1.0,

        // Top face
        -1.0,  1.0, -1.0,
        -1.0,  1.0,  1.0,
         1.0,  1.0,  1.0,
         1.0,  1.0, -1.0,

        // Bottom face
        -1.0, -1.0, -1.0,
         1.0, -1.0, -1.0,
         1.0, -1.0,  1.0,
        -1.0, -1.0,  1.0,

        // Right face
         1.0, -1.0, -1.0,
         1.0,  1.0, -1.0,
         1.0,  1.0,  1.0,
         1.0, -1.0,  1.0,

        // Left face
        -1.0, -1.0, -1.0,
        -1.0, -1.0,  1.0,
        -1.0,  1.0,  1.0,
        -1.0,  1.0, -1.0,
    ];
    //for(var i = 0;i<vertices.length;++i)
    //	vertices[i] *= 1.0;//10000;
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    cubeVertexPositionBuffer.itemSize = 3;
    cubeVertexPositionBuffer.numItems = 24;

    cubeVertexNormalBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexNormalBuffer);
    var vertexNormals = [
        // Front face
         0.0,  0.0,  1.0,
         0.0,  0.0,  1.0,
         0.0,  0.0,  1.0,
         0.0,  0.0,  1.0,

        // Back face
         0.0,  0.0, -1.0,
         0.0,  0.0, -1.0,
         0.0,  0.0, -1.0,
         0.0,  0.0, -1.0,

        // Top face
         0.0,  1.0,  0.0,
         0.0,  1.0,  0.0,
         0.0,  1.0,  0.0,
         0.0,  1.0,  0.0,

        // Bottom face
         0.0, -1.0,  0.0,
         0.0, -1.0,  0.0,
         0.0, -1.0,  0.0,
         0.0, -1.0,  0.0,

        // Right face
         1.0,  0.0,  0.0,
         1.0,  0.0,  0.0,
         1.0,  0.0,  0.0,
         1.0,  0.0,  0.0,

        // Left face
        -1.0,  0.0,  0.0,
        -1.0,  0.0,  0.0,
        -1.0,  0.0,  0.0,
        -1.0,  0.0,  0.0
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertexNormals), gl.STATIC_DRAW);
    cubeVertexNormalBuffer.itemSize = 3;
    cubeVertexNormalBuffer.numItems = 24;

    cubeVertexTextureCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexTextureCoordBuffer);
    var textureCoords = [
        // Front face
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,

        // Back face
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
        0.0, 0.0,

        // Top face
        0.0, 1.0,
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,

        // Bottom face
        1.0, 1.0,
        0.0, 1.0,
        0.0, 0.0,
        1.0, 0.0,

        // Right face
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
        0.0, 0.0,

        // Left face
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(textureCoords), gl.STATIC_DRAW);
    cubeVertexTextureCoordBuffer.itemSize = 2;
    cubeVertexTextureCoordBuffer.numItems = 24;

    cubeVertexIndexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, cubeVertexIndexBuffer);
    var cubeVertexIndices = [
        0, 1, 2,      0, 2, 3,    // Front face
        4, 5, 6,      4, 6, 7,    // Back face
        8, 9, 10,     8, 10, 11,  // Top face
        12, 13, 14,   12, 14, 15, // Bottom face
        16, 17, 18,   16, 18, 19, // Right face
        20, 21, 22,   20, 22, 23  // Left face
    ];
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(cubeVertexIndices), gl.STATIC_DRAW);
    cubeVertexIndexBuffer.itemSize = 1;
    cubeVertexIndexBuffer.numItems = 36;
}

//http://delphic.me.uk/webgltext.html
function drawScene() {
	var gl = 3dViewer.gl;


    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexPositionBuffer);
    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexPositionAttribute, cubeVertexPositionBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexNormalBuffer);
    gl.vertexAttribPointer(3dViewer.shaderProgram.vertexNormalAttribute, cubeVertexNormalBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, cubeVertexTextureCoordBuffer);
    gl.vertexAttribPointer(3dViewer.shaderProgram.textureCoordAttribute, cubeVertexTextureCoordBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, 3dViewer.cTexture);
    gl.uniform1i(3dViewer.shaderProgram.samplerUniform, 0);
    var lighting = true;
    gl.uniform1i(3dViewer.shaderProgram.useLightingUniform, lighting);
    if (lighting) {
        gl.uniform3f(3dViewer.shaderProgram.ambientColorUniform, 0.2, 0.2, 0.2);

        var lightingDirection = [ -0.25, -0.25, -1 ];
        var adjustedLD = vec3.create();
        vec3.normalize(lightingDirection, adjustedLD);
        vec3.scale(adjustedLD, -1);
        gl.uniform3fv(3dViewer.shaderProgram.lightingDirectionUniform, adjustedLD);
        gl.uniform3f(3dViewer.shaderProgram.directionalColorUniform, 0.8, 0.8, 0.8);
    }

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, cubeVertexIndexBuffer);
    3dViewer.setMatrixUniforms();
    gl.drawElements(gl.TRIANGLES, cubeVertexIndexBuffer.numItems, gl.UNSIGNED_SHORT, 0);
}





