//=====================================================================================
//
//	Created Dec, 2012
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	Desktop.js
//
//	This is the desktop code for the user interface for ots. ots is the DAQ
// 		and control software for the Fermi Strips Telescope.
//
//	The desktop consists of a dashboard and an arbitrary amount of windows
//
//=====================================================================================

var Desktop = Desktop || {}; //define Desktop namespace

if (typeof Debug == 'undefined') 
	console.log('ERROR: Debug is undefined! Must include Debug.js before Desktop.js');
else if (typeof Globals == 'undefined')
    console.log('ERROR: Globals is undefined! Must include Globals.js before Desktop.js');
else
	Desktop.desktop; //this is THE global desktop variable

Desktop.init = function(security) {
	
	Desktop.desktop = Desktop.createDesktop(security);
	if(Desktop.desktop)
		Debug.log("Desktop.desktop Initalized Successfully",Debug.LOW_PRIORITY);
}

Desktop.SECURITY_TYPE_NONE = "NoSecurity";
Desktop.SECURITY_TYPE_DIGEST_ACCESS = "DigestAccessAuthentication";
		
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//call createDesktop to create instance of a desktop
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
Desktop.createDesktop = function(security) {	
	
	if (typeof Debug == 'undefined') return 0; //fail if debug not defined just to force consistent behavior
	
	if(false === (this instanceof Desktop.createDesktop)) {
		//here to correct if called as "var v = Desktop.createDesktop();"
		//	instead of "var v = new Desktop.createDesktop();"
        return new Desktop.createDesktop(security);
    }
	

	//------------------------------------------------------------------
	//list of members functions ----------------------
	//------------------------------------------------------------------
	//private:
	//public:
	
	//------------------------------------------------------------------
	//create private members variables ----------------------
	//------------------------------------------------------------------
	
	var _defaultWindowMinZindex = 10;	
	var _defaultWindowMaxZindex	= 10000;
	var _defaultDashboardZindex	= 11000;
	var _defaultWidth = 620;
	var _defaultHeight = 580;
	var _defaultLeft = 200;
	var _defaultTop = 70;
	var _defaultLeftOffset = 50;
	var _defaultTopOffset = 25;
	var _defaultOffsetTimes = 5;
	var _currentLeft = _defaultLeft;
	var _currentTop = _defaultTop;
	
	var _windows = new Array(); //windows are initialized to empty, array represents z-depth also
	var _desktopElement;
    var _dashboard, _icons, _windowZmailbox, _mouseOverXmailbox, _mouseOverYmailbox;
    var _needToLoginMailbox, _updateTimeMailbox, _updateSettingsMailbox, _settingsLayoutMailbox, _openWindowMailbox;
    var _windowColorPostbox;
    var _MAILBOX_TIMER_PERIOD = 500; //timer period for checking mailbox and system messages: 500 ms
    var _sysMsgId = 0; //running counter to identify system message pop-ups
	var _SYS_MSG_SOUND_PATH = "/WebPath/sounds/fx-System-Message.wav"; // "http://www.soundjay.com/button/button-2.wav"; //must be .wav for firefox incompatibility	
    var _sysMsgSound = new Audio(_SYS_MSG_SOUND_PATH);
	
	var _winId = 1000; //running window id number
			
	var _login;
	
	//for system messages
	var _sysMsgCounter = 0;
	var _SYS_MSG_MAX_COUNT = 10; //number of check mailbox timers to count through before checking for system messages
	
	
	//------------------------------------------------------------------
	//create public members variables ----------------------
	//------------------------------------------------------------------
	this.dashboard;
	this.login;    
	this.icons;   
	this.checkMailboxTimer;
	this.serverConnected = true;
	this.security = security;
    
	this.defaultWindowFrameColor = "rgba(196,229,255,.9)";

	
	//------------------------------------------------------------------
	//create PRIVATE members functions ----------------------
	//------------------------------------------------------------------
	var _handleDesktopResize = function(event) {
		_desktopElement.style.height = (window.innerHeight-_desktopElement.offsetTop) + "px";
		_desktopElement.style.width = (window.innerWidth-_desktopElement.offsetLeft) + "px";
        if(!_dashboard) return; //initial calls dashboard might not be defined
        _dashboard.redrawDashboard();
        if(_login) _login.redrawLogin();
        if(_icons) _icons.redrawIcons();
         
        //if top windows is maximized, then resize
        if(_windows.length && _windows[_windows.length-1].isMaximized()) {
            var w = _windows[_windows.length-1];
            w.setWindowSizeAndPosition(w.getWindowX(),w.getWindowY(),w.getWindowWidth(),w.getWindowHeight());
        }
        
        //_icons.style.left = Desktop.desktop.getDesktopContentX()+50+"px";
        //_icons.style.top = Desktop.desktop.getDesktopContentY()+50+"px";
	}
	
	//return current window layout in string with parameters separated by commas
	var _getWindowLayoutStr = function() {		
		var layout = "[";				
		for(var i=0;i<_windows.length;++i) {		
			if(_windows[i].getWindowName() == "Settings") continue; //skip settings
			layout += _windows[i].getWindowName() 
										+ "," + _windows[i].getWindowSubName() 
										+ "," + _windows[i].getWindowUrl().replace(/&/g,'%38').replace(/=/g,'%61')  //global replace & and =
										+ "," + _windows[i].getWindowX()
										+ "," + _windows[i].getWindowY()
										+ "," + _windows[i].getWindowWidth()
										+ "," + _windows[i].getWindowHeight()
										+ "," + (_windows[i].isMinimized()?"0":"1")
										+ ", "; //last comma (with space for settings display)
		}
		layout += "]";
		return layout;
	}
	
	//for login
	var _scrambleEggs = function(u) { return u; }
	
    var _getForeWindow = function() { return _windows.length?_windows[_windows.length-1]:0; } //return last window in array as forewindow
    
    var _closeWindow = function(win) {
    	Desktop.desktop.setForeWindow(win);
		win.windiv.parentNode.removeChild(win.windiv); //remove from page!
		
		//delete top window from windows array
		//		Debug.log("Desktop Window closed z:" + _windows[_windows.length-1].getWindowZ(),Debug.LOW_PRIORITY);
		_windows.splice(_windows.length-1,1);
		//		Debug.log("Desktop Windows left:" + _windows.length,Debug.LOW_PRIORITY);
        
        _dashboard.updateWindows();
    }
    
    //===========================================================
	//_checkMailboxes ~~~
	//	called periodically (e.g. every _MAILBOX_TIMER_PERIODms)
    //  check div mailboxes that are shared by window content code and take action if necessary
    //	check for settings change
	var _checkMailboxes = function(win) 
	{		
		//Debug.log("_checkMailboxes sysMsgCounter=" +_sysMsgCounter);
		
		

	    //check _openWindowMailbox to see if a window opening is being requested by a Desktop Window
	    //	From requesting window (check that done=1):
	    // 		innerHTML = requestingWindowId=<window uid>&windowPath=<window path to open>
	    //	Response when done:
	    //		innerHTML = requestingWindowId=<window uid>&done=1
	    if(_openWindowMailbox.innerHTML != "")
	    {
	    	Debug.log("_openWindowMailbox.innerHTML=" + _openWindowMailbox.innerHTML);
	    	
	    	//get parameters
	    	var paramsStr = _openWindowMailbox.innerHTML;
	    	var params = [];
	    	var paramCnt = 5;
	    	var spliti, splitiOld = 0; 
	    	for(var i=0;i<paramCnt;++i)
	    	{
	    		if(i == paramCnt-1) //last one take the whole thing (this is path, and could have &'s in it)
	    		{
	    			params.push(paramsStr.substr(splitiOld));
	    			break;
	    		}
	    		//for others, handle like normal get param
	    		spliti = paramsStr.indexOf('&amp;', splitiOld);	    		
	    		params.push(paramsStr.substr(splitiOld,spliti-splitiOld))
				splitiOld = spliti+5;
	    	}
	    	
	    	var varPair;
	    	var requestingWindowId = "", windowPath = "";
	    	var windowName, windowSubname, windowUnique;
	    	for(var i=0;i<params.length;++i)
	    	{
	    		spliti = params[i].indexOf('=');
	    		varPair = [params[i].substr(0,spliti),params[i].substr(spliti+1)];	    		
				if(varPair[0] 		== "requestingWindowId")
					requestingWindowId 	= varPair[1];
				else if(varPair[0] 	== "windowPath")
					windowPath 			= varPair[1];	
				else if(varPair[0] 	== "windowName")
					windowName 			= varPair[1];	
				else if(varPair[0] 	== "windowSubname")
					windowSubname		= varPair[1];	
				else if(varPair[0] 	== "windowUnique")
					windowUnique 		= varPair[1];	 
	    	}
	    	if(requestingWindowId != "" && windowPath != "")
	    	{
	    		//have work to do!
	    		Debug.log("_openWindowMailbox.innerHTML=" + _openWindowMailbox.innerHTML);
		    	Debug.log("requestingWindowId=" + requestingWindowId);
		    	Debug.log("windowPath=" + windowPath);
		    	Debug.log("windowName=" + windowName);
		    	Debug.log("windowSubname=" + windowSubname);
		    	Debug.log("windowUnique=" + windowUnique);

		    	var newWin = Desktop.desktop.addWindow(	//(name,subname,url,unique)
		    			windowName, 
						windowSubname,
						windowPath,		//e.g. "http://rulinux03.dhcp.fnal.gov:1983/WebPath/html/ConfigurationGUI.html?urn=280",						
						eval(windowUnique));			
		    			

		    	//delate the fore window
				setTimeout(function(){ Desktop.desktop.setForeWindow(newWin); }, 200);
		    	
				var str = "requestingWindowId=" + requestingWindowId;
				str += "&done=1";	
				_openWindowMailbox.innerHTML = str; //indicate done
	    	}
	    }
		
		//other things besides opening windows
	    if(!Desktop.desktop.login || !Desktop.desktop.login.getCookieCode(true)) return; //don't do things if not through login

	    //	check if a window iFrame has taken focus and tampered with z mailbox. If so 'officially' set to fore window
		if(_windowZmailbox.innerHTML > _defaultWindowMaxZindex) 
		{
			Desktop.desktop.setForeWindow(0); //use function just to standardize, do not change current foreground			
			//Debug.log("Desktop Foreground Window Refreshed by Timeout",Debug.LOW_PRIORITY);
		}
	    
	    //check need for login mailbox
	    if(_needToLoginMailbox.innerHTML == "1") 
	    {
	        _needToLoginMailbox.innerHTML = ""; //reset
			Debug.log("DesktopContent signaled new login needed!",Debug.HIGH_PRIORITY);
	        Desktop.logout();
		}
	    
	    //check if cookie time from content is newer than cookie time in login
	    if(parseInt(_updateTimeMailbox.innerHTML) > parseInt(Desktop.desktop.login.getCookieTime()))
	        Desktop.desktop.login.updateCookieFromContent(parseInt(_updateTimeMailbox.innerHTML)); //update based on content value
	        
	    //check if update settings is necessary
	    if(_updateSettingsMailbox.innerHTML != "") 
	    { 
	    	//this mailbox defines read/write actions between settings dialog and desktop
	    	
			//Debug.log("Desktop Settings update " + _updateSettingsMailbox.innerHTML ,Debug.LOW_PRIORITY);
			
			if(_updateSettingsMailbox.innerHTML == "LAYOUT") //settings is reading from desktop
			{
				//return current window layout in mailbox with parameters separated by commas
				var layout = _getWindowLayoutStr();
				_settingsLayoutMailbox.innerHTML = layout;
				Debug.log("Desktop _updateSettingsMailbox " + layout,Debug.LOW_PRIORITY);
			}
			else //settings is writing to destkop
			{			
				//setup req with settings data
				var xml = _updateSettingsMailbox.innerHTML;
				var req = req || {};
				req.responseXML = ( new window.DOMParser() ).parseFromString(xml, "text/xml"); //get xml object
				_login.applyUserPreferences(req);
			}
			
			_updateSettingsMailbox.innerHTML = ""; //clear to prepare for next time
	    }
	    
	    //system messages check (and submit current window layout)
	    ++_sysMsgCounter;
		if(_sysMsgCounter == _SYS_MSG_MAX_COUNT)
		{  		
			Desktop.XMLHttpRequest("Request?RequestType=getSystemMessages","",_handleSystemMessages);
		}
	}
	
	//===========================================================
	var _lastSystemMessage = ""; //prevent repeats
	//_handleSystemMessages ~~~
	//	handles request returns periodically (ever _SYS_MSG_MAX_COUNT times through _checkMailboxes)
	var _handleSystemMessages = function(req) {
		Debug.log("Desktop _handleSystemMessages " + req.responseText,Debug.LOW_PRIORITY);
		
		_sysMsgCounter = 0; //reset system message counter to setup next request

		var userLock; //tmp hold user with lock
		userLock = Desktop.getXMLValue(req,"username_with_lock"); //get user with lock
		Desktop.desktop.dashboard.displayUserLock(userLock);
		
		var tmp = Desktop.getXMLValue(req,"systemMessages");
	    if(!tmp) return; //did not find return string	    
		
	    //disallow repeats (due to broadcast messages hanging around)
    	//if(_lastSystemMessage == tmp) return;    	
	    //_lastSystemMessage = tmp;	    
	    var tmpi;
	    if((tmpi = tmp.indexOf(_lastSystemMessage)) >= 0)
	    {
	    	Debug.log("Desktop pretmp " + tmp);
	    	Debug.log("Desktop _lastSystemMessage " + _lastSystemMessage);
	    	Debug.log("Desktop tmp " + tmp.substr(tmpi+_lastSystemMessage.length+1));
	    	tmp = tmp.substr(tmpi+_lastSystemMessage.length+1);
	    }
	   
			//Debug.log("Desktop tmp " + tmp.substr(tmpi+_lastSystemMessage.length+1));
	    
    	
	    var msgArr = tmp.split("|");	    
    	Debug.log("Desktop msgArr.length " + msgArr.length);
	    
	    if(msgArr.length < 2) return; //no new messages left
	    
	    Debug.log("Desktop _handleSystemMessages ========================== " + Desktop.formatTime(msgArr[0]) +
				" - " + msgArr[1]
				,Debug.LOW_PRIORITY);
				
		++_sysMsgId; //increment to new ID
		var sysMsgEl = document.createElement("div");
		sysMsgEl.setAttribute("class", "Desktop-systemMessageBox");
		sysMsgEl.setAttribute("id", "Desktop-systemMessageBox-" + _sysMsgId);
		sysMsgEl.style.left = (50 + _sysMsgId % 5 * 10) + "px";
		sysMsgEl.style.top = (50 + _sysMsgId % 5 * 10) + "px";
	    _desktopElement.appendChild(sysMsgEl);
	    
	    var str = "";
	    
	    for(var i=0;i<msgArr.length;i+=2)
	    {
		    str += "<div style='font-size:12px'>System Message Received at " + Desktop.formatTime(msgArr[i]) + "</div>";
		    str += "<div>" + msgArr[i+1] + "</div><br>";	
		    _lastSystemMessage = msgArr[i] + "|" + msgArr[i+1];
	    }
	    
	    //dismiss link
	    str += "<div style='float:right; margin-left:30px'>";
	    str += "<a href='Javascript:Desktop.closeSystemMessage(" + _sysMsgId + ");' " +
	    		"title='Click here to dismiss system message'>Dismiss</a></div>";
	    	    
	    sysMsgEl.innerHTML = str;
	    
	    if(sysMsgEl.clientHeight > 300)
	    	sysMsgEl.style.height = 300 + "px";

	    //play sound alert
	    _sysMsgSound.src = _SYS_MSG_SOUND_PATH; // buffers automatically when created
	    _sysMsgSound.play();   	
	   
	}
	
	//------------------------------------------------------------------
	//create PUBLIC members functions ----------------------
	//------------------------------------------------------------------
    this.getDesktopWidth = function() { return _desktopElement.clientWidth;}
    this.getDesktopHeight = function() { return _desktopElement.clientHeight;}
    this.getDesktopX = function() { return _desktopElement.offsetLeft;}
    this.getDesktopY = function() { return _desktopElement.offsetTop;}
    this.getDesktopContentX = function() { return _desktopElement.offsetLeft+_dashboard.getDashboardWidth();}
    this.getDesktopContentY = function() { return _desktopElement.offsetTop+_dashboard.getDashboardHeight();}
    this.getDesktopContentWidth = function() { return _desktopElement.clientWidth-_dashboard.getDashboardWidth();}
    this.getDesktopContentHeight = function() { return _desktopElement.clientHeight-_dashboard.getDashboardHeight();}
    this.getNumberOfWindows = function() { return _windows.length;}
    this.getWindowNameByIndex = function(i) { return _windows[i].getWindowName();}
    this.getWindowSubNameByIndex = function(i) { return _windows[i].getWindowSubName();}
    this.getWindowByIndex = function(i) { return _windows[i];}      
	this.getForeWindow = _getForeWindow;
    this.redrawDesktop = _handleDesktopResize;
    this.getLastFrameMouseX = function() { return parseInt(_mouseOverXmailbox.innerHTML);} 
    this.getLastFrameMouseY = function() { return parseInt(_mouseOverYmailbox.innerHTML);} 
    this.resetFrameMouse = function() { _mouseOverXmailbox.innerHTML = -1;_mouseOverYmailbox.innerHTML = -1;} 
	this.getWindowLayoutStr = _getWindowLayoutStr;
	
		//addWindow ~~~
		//	Adds a window to desktop at default location, with default size
		// 	Window title bar will read "name - subname" and will be organized by name
		//	in dashboard. Window will display page at url.
		//	If unique, the window is not made if there already exists a window
		//	with the same "name - subname".
		// 	
		//	If subname = "" it is ignored. 	
		//
		//  returns new window
	this.addWindow = function(name,subname,url,unique) {
		
		Debug.log(name + " - " + subname + " - " + url + " - " + unique,Debug.LOW_PRIORITY);
		
		if(unique) {
			Debug.log("Adding window uniquely",Debug.LOW_PRIORITY);
			for(var i=0;i<_windows.length;++i)
				if(_windows[i].getWindowName() == name && _windows[i].getWindowSubName() == subname) {
					Debug.log("Window creation failed. Not unique.",Debug.LOW_PRIORITY);
                    this.setForeWindow(_windows[i]); //bring window to front
					return;
				}
		}
		
		if(_windows.length + _defaultWindowMinZindex >= _defaultWindowMaxZindex) {
			Debug.log("FAILED -- Desktop Window Added - too many windows!",Debug.HIGH_PRIORITY);
			return;
		}
				
        //subname += _winId; //for visual window debugging (but destroys uniqueness)
		var newWin = Desktop.createWindow(_winId++,_windows.length + _defaultWindowMinZindex,name,subname,url,
			_defaultWidth,_defaultHeight,_dashboard.getDashboardWidth() + _currentLeft,_currentTop);

		//handle initial window left,top evolution
		if(_currentLeft > _defaultLeft+_defaultOffsetTimes*_defaultLeftOffset) {
			_currentLeft = _defaultLeft;
			if(_currentTop > _defaultTop + (_defaultOffsetTimes+1)*_defaultTopOffset)
				_currentTop = _defaultTop;
			else
				_currentTop = _defaultTop + _defaultTopOffset;
		}
		else
		{
			_currentLeft += _defaultLeftOffset;
			_currentTop += _defaultTopOffset;
		}
		
		_windows.push(newWin); //add to windows array
        
	   	_desktopElement.appendChild(newWin.windiv); //add to desktop element and show on screen!
	   	
		Debug.log("Desktop Window Added with id " + _windows[_windows.length-1].getWindowId(),Debug.LOW_PRIORITY);
        
        _dashboard.updateWindows();
        
        return newWin;
	}
	
		//getWindowById ~~~
		//	Find window by id
	this.getWindowById = function(id) {
		for(var i=0;i<_windows.length;++i) {
			if(_windows[i].getWindowId() == id) return _windows[i];
		}		
		return -1;
	}

		//setForeWindow ~~~
		//	handle bringing window to front
	this.setForeWindow = function(win) {
		
		//resort by z and renumber - windows with Z out of range of array are due to iframe onFocus solution			
        var tmp;
        for(var i=0;i<_windows.length-1;++i) {        	
            var min = i;
            for(var j=i+1;j<_windows.length;++j) 
                if(_windows[j].getWindowZ() < _windows[min].getWindowZ()) 
                    min = j;
                    
            //have min, swap to i
            tmp = _windows[i];
            _windows[i] = _windows[min];
            _windows[min] = tmp;
            _windows[i].setWindowZ(i+_defaultWindowMinZindex); //done with new window i
        }
        _windows[_windows.length-1].setWindowZ(_windows.length-1+_defaultWindowMinZindex); // last window still needs z-fixed
		_windowZmailbox.innerHTML = _defaultWindowMaxZindex;        //reset windowZmailbox for next set of foci
		//at this point windows are in standard Z arrangement
					
		//find win in windows array then bring to "top"
		var found = 0;
		for(var i=0;win && i<=_windows.length;++i) {	//only search, if win is valid (if not this function was likely called by timer watchdog _checkMailboxes())
			if(found) //copy each window down within windows array
			{
				var winToMov = i<_windows.length?_windows[i]:win; //if to the end, put the win in question
				winToMov.setWindowZ(i-1 + _defaultWindowMinZindex); //standardize window Z
				_windows[i-1] = winToMov;
			}
			else if(_windows[i] == win) found = 1; //found!
		}
		
		if(!found && win) Debug.log("FAIL -- Desktop Foreground window not Found!! Window: " + win.getWindowSubName(),Debug.HIGH_PRIORITY);
		
		_dashboard.updateWindows();
		
		//if(win) Debug.log("Desktop Window Set to Foreground named " + win.getWindowSubName(),Debug.LOW_PRIORITY);
		//else  Debug.log("Desktop Foreground Window with no parameter",Debug.LOW_PRIORITY);
	}
	
		//closeWindowById ~~~
		//	Find window by id
	this.closeWindowById = function(id) {
		var win = this.getWindowById(id);
		if(win == -1) return -1;
		_closeWindow(win);		
	}
    
    	//maximizeWindowById ~~~
		//	Find window by id
	this.maximizeWindowById = function(id) {
        var win = this.getWindowById(id);
		if(win == -1) return -1;
		this.setForeWindow(win);
        this.toggleFullScreen();
    }
    
        //toggleFullScreen ~~~
        //	Toggle current top window full screen (can be called as event)
    this.toggleFullScreen = function(e) {
        if(!_getForeWindow()) return;
        _getForeWindow().maximize();
        _dashboard.redrawFullScreenButton();
        Debug.log("Full Screen Toggled",Debug.LOW_PRIORITY);
    }
    
        //minimizeWindowById ~~~
		//	Find window by id
	this.minimizeWindowById = function(id) {
        var win = this.getWindowById(id);
		if(win == -1) return -1;
        
		this.setForeWindow(win);
        this.toggleMinimize();
    }
    
        //toggleMinimize ~~~
        //	Toggle current top window minimize functionality (can be called as event)
    this.toggleMinimize = function(e) {
        if(!_getForeWindow()) return;
        _getForeWindow().minimize();
        Debug.log("Minimize Toggled",Debug.LOW_PRIORITY);
    }
        //clickedWindowDashboard ~~~
		//	Handle window selection using dashboard
	this.clickedWindowDashboard = function(id) {
        var win = this.getWindowById(id);
		if(win == -1) return -1;
        if(_getForeWindow() != win) { //if not currently foreground window, set as only
            if(_getForeWindow().isMaximized()) this.toggleFullScreen(); //if old foreground is full screen, toggle
            this.setForeWindow(win);
            if(_getForeWindow().isMinimized()) this.toggleMinimize(); //if new foreground is minimized, toggle
            return;
        }
        //else minimize
        this.toggleMinimize();
    }
    
	 	//setDefaultWindowColor() ~~~
		//	set background color for all windows
	this.setDefaultWindowColor = function(color) {
		this.defaultWindowFrameColor = color;
	    _windowColorPostbox.innerHTML = this.defaultWindowFrameColor; //set to color string
		
		for(var i=0;i<_windows.length;++i)
			_windows[i].windiv.style.backgroundColor = this.defaultWindowFrameColor;
	}

	 	//defaultLayoutSelect() ~~~
		//	set default layout of windows
		//	0-1 are system defaults
		//	3-5 are user defaults
		//	6 is last saved layout checkpoint 
	this.defaultLayoutSelect = function(i) {
		Debug.log("Desktop defaultLayoutSelect " + i,Debug.LOW_PRIORITY);
			
		var layoutStr;
	  	if(i >= 3 && i <= 6) //user default or current checkpoint
	  		layoutStr = _login.getUserDefaultLayout(i-3);
	  	else if(i >= 0 && i <= 1) //system defaults
	  		layoutStr = _login.getSystemDefaultLayout(i);
	  	else //invalid
	  		return;
		var layoutArr = layoutStr.split(",");
		var numOfFields = ((layoutArr.length-1)%8==0)?8:7; //hack to be backwards compatible with 7 fields (new way adds the 8th field for isMinimized)
		var numOfWins = parseInt(layoutArr.length/numOfFields);
		Debug.log("Desktop defaultLayoutSelect layout numOfFields=" + numOfFields);
		Debug.log("Desktop defaultLayoutSelect layout " + numOfWins + " windows - " + layoutStr,Debug.LOW_PRIORITY);	
		
		//clear all current windows
		Desktop.desktop.closeAllWindows();
		
		//open default layout
		for(i=0;i<numOfWins;++i) {		
			Debug.log("adding " + layoutArr[i*numOfFields].substr(1) + "-" + layoutArr[i*numOfFields+1],Debug.LOW_PRIORITY);	
			this.addWindow(	//(name,subname,url,unique)
				layoutArr[i*numOfFields].substr(1), 
				layoutArr[i*numOfFields+1],
				layoutArr[i*numOfFields+2].replace(/%38/g,"&").replace(/%61/g,"="), //replace back = and &
				false);				
			_windows[_windows.length-1].setWindowSizeAndPosition(		//(x,y,w,h)		
				layoutArr[i*numOfFields+3],
				layoutArr[i*numOfFields+4],
				layoutArr[i*numOfFields+5],
				layoutArr[i*numOfFields+6]);
			if(numOfFields == 8 && 
					!(layoutArr[i*numOfFields+7]|0)) //convert to integer, if 0 then minimize
				_windows[_windows.length-1].minimize();
		}	  	
	}
	
 	//closeAllWindows() ~~~
 	// close all windows is used when default layout is changed or a new user logs in
	this.closeAllWindows = function() { 
		Debug.log("Desktop closeAllWindows",Debug.LOW_PRIORITY);	
		//clear all current windows
		while(_windows.length) _closeWindow(_windows[_windows.length-1]);
	}

 	//resetDesktop() ~~~
 	// called by successful login to reset desktop based on user's permissions
	this.resetDesktop = function(permissions) {
        
		_needToLoginMailbox.innerHTML = ""; //reset mailbox
		//update icons based on permissions
		Desktop.desktop.icons.resetWithPermissions(permissions);
		
		//re-start timer for checking foreground window changes due to iFrame content code
		window.clearInterval(Desktop.desktop.checkMailboxTimer);
		Desktop.desktop.checkMailboxTimer = setInterval(_checkMailboxes,_MAILBOX_TIMER_PERIOD);
	}
		
	//------------------------------------------------------------------
	//handle class construction ----------------------
	//------------------------------------------------------------------
	
	//create a div just to direct style properly to contents of Desktop
	_desktopElement = document.createElement("div");
	_desktopElement.setAttribute("id", "Desktop");
	document.body.appendChild(_desktopElement);
	document.body.onmousemove = Desktop.handleBodyMouseMove;
	window.onmouseup = Desktop.handleWindowMouseUp; //added to fix firefox mouseup glitch with window moving	
	document.body.addEventListener('touchmove',Desktop.handleBodyTouchMove);
	document.body.addEventListener('touchend',Desktop.handleBodyTouchEnd);
	window.onresize = _handleDesktopResize;
	Desktop.desktop = this;
	
	//create windowZmailbox for focus switching through iFrame
    _windowZmailbox = document.createElement("div");
	_windowZmailbox.setAttribute("id", "Desktop-windowZmailbox");
	_windowZmailbox.innerHTML = _defaultWindowMaxZindex;
	_windowZmailbox.style.display = "none";
    _desktopElement.appendChild(_windowZmailbox);
    
	//create mouseOverXmailbox for focus switching through iFrame
    _mouseOverXmailbox = document.createElement("div");
	_mouseOverXmailbox.setAttribute("id", "Desktop-mouseOverXmailbox");
	_mouseOverXmailbox.style.display = "none";
    _desktopElement.appendChild(_mouseOverXmailbox);
    
	//create mouseOverYmailbox for focus switching through iFrame
    _mouseOverYmailbox = document.createElement("div");
	_mouseOverYmailbox.setAttribute("id", "Desktop-mouseOverYmailbox");
	_mouseOverYmailbox.style.display = "none";
    _desktopElement.appendChild(_mouseOverYmailbox);
    this.resetFrameMouse();
    
    //create mailboxes for Window content code
    var tmpHiddenDiv = document.createElement("div");
    tmpHiddenDiv.setAttribute("id", "DesktopContent-cookieCodeMailbox");
    tmpHiddenDiv.style.display = "none";
    _desktopElement.appendChild(tmpHiddenDiv);
    _updateTimeMailbox = document.createElement("div");
    _updateTimeMailbox.setAttribute("id", "DesktopContent-updateTimeMailbox");
    _updateTimeMailbox.style.display = "none";
    _desktopElement.appendChild(_updateTimeMailbox);
    _needToLoginMailbox = document.createElement("div");
    _needToLoginMailbox.setAttribute("id", "DesktopContent-needToLoginMailbox");
    _needToLoginMailbox.style.display = "none";
    _desktopElement.appendChild(_needToLoginMailbox);   
    
    //create mailbox for opening windows from Desktop Windows    
    _openWindowMailbox = document.createElement("div");
    _openWindowMailbox.setAttribute("id", "DesktopContent-openWindowMailbox");
    _openWindowMailbox.style.display = "none";
    _desktopElement.appendChild(_openWindowMailbox);   
    
    //create mailbox for settings update
    _updateSettingsMailbox = document.createElement("div");
    _updateSettingsMailbox.setAttribute("id", "DesktopContent-updateSettingsMailbox");
    _updateSettingsMailbox.style.display = "none";
    _updateSettingsMailbox.innerHTML = ""; //init to empty
    _desktopElement.appendChild(_updateSettingsMailbox);
    _settingsLayoutMailbox = document.createElement("div");
    _settingsLayoutMailbox.setAttribute("id", "DesktopContent-settingsLayoutMailbox");
    _settingsLayoutMailbox.style.display = "none";
    _settingsLayoutMailbox.innerHTML = ""; //init to empty
    _desktopElement.appendChild(_settingsLayoutMailbox);
    
    //create postbox for chosen color settings
    _windowColorPostbox = document.createElement("div");
    _windowColorPostbox.setAttribute("id", "DesktopContent-windowColorPostbox");
    _windowColorPostbox.style.display = "none";
    _windowColorPostbox.innerHTML = this.defaultWindowFrameColor; //init to color string
    _desktopElement.appendChild(_windowColorPostbox);
    
    
    //add dashboard
	this.dashboard = _dashboard = Desktop.createDashboard(_defaultDashboardZindex);
    _desktopElement.appendChild(_dashboard.dashboardElement);
    
    //add icons
	this.icons = _icons = Desktop.createIcons(0);
    _desktopElement.appendChild(_icons.iconsElement);
        
	_handleDesktopResize();

	this.checkMailboxTimer = setInterval(_checkMailboxes,_MAILBOX_TIMER_PERIOD); //start timer for checking foreground window changes due to iFrame content code

	//add login
	this.login = _login = new Desktop.login(!(this.security == Desktop.SECURITY_TYPE_NONE)); //pass true to enable login
	if(_login.loginDiv)
		_desktopElement.appendChild(_login.loginDiv); //add to desktop element for login to display things
    
	Debug.log("Desktop Created",Debug.LOW_PRIORITY);
    

}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//define Desktop mouse helper functions
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

Desktop.foreWinLastMouse = [-1,-1];
Desktop.winManipMode = -1;
Desktop.stretchAndMoveInterval = 0; //used to stretch and move even while moving over iFrames
Desktop.disableMouseDown = 0;

////////////// TOUCHES START CODE ////////////////////
//Desktop.handleTouchStart ~~
//  touch start is called before mouse down, so need to prepare mousedown
//		as though mousemove has been called. Only allow moving window.
//		Disallow dashboard resizing.
Desktop.handleTouchStart = function(touchEvent) {
	Desktop.disableMouseDown = 1; //Disable mouse down on windows if touches are happening
    var touch = touchEvent.targetTouches[0];
	
	var winId = this.id.split('-')[1]; //get id string from div container id
	var isDashboard = (winId == "windowDashboard");
	var win;
	if(!isDashboard){
		win = Desktop.desktop.getWindowById(winId);
		if(win == -1) return false;
		if(win.isMaximized()) {this.style.cursor = "default";return false;}
		
		//bring window to front if newly selected
		if(Desktop.desktop.getForeWindow() != win)
			Desktop.desktop.setForeWindow(win);	
	}
	else return false; //disable dashboard sizing 
    
	//if not manipulating the foreground window	
	if(Desktop.foreWinLastMouse[0] == -1) { 
		var locX = touch.pageX - this.offsetLeft;
		var locY = touch.pageY - this.offsetTop;
		
		//Debug.log("Touch Down " + locX + " - " + locY);
		
		Desktop.desktop.getForeWindow().hideFrame();
		
		Desktop.foreWinLastMouse = [touch.pageX,touch.pageY];
		
		if(locY < win.getWindowHeaderHeight()) { //move 
			Desktop.winManipMode = 0;
		}
	}
	
	return false; //to disable drag and drops
}

//Desktop.handleTouchEnd ~~
//  determine starting mouse position of move or resize
Desktop.handleBodyTouchEnd = function(touchEvent) {Desktop.handleTouchEnd(touchEvent);}
Desktop.handleTouchEnd = function(touchEvent) {
	
	if(Desktop.foreWinLastMouse[0] != -1) //action was happening
	{
		Desktop.foreWinLastMouse = [-1,-1];
		Desktop.winManipMode = -1;
		if(Desktop.desktop.getForeWindow()) Desktop.desktop.getForeWindow().showFrame();	
		//Debug.log("Touch End ");
	}
}

//Desktop.handleTouchMove ~~
//  determine starting mouse position of move or resize
Desktop.handleBodyTouchMove = function(touchEvent) {Desktop.handleTouchMove(touchEvent);}
Desktop.handleTouchMove = function(touchEvent) {
	if(Desktop.winManipMode != -1 && Desktop.foreWinLastMouse[0] != -1) //action happen now
	{
		touchEvent.preventDefault(); //fix chrome issue of only 2 fires
		touchEvent.cancelBubble=true; //eat event away so scrolling doesnt happen
		
	    var touch = touchEvent.targetTouches[0];	
	    var delta = [touch.pageX-Desktop.foreWinLastMouse[0], touch.pageY-Desktop.foreWinLastMouse[1]];		
		//Debug.log("Touch move " + delta[0] + " , " + delta[1]);	
		Desktop.desktop.getForeWindow().moveWindowByOffset(delta[0],delta[1]);
		Desktop.foreWinLastMouse = [touch.pageX,touch.pageY];
	}
}
////////////// TOUCHES END CODE ////////////////////

//Desktop.handleWindowMouseDown ~~
//  determine starting mouse position of move or resize
Desktop.handleWindowMouseDown = function(mouseEvent) {
	var winId = this.id.split('-')[1]; //get id string from div container id
	var isDashboard = (winId == "windowDashboard");
	var win;
	if(!isDashboard) {
		win = Desktop.desktop.getWindowById(winId);
		if(win == -1) return false;
		
		//bring window to front if newly selected
		if(Desktop.desktop.getForeWindow() != win)
			Desktop.desktop.setForeWindow(win);	
	}	
		
	//touches can disable window mouse ops
	if(!Desktop.disableMouseDown && Desktop.winManipMode != -1 && this.style.cursor != "default") //if moving or resizing window
	{
		//register move cursor and window in question
		Desktop.foreWinLastMouse = [mouseEvent.clientX,mouseEvent.clientY];
		if(!isDashboard) Desktop.desktop.getForeWindow().hideFrame();
		//Debug.log("Move/Resize Mode: " + Desktop.winManipMode);
	}
	
	//if(!isDashboard) Debug.log("Mouse Down WinId:" + win.getWindowId() + " - " + this.style.cursor,Debug.LOW_PRIORITY);
			
	return false; //to disable drag and drops
}

//handleWindowMouseUp ~~
//  indicate that no further movement is happening
Desktop.handleWindowMouseUp = function(mouseEvent) {
	
	if(Desktop.foreWinLastMouse[0] != -1) //currently action happening on foreground window
	{			
		if(Desktop.stretchAndMoveInterval) {
			clearInterval(Desktop.stretchAndMoveInterval);	//kill interval iframe mouse watchdog
			Desktop.stretchAndMoveInterval = 0;
		}
		
		Desktop.foreWinLastMouse = [-1,-1];	//indicate no movements happening	
		Desktop.winManipMode = -1;
		if(Desktop.desktop.getForeWindow()) Desktop.desktop.getForeWindow().showFrame();		
		
		//Debug.log("Mouse was released! which=" + mouseEvent.which);
	}
	return false;
}

//handle window move and resize
Desktop.handleWindowMouseMove = function(mouseEvent) {
	var winId = this.id.split('-')[1]; //get id string from div container id
	var isDashboard = (winId == "windowDashboard");
	var win;
	if(!isDashboard){
		win = Desktop.desktop.getWindowById(winId);
		if(win == -1) return false;
		if(win.isMaximized()) {this.style.cursor = "default";return false;}
	}
    
	//change mouse cursor if over a window object and not manipulating the foreground window	
	if(Desktop.foreWinLastMouse[0] == -1) { 
		var locX = mouseEvent.clientX - this.offsetLeft;
		var locY = mouseEvent.clientY - this.offsetTop;
		
		var hotCornerSz = 7;
		if(isDashboard) {
			if(locX > Desktop.desktop.dashboard.getDashboardWidth() - hotCornerSz) {
				this.style.cursor = "e-resize";
				Desktop.winManipMode = 100;
			}
			else
				this.style.cursor = "default";	
		}
		else {
			if((locX < hotCornerSz && locY < hotCornerSz) ||
				(locX > win.getWindowWidth() - hotCornerSz && locY > win.getWindowHeight() - hotCornerSz)) {
				this.style.cursor = "nw-resize";
				Desktop.winManipMode = locY < hotCornerSz?1:2;
			}
			else if((locX > win.getWindowWidth() - hotCornerSz && locY < hotCornerSz) ||
				(locX < hotCornerSz && locY > win.getWindowHeight() - hotCornerSz)) {		
				this.style.cursor = "ne-resize";
				Desktop.winManipMode = locY < hotCornerSz?3:4;
			}
			else if(locX < hotCornerSz) {		
				this.style.cursor = "w-resize";
				Desktop.winManipMode = 5;
			}
			else if(locX > win.getWindowWidth() - hotCornerSz) {		
				this.style.cursor = "e-resize";
				Desktop.winManipMode = 6;
			}
			else if(locY < hotCornerSz) {		
				this.style.cursor = "n-resize";
				Desktop.winManipMode = 7;
			}
			else if(locY > win.getWindowHeight() - hotCornerSz) {		
				this.style.cursor = "s-resize";
				Desktop.winManipMode = 8;
			}
			else if(locY < win.getWindowHeaderHeight()) {
				this.style.cursor = "all-scroll";		 
				Desktop.winManipMode = 0;
			}
			else
				this.style.cursor = "default";
		}	
	}
		
	//handle mouse movements within the page for better user response
	Desktop.handleBodyMouseMove(mouseEvent);
	
	return false; //to disable drag and drops
}

//handle resizing and moving events for desktop
Desktop.handleBodyMouseMove = function(mouseEvent) {

    Desktop.desktop.resetFrameMouse(); //reset last iFrame mouse move
    
	//handle special case for dashboard resize
	if(Desktop.foreWinLastMouse[0] != -1 && Desktop.winManipMode == 100) { 
	
		if(mouseEvent.which == 0) //mouse button was released!!
			return Desktop.handleWindowMouseUp(mouseEvent);
			
		var delta = mouseEvent.clientX-Desktop.foreWinLastMouse[0];
		Desktop.desktop.dashboard.setDashboardWidth(Desktop.desktop.dashboard.getDashboardWidth()+delta);
  		Desktop.foreWinLastMouse = [mouseEvent.clientX,mouseEvent.clientY];				
			
		if(Desktop.stretchAndMoveInterval == 0)  //start timer for iframe mouse watchdog
			Desktop.stretchAndMoveInterval = setInterval( 
				function() { //handle dashboard resize remotely through iframe mouse event
					if(Desktop.desktop.getLastFrameMouseX() == -1) return; //if not in iframe do nothing
					
					var delta = Desktop.desktop.getLastFrameMouseX()-Desktop.foreWinLastMouse[0];		
					Desktop.desktop.dashboard.setDashboardWidth(Desktop.desktop.dashboard.getDashboardWidth()+delta);
  					Desktop.foreWinLastMouse = [Desktop.desktop.getLastFrameMouseX(),Desktop.desktop.getLastFrameMouseY()];			
				}
				,10);
				
		return false;
	}
			
	if(!Desktop.desktop.getForeWindow()) return false;
	
	if(Desktop.foreWinLastMouse[0] != -1)			//window selected and mouse moving now so do something
	{		
		if(mouseEvent.which == 0) //mouse button was released!!
			return Desktop.handleWindowMouseUp(mouseEvent);	
			
		var delta = [mouseEvent.clientX-Desktop.foreWinLastMouse[0], mouseEvent.clientY-Desktop.foreWinLastMouse[1]];	
		
        Desktop.handleWindowManipulation(delta);
		
   		Desktop.foreWinLastMouse = [mouseEvent.clientX,mouseEvent.clientY];
        
        if(Desktop.stretchAndMoveInterval == 0)  //start timer for iframe mouse watchdog
			Desktop.stretchAndMoveInterval = setInterval(
                function() { //handle dashboard resize remotely through iframe mouse event
                    if(Desktop.desktop.getLastFrameMouseX() == -1) return; //if not in iframe do nothing
                     
                    var delta = [Desktop.desktop.getLastFrameMouseX()-Desktop.foreWinLastMouse[0],
                                 Desktop.desktop.getLastFrameMouseY()-Desktop.foreWinLastMouse[1]];
                    Desktop.handleWindowManipulation(delta);
                    Desktop.foreWinLastMouse = [Desktop.desktop.getLastFrameMouseX(),Desktop.desktop.getLastFrameMouseY()];
                }
                ,10);
	}

	return false;
}


//handle resizing and moving events for desktop
Desktop.handleWindowManipulation = function(delta) {
    if(!Desktop.desktop.getForeWindow()) return false;
	
	var win = Desktop.desktop.getForeWindow();
    
    switch(Desktop.winManipMode) { 
		case 0: //move
			win.moveWindowByOffset(delta[0],delta[1]);
			break;
		case 1: //size from top-left
			win.resizeAndPositionWindow(
				win.getWindowX() + delta[0],
				win.getWindowY() + delta[1],
				win.getWindowWidth() - delta[0],
				win.getWindowHeight() - delta[1]);
			break;
		case 2: //size from bottom-right		
			win.resizeAndPositionWindow(
				win.getWindowX(),
				win.getWindowY(),
				win.getWindowWidth() + delta[0],
				win.getWindowHeight() + delta[1]);
			break;
		case 3: //size from top-right
			win.resizeAndPositionWindow(
				win.getWindowX(),
				win.getWindowY() + delta[1],
				win.getWindowWidth() + delta[0],
				win.getWindowHeight() - delta[1]);
			break;
		case 4: //size from bottom-left
			win.resizeAndPositionWindow(
				win.getWindowX() + delta[0],
				win.getWindowY(),
				win.getWindowWidth() - delta[0],
				win.getWindowHeight() + delta[1]);
			break;
		case 5: //size from left
			win.resizeAndPositionWindow(
				win.getWindowX() + delta[0],
				win.getWindowY(),
				win.getWindowWidth() - delta[0],
				win.getWindowHeight());
			break;
		case 6: //size from right
			win.resizeAndPositionWindow(
				win.getWindowX(),
				win.getWindowY(),
				win.getWindowWidth() + delta[0],
				win.getWindowHeight());
			break;
		case 7: //size from top
			win.resizeAndPositionWindow(
				win.getWindowX(),
				win.getWindowY() + delta[1],
				win.getWindowWidth(),
				win.getWindowHeight() - delta[1]);
			break;
		case 8: //size from bottom
			win.resizeAndPositionWindow(
				win.getWindowX(),
				win.getWindowY(),
				win.getWindowWidth(),
				win.getWindowHeight() + delta[1]);
			break;
		default:
    }
}

Desktop.handleWindowButtonDown = function(mouseEvent) {
	mouseEvent.cancelBubble=true; //do nothing but eat event away from window so window doesn't move
	return false;
}

Desktop.handleWindowMinimize = function(mouseEvent) {
	Debug.log("minimize " + this.id.split('-')[1]);
	Desktop.desktop.minimizeWindowById(this.id.split('-')[1]);
	return false;
}

Desktop.handleWindowMaximize = function(mouseEvent) {
	Debug.log("maximize " + this.id.split('-')[1]);
	Desktop.desktop.maximizeWindowById(this.id.split('-')[1]);
	return false;
}

Desktop.handleWindowClose = function(mouseEvent) {
//	Debug.log("close Window " + this.id.split('-')[1]);
	Desktop.desktop.closeWindowById(this.id.split('-')[1]);
	return false;
}

//Desktop.XMLHttpRequest ~~
// forms request properly for ots server, POSTs data
// and when request is returned, returnHandler is called with 
// req result on success, if failure do to bad url called with 0
//
// reqIndex is used to give the returnHandler an index to route responses to.
//
Desktop.XMLHttpRequest = function(requestURL, data, returnHandler, reqIndex) {

	var errStr = "";            
	var req = new XMLHttpRequest();
	
	req.onreadystatechange = function() {
        if (req.readyState==4) {  //when readyState=4 return complete, status=200 for success, status=400 for fail
	        if(req.status==200)
			{
	        	//response received
	        	
				if(!Desktop.desktop.serverConnected)	//mark as connected
				{
					Desktop.desktop.serverConnected = true;
		        	Desktop.desktop.dashboard.displayConnectionStatus(true);
				}
				
				//check if failed due to cookieCode and go to login prompt
				if(req.responseText == Globals.REQ_NO_PERMISSION_RESPONSE)
				{
					errStr = "Request failed do to insufficient account permissions.";
					//return;
				}
				else if(req.responseText == Globals.REQ_NO_LOGIN_RESPONSE) 
				{
					errStr = "Login has expired.";
					window.clearInterval(Desktop.desktop.checkMailboxTimer); //stop checking mailbox
					Desktop.logout(); 
					//return;
				}
					
		        //Debug.log("Request Response Text " + req.responseText + " ---\nXML " + req.responseXML,Debug.LOW_PRIORITY);
			}
			else 
			{
				//response failure
				
				if(Desktop.desktop.serverConnected) //mark as disconnected
				{
					Desktop.desktop.serverConnected = false;
		        	Desktop.desktop.dashboard.displayConnectionStatus(false);
				}

				errStr = "Request Failed - Bad Address:\n" + requestURL;
				window.clearInterval(Desktop.desktop.checkMailboxTimer);  //stop checking mailbox
				Desktop.logout();
			}	        

			if(errStr != "")
			{
				errStr += "\n\n(Try refreshing the page, or alert ots admins if problem persists.)";
				Debug.log("Error: " + errStr,Debug.HIGH_PRIORITY);
				//alert(errStr);
				req = 0; //force to 0 to indicate error
			}
			if(returnHandler) returnHandler(req,reqIndex,errStr);
		}
    }
    
    if(Desktop.desktop.login) //add cookie code if login instance has been created
	    data = "CookieCode="+Desktop.desktop.login.getCookieCode()+"&"+data;
    requestURL = "/urn:xdaq-application:lid="+urnLid+"/"+requestURL;
    //Debug.log("Post " + requestURL + "\n\tData: " + data);
	req.open("POST",requestURL,true);
	//req.setRequestHeader("Content-type","application/x-www-form-urlencoded");
	req.setRequestHeader("Content-Type", "text/plain;charset=UTF-8");
	req.send(data);	
}

//returns xml entry value for an attribute 
Desktop.getXMLAttributeValue = function(req, name, attribute) {
	if(req && req.responseXML && req.responseXML.getElementsByTagName(name).length > 0)
		return req.responseXML.getElementsByTagName(name)[0].getAttribute(attribute);
	else
		return undefined;
}

//returns xml entry value for attribue 'value'
Desktop.getXMLValue = function(req, name) {
	return Desktop.getXMLAttributeValue(req,name,"value");
}

//logout and login prompt
Desktop.logout = function () {     
	if(Desktop.desktop && Desktop.desktop.login)
     	Desktop.desktop.login.logout();  
}

//formatTime ~
Desktop.formatTime = function(t) {
	var date = new Date(t * 1000);
	var mm = date.getMinutes() < 10?"0"+date.getMinutes():date.getMinutes();
	var ss = date.getSeconds() < 10?"0"+date.getSeconds():date.getSeconds();				
	return date.getHours() + ":" + mm + ":" + ss;
}

//closeSystemMessage
Desktop.closeSystemMessage  = function(id) {
	var el = document.getElementById("Desktop-systemMessageBox-" + id);	
	el.parentNode.removeChild(el); //remove from page!
}

Desktop.isWizardMode = function() {
	//return true if in --config desktop mode
	Debug.log("Desktop Security: " + Desktop.desktop.security);
	
	return !(!Desktop.desktop.security || 
    		Desktop.desktop.security == Desktop.SECURITY_TYPE_DIGEST_ACCESS ||
			Desktop.desktop.security == Desktop.SECURITY_TYPE_NONE); 
}

