//=====================================================================================
//
//	Created Dec, 2012
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	DesktopWindow.js
//
//	This is the desktop code for the user interface for ots. ots is the DAQ
// 		and control software for the Fermi Strips Telescope.
//
//	The desktop consists of a dashboard and an arbitrary amount of windows
//
//=====================================================================================

if (typeof Debug == 'undefined') 
	console.log('ERROR: Debug is undefined! Must include Debug.js before Desktop.js');
	
if (typeof Desktop == 'undefined') 
	console.log('ERROR: Desktop is undefined! Must include Desktop.js before DesktopWindow.js');
else {

		
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	//define Desktop.createWindow to return window class	
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	Desktop.createWindow = function(id,z,name,subname,url,w,h,x,y) {
	
		if(false === (this instanceof Desktop.createWindow)) {
			//here to correct if called as "var v = Desktop.createDesktop();"
			//	instead of "var v = new Desktop.createDesktop();"
	        return new Desktop.createWindow(id,z,name,subname,url,w,h,x,y);
	    }
		
		//------------------------------------------------------------------
		//create private members variables ----------------------
		//------------------------------------------------------------------
				
		
		//all units in pixels unless otherwise specified
		
		var _defaultWindowMinWidth = 100;	
		var _defaultWindowMinHeight = 100;	
		
		var _defaultHeaderHeight = 30;  	
		var _defaultHeaderLeftMargin = 10;	
		var _defaultHeaderFontSize = 16;
		var _defaultButtonSize = 20;
		var _defaultButtonLeftMargin = 2;
		var _defaultButtonTopMargin = 1;
		
		var _defaultFrameBorder = 6;	
				
		var _name;
		var _subname;
		var _url = url;	
		var _id = id; //unique window id, cannot change! Used as link between html div container and object

		var _winhdr,_winfrm,_winfrmHolder;
		
		var _w,_h,_x,_y; //position and size members
		var _isMaximized = false;
		var _isMinimized = false;
		var _z = z;
		
		//------------------------------------------------------------------
		//create public members variables ----------------------
		//------------------------------------------------------------------
		this.windiv;
        
		//------------------------------------------------------------------
		//create PRIVATE members functions ----------------------
		//------------------------------------------------------------------
		//_refreshHeader()
		
			//_refreshHeader() ~~~
			//	private function to refresh header name based on window size
			// 	clip text if too long
		var _refreshHeader = function() {
			var hdrW = _w-2*_defaultHeaderLeftMargin-4*(_defaultButtonSize+_defaultButtonLeftMargin);
			_winhdr.style.width = hdrW +"px"; 
			_winhdr.innerHTML = _name + (_subname==""?"":" - ") + _subname;
			while(_winhdr.scrollWidth > hdrW && _winhdr.innerHTML.length > 4)
				_winhdr.innerHTML = _winhdr.innerHTML.substr(0,_winhdr.innerHTML.length-4)+"...";
		}
		
		var _handleWindowContentLoading = function() {
			//remove the "Loading" once iframe loades
			if(_winfrmHolder.childNodes.length > 1) 
				_winfrmHolder.removeChild(
						document.getElementById("DesktopWindowFrameLoadingDiv-"+_id));
		}
		//------------------------------------------------------------------
		//create PUBLIC members functions ----------------------
		//------------------------------------------------------------------
		//setWindowNameAndSubName
		//setWindowSizeAndPosition
		//moveWindowByOffset
		//resizeAndPositionWindow(x,y,w,h)
		//maximize
		//minimize
		
			//member variable access functions ~~~
		this.getWindowName = function() { return _name; }
		this.getWindowSubName = function() { return _subname; }
		this.getWindowUrl = function() { return _url; }
		this.getWindowId = function() { return _id; }
		this.getWindowX = function() { return _x; }
		this.getWindowY = function() { return _y; }
		this.getWindowZ = function() { return parseInt(this.windiv.style.zIndex);} //return integer
		this.getWindowWidth = function() { return _w; }
		this.getWindowHeight = function() { return _h; }
		this.getWindowHeaderHeight = function() { return _defaultHeaderHeight; }
		this.isMaximized = function() {return _isMaximized;}
		this.isMinimized = function() {return _isMinimized;}
				
		this.setWindowZ = function(z) { _z = z; this.windiv.style.zIndex = _z; }
		
		this.showFrame = function() { _winfrm.style.display = "inline"; }
		this.hideFrame = function() { _winfrm.style.display = "none"; }
		
			//setWindowNameAndSubName() ~~~
			//	set name and subname
		this.setWindowNameAndSubName = function(name,subname) {
			_name = name; _subname = subname;
			_refreshHeader();			
			Debug.log("Desktop Window name=" + name + " and subname=" + subname,Debug.LOW_PRIORITY);
		}
		
			//setWindowSizeAndPosition() ~~~
			//	set size and position of window and its elements
		this.setWindowSizeAndPosition = function(x,y,w,h) {		
			x = parseInt(x);
			y = parseInt(y);		
			w = parseInt(w);
			h = parseInt(h);
			
			//apply minimum size requirements and maximized state
			_w = _isMaximized?Desktop.desktop.getDesktopContentWidth():(w < _defaultWindowMinWidth?_defaultWindowMinWidth:w);
			_h = _isMaximized?Desktop.desktop.getDesktopContentHeight():(h < _defaultWindowMinHeight?_defaultWindowMinHeight:h);
			_x = _isMaximized?Desktop.desktop.getDesktopContentX():x;
			_y = _isMaximized?Desktop.desktop.getDesktopContentY():y;
			
			//keep window within desktop content bounds
			if(_x + _w > Desktop.desktop.getDesktopContentX() + Desktop.desktop.getDesktopContentWidth())
				_x = Desktop.desktop.getDesktopContentX() + Desktop.desktop.getDesktopContentWidth() - _w;
			if(_y + _h > Desktop.desktop.getDesktopContentY() + Desktop.desktop.getDesktopContentHeight())
				_y = Desktop.desktop.getDesktopContentY() + Desktop.desktop.getDesktopContentHeight() - _h;
			
            this.windiv.style.width = _w +"px";
		   	this.windiv.style.height = _h+"px"; 
			this.windiv.style.left = _x+"px"; 
		   	this.windiv.style.top = _y+"px";    	
		   		
			_refreshHeader();				
			
			_winfrm.style.width = (_w-2*_defaultFrameBorder-2)+"px";  //extra 2 for border pixels
			_winfrm.style.height = (_h-_defaultHeaderHeight-_defaultFrameBorder-2)+"px"; 	//extra 2 for border pixels		  
			_winfrmHolder.style.width = (_w-2*_defaultFrameBorder-2)+"px";  //extra 2 for border pixels 
			_winfrmHolder.style.height = (_h-_defaultHeaderHeight-_defaultFrameBorder-2)+"px"; 	//extra 2 for border pixels	
			
            //Debug.log("Desktop Window position to " + _x + "," +
            //           _y + " size to " + _w + "," + _h,Debug.LOW_PRIORITY);
					
            if(_isMaximized){ //keep proper dimensions
                _winfrm.style.position = "absolute";
                _winfrm.style.zIndex = _z + 1;
                _winfrm.style.width = _w + "px";
                _winfrm.style.height = _h + "px";
                _winfrm.style.left ="-1px";
                _winfrm.style.top = "-1px";
                _winfrmHolder.style.position = "absolute";
                _winfrmHolder.style.width = (_w)+"px";  //extra 2 for border pixels 
                _winfrmHolder.style.height = (_h)+"px"; 	//extra 2 for border pixels
                _winfrmHolder.style.left =(-_defaultFrameBorder-2) + "px";
                _winfrmHolder.style.top = "-1px";	
                			

                _w = w < _defaultWindowMinWidth?_defaultWindowMinWidth:w;
                _h = h < _defaultWindowMinHeight?_defaultWindowMinHeight:h;
                _x = x;
                _y = y;
                
                //hide window header (in case user page is transparent)
                var hdrs = this.windiv.getElementsByClassName("DesktopWindowButton");
                for(var h=0;hdrs && h<hdrs.length;++h)
                	hdrs[h].style.display = "none";
                hdrs = this.windiv.getElementsByClassName("DesktopWindowHeader");
                for(var h=0;hdrs && h<hdrs.length;++h)
					hdrs[h].style.display = "none";
            }
            else {
                _winfrm.style.zIndex = _z;
                _winfrm.style.position = "static";
                _winfrmHolder.style.position = "static";

                //show window header (for case user page is transparent)
                var hdrs = this.windiv.getElementsByClassName("DesktopWindowButton");
                for(var h=0;hdrs && h<hdrs.length;++h)
                	hdrs[h].style.display = "block";
                hdrs = this.windiv.getElementsByClassName("DesktopWindowHeader");
                for(var h=0;hdrs && h<hdrs.length;++h)
					hdrs[h].style.display = "block";
            }
			
            Desktop.desktop.login.resetCurrentLayoutUpdateTimer();
		}
			//moveWindowByOffset() ~~~
			//	move position of window and its elements by an offset
		this.moveWindowByOffset = function(dx,dy) {
			_x += dx;
			_y += dy;
            //if(_x < Desktop.desktop.getDesktopContentX()) _x = Desktop.desktop.getDesktopContentX();
            if(_y < Desktop.desktop.getDesktopContentY()) _y = Desktop.desktop.getDesktopContentY();
			this.windiv.style.left = _x+"px"; 
		   	this.windiv.style.top = _y+"px";    
		   	
			//Debug.log("Desktop Window position to " + _x + "," +
			//	_y ,Debug.LOW_PRIORITY);
		   	
		   	//reset current layout update timer if a window moves
		   	Desktop.desktop.login.resetCurrentLayoutUpdateTimer();
		}
			//resizeAndPositionWindow(x,y,w,h) ~~~
			//	resize and position of window and its elements
			//	do not allow weird re-size effects
		this.resizeAndPositionWindow = function(x,y,w,h) {		
			if((w <= _defaultWindowMinWidth && x > _x) ||
				(h <= _defaultWindowMinHeight && y > _y)) return;
            if(x < Desktop.desktop.getDesktopContentX()) x = Desktop.desktop.getDesktopContentX();
            if(y < Desktop.desktop.getDesktopContentY()) y = Desktop.desktop.getDesktopContentY();            
			this.setWindowSizeAndPosition(x,y,w,h);
		}
				
        	//maximize() ~~~
			//	maximize window toggle fulls screen mode
		this.maximize = function() {
		    _isMinimized = false; 
		    //if(_isMinimized) Desktop.desktop.toggleMinimize(); //untoggle minimize flag
		    _isMaximized = true;//!_isMaximized;
		    if(_isMaximized) //make sure is visible
			this.windiv.style.display = "inline";
		    this.setWindowSizeAndPosition(_x+10,_y,_w,_h);
		    window.parent.document.title= _name;
		    console.log(document.title, _name, "Maximize()");
			
		}

		this.unmaximize = function() {
		  
		    _isMaximized = false;
		    if(_isMaximized) //make sure is visible
			this.windiv.style.display = "inline";
		    this.setWindowSizeAndPosition(_x,_y,_w,_h);
		    window.parent.document.title = Desktop.isWizardMode()?"ots wiz":"ots";


		}

			//minimize() ~~~
			//	minimize window toggles visible or not (does not affect current position/size)
		this.minimize = function() {
		    //if(_isMaximized) Desktop.desktop.toggleFullScreen(); //untoggle minimize flag
		    if(_isMaximized)
			window.parent.document.title = _name;
		    else
			window.parent.document.title = Desktop.isWizardMode()?"ots wiz":"ots";
		    _isMinimized = true; //!_isMinimized;
		    this.windiv.style.display = "none";//_isMinimized?"none":"inline";
		    Debug.log("-----------Chat this.windiv.style.display now is " + this.windiv.style.display);
		}

		this.unminimize = function() {
		    //if(_isMaximized) Desktop.desktop.toggleFullScreen(); //untoggle minimize flag
		    if(_isMaximized)
			window.parent.document.title = _name;
		    else
			window.parent.document.title = Desktop.isWizardMode()?"ots wiz":"ots";
		   
		    _isMinimized = false;
		    this.windiv.style.display = "inline";
		    Debug.log("-----------Chat this.windiv.style.display now is " + this.windiv.style.display);
		}
                
		//------------------------------------------------------------------
		//handle class construction ----------------------
		//------------------------------------------------------------------
		
			//create holding container
		this.windiv = document.createElement("div"); 
		this.windiv.setAttribute("class", "DesktopWindow");   	
		this.windiv.setAttribute("id", "DesktopWindow-" + _id); //setup ids
		this.windiv.style.backgroundColor = Desktop.desktop.defaultWindowFrameColor;
	   	this.windiv.style.position = "absolute";
	   	this.windiv.style.zIndex = _z;
	   	
			//create header
		_winhdr = document.createElement("div"); 
		_winhdr.setAttribute("class", "DesktopWindowHeader");
		_winhdr.setAttribute("id", "DesktopWindowHeader-" + _id);	   		
	   	_winhdr.style.height = _defaultHeaderHeight+"px";
	   	_winhdr.style.marginLeft = _defaultHeaderLeftMargin+"px";
		_winhdr.style.fontSize = _defaultHeaderFontSize+"px"; 	 		
	   	this.setWindowNameAndSubName(name, subname); // set name and subname		   	
	   	this.windiv.appendChild(_winhdr); //add header to window	   	
	   	
			//create buttons
                var tmpBtn = document.createElement("div");
                tmpBtn.setAttribute("class", "DesktopWindowButton");
                tmpBtn.setAttribute("id", "DesktopWindowButtonRefresh-" + _id);
                tmpBtn.style.width = (_defaultButtonSize) +"px";
                tmpBtn.style.height = (_defaultButtonSize) +"px";
                tmpBtn.style.marginLeft = (_defaultButtonLeftMargin) +"px";
                tmpBtn.style.marginTop = (_defaultButtonTopMargin) +"px";
                tmpBtn.onmouseup = Desktop.handleWindowRefresh;
                tmpBtn.onmousedown = Desktop.handleWindowButtonDown;
                var tmpEl = document.createElement("div");
                tmpEl.setAttribute("class", "DesktopWindowButtonGraphicRefresh");
                tmpEl.innerHTML = "↻";
                tmpBtn.appendChild(tmpEl);
                this.windiv.appendChild(tmpBtn); //add button to window 

                var tmpBtn = document.createElement("div");
                tmpBtn.setAttribute("class", "DesktopWindowButton");
                tmpBtn.setAttribute("id", "DesktopWindowButtonMin-" + _id);
                tmpBtn.style.width = (_defaultButtonSize) +"px"; 
                tmpBtn.style.height = (_defaultButtonSize) +"px";                         
                tmpBtn.style.marginLeft = (_defaultButtonLeftMargin) +"px";         
                tmpBtn.style.marginTop = (_defaultButtonTopMargin) +"px";  
		tmpBtn.onmouseup = Desktop.handleWindowMinimize;         
                tmpBtn.onmousedown = Desktop.handleWindowButtonDown;
                var tmpEl = document.createElement("div");
                tmpEl.setAttribute("class", "DesktopWindowButtonGraphicMin");
                tmpBtn.appendChild(tmpEl);                
		this.windiv.appendChild(tmpBtn); //add button to window 

                tmpBtn = document.createElement("div");
                tmpBtn.setAttribute("class", "DesktopWindowButton");
                tmpBtn.setAttribute("id", "DesktopWindowButtonMax-" + _id);
                tmpBtn.style.width = (_defaultButtonSize) +"px"; 
                tmpBtn.style.height = (_defaultButtonSize) +"px";                         
                tmpBtn.style.marginLeft = (_defaultButtonLeftMargin) +"px";         
                tmpBtn.style.marginTop = (_defaultButtonTopMargin) +"px";  
		tmpBtn.onmouseup = Desktop.handleWindowMaximize;         
                tmpBtn.onmousedown = Desktop.handleWindowButtonDown;
                var tmpEl = document.createElement("div");
                tmpEl.setAttribute("class", "DesktopWindowButtonGraphicMax");
                tmpBtn.appendChild(tmpEl);                
		this.windiv.appendChild(tmpBtn); //add button to window   
		
		tmpBtn = document.createElement("div");
		tmpBtn.setAttribute("class", "DesktopWindowButton");
		tmpBtn.setAttribute("id", "DesktopWindowButtonClose-" + _id);
		tmpBtn.style.width = (_defaultButtonSize) +"px"; 
		tmpBtn.style.height = (_defaultButtonSize) +"px"; 			
		tmpBtn.style.marginLeft = (_defaultButtonLeftMargin) +"px"; 	
		tmpBtn.style.marginTop = (_defaultButtonTopMargin) +"px";  
	   	tmpBtn.onmouseup = Desktop.handleWindowClose; 	
		tmpBtn.onmousedown = Desktop.handleWindowButtonDown;
		var tmpEl = document.createElement("div");
		tmpEl.setAttribute("class", "DesktopWindowButtonGraphicClose");
		tmpEl.innerHTML = "x";
		tmpBtn.appendChild(tmpEl);		
	   	this.windiv.appendChild(tmpBtn); //add button to window	     
		  	
	   		//create iframe holder for displaying during window manipulations
	   	_winfrmHolder = document.createElement("div"); 
		_winfrmHolder.setAttribute("class", "DesktopWindowFrameHolder");
		_winfrmHolder.setAttribute("id", "DesktopWindowFrameHolder-" + _id);
		_winfrmHolder.style.marginLeft = _defaultFrameBorder+"px";	
		_winfrmHolder.innerHTML = 
				"<div class='DesktopWindowHeader' id='DesktopWindowFrameLoadingDiv-"+
				_id + "' style='width:100px;height:50px;position:relative;top:50%;left:50%;margin-top:-25px;margin-left:-50px;text-align:center;margin-bottom:-50px;'>" + 
				"Loading..." + "</div>";
		
	   		//create iframe
	   	_winfrm = document.createElement("iframe"); 
		_winfrm.setAttribute("class", "DesktopWindowFrame");
		_winfrm.setAttribute("id", "DesktopWindowFrame-" + _id);
		_winfrm.setAttribute("name", "DesktopWindowFrame-" + _id);	   			
		_winfrm.onload = _handleWindowContentLoading; //event to delete "Loading"
		_winfrm.setAttribute("src", _url);
		_winfrmHolder.appendChild(_winfrm); //add frame to holder	 
		this.windiv.appendChild(_winfrmHolder); //add holder to window  
		
		this.setWindowSizeAndPosition(x,y,w,h); //setup size and position
			   	   	
	   		//add mouse handlers
	   	this.windiv.onmousedown = Desktop.handleWindowMouseDown;
	   	this.windiv.onmouseup   = Desktop.handleWindowMouseUp;
	   	this.windiv.onmousemove = Desktop.handleWindowMouseMove;
		this.windiv.ondblclick  = Desktop.handleWindowMaximize;
	   	
	   		//add touch handlers (for mobile devices)
	   	this.windiv.addEventListener('touchstart',Desktop.handleTouchStart);
	   	this.windiv.addEventListener('touchend',Desktop.handleTouchEnd);
	   	this.windiv.addEventListener('touchmove',Desktop.handleTouchMove);
	   		   	
		Debug.log("Desktop Window Created",Debug.LOW_PRIORITY);
	
	}
}