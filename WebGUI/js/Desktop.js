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
	//
	//public:
	//	Desktop.desktopTooltip()
	//	this.getDesktopWidth = function() { return _desktopElement.clientWidth;}
	//	this.getDesktopHeight = function() { return _desktopElement.clientHeight;}
	//	this.getDesktopX = function() { return _desktopElement.offsetLeft;}
	//	this.getDesktopY = function() { return _desktopElement.offsetTop;}
	//	this.getDesktopContentX = function() { return _desktopElement.offsetLeft+_dashboard.getDashboardWidth();}
	//	this.getDesktopContentY = function() { return _desktopElement.offsetTop+_dashboard.getDashboardHeight();}
	//	this.getDesktopContentWidth = function() { return _desktopElement.clientWidth-_dashboard.getDashboardWidth();}
	//	this.getDesktopContentHeight = function() { return _desktopElement.clientHeight-_dashboard.getDashboardHeight();}
	//	this.getNumberOfWindows = function() { return _windows.length;}
	//	this.getWindowNameByIndex = function(i) { return _windows[i].getWindowName();}
	//	this.getWindowSubNameByIndex = function(i) { return _windows[i].getWindowSubName();}
	//	this.getWindowByIndex = function(i) { return _windows[i];}      
	//	this.getForeWindow = _getForeWindow;
	//	this.redrawDesktop = _handleDesktopResize;
	//	this.getLastFrameMouseX = function() { return parseInt(_mouseOverXmailbox.innerHTML);} 
	//	this.getLastFrameMouseY = function() { return parseInt(_mouseOverYmailbox.innerHTML);} 
	//	this.resetFrameMouse = function() { _mouseOverXmailbox.innerHTML = -1;_mouseOverYmailbox.innerHTML = -1;} 
	//	this.getWindowLayoutStr = _getWindowLayoutStr;
	//	this.addWindow(name,subname,url,unique,extraStep)
	//	this.getWindowById(id)
	//	this.setForeWindow(win)
	//	this.closeWindowById(id)
	//	this.maximizeWindowById(id)
	//	this.toggleFullScreen(e)
	//	this.minimizeWindowById(id)
	//	this.toggleMinimize(e)
	//	this.clickedWindowDashboard(id)
	//	this.setDefaultWindowColor(color)
	//	this.defaultLayoutSelect(i)
	//	this.closeAllWindows()
	//	this.resetDesktop(permissions)
	//	this.actOnParameterAction()
	//	Desktop.XMLHttpRequest(requestURL, data, returnHandler, reqIndex) 
	//	Desktop.getXMLAttributeValue(req, name, attribute)
	//	Desktop.getXMLValue(req, name)
	//	Desktop.logout()
	//	Desktop.formatTime(t)
	//	Desktop.closeSystemMessage(id)
	//	Desktop.isWizardMode()
	//	Desktop.openNewBrowserTab(name,subname,windowPath,unique)
	//	Desktop.desktopTooltip
	
	//"public" handlers:
	//	Desktop.mouseMoveSubscriber(newHandler)
	
	//private:
	//	_handleDesktopResize(event)
	//	_getWindowLayoutStr()
	//	_getForeWindow()
	//	_closeWindow(win)
	//	_checkMailboxes(win)
	//	_handleSystemMessages(req)
	
	//"private" handlers:
	// 	Desktop.handleTouchStart(touchEvent)
	//	Desktop.handleBodyTouchEnd(touchEvent)
	//	Desktop.handleTouchEnd(touchEvent)
	//	Desktop.handleBodyTouchMove(touchEvent)
	//	Desktop.handleTouchMove(touchEvent)
	//	Desktop.handleWindowMouseDown(mouseEvent)
	//	Desktop.handleWindowMouseUp(mouseEvent)
	//	Desktop.handleWindowMouseMove(mouseEvent)
	//	Desktop.handleBodyMouseMove(mouseEvent)
	//	Desktop.handleWindowManipulation(delta)
	//	Desktop.handleWindowButtonDown(mouseEvent)
	//	Desktop.handleWindowMinimize(mouseEvent)
	//	Desktop.handleWindowMaximize(mouseEvent)
	//	Desktop.handleWindowClose(mouseEvent)
	
	
	
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
    var _needToLoginMailbox, _updateTimeMailbox, _updateSettingsMailbox, _settingsLayoutMailbox, _openWindowMailbox, _blockSystemCheckMailbox;
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
	
	var _firstCheckOfMailboxes = true;
	
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
	//	Note: represent position in terms of 0-10000 for the entire Desktop Content area
	//		- this should allow for translation to any size Desktop Content area when loaded
	var _getWindowLayoutStr = function() {		
		var dw = Desktop.desktop.getDesktopContentWidth()/10000.0; //to calc int % 0-10000
		var dh = Desktop.desktop.getDesktopContentHeight()/10000.0;//to calc int % 0-10000
		var dx = Desktop.desktop.getDesktopContentX();
		var dy = Desktop.desktop.getDesktopContentY();
		
		var layout = ""; //"[";				
		for(var i=0;i<_windows.length;++i) 
		{		
			if(_windows[i].getWindowName() == "Settings") continue; //skip settings window
						
			layout += (i?",":"") + 
					encodeURIComponent(_windows[i].getWindowName()) 
					+ "," + encodeURIComponent(_windows[i].getWindowSubName()) 
					+ "," + encodeURIComponent(_windows[i].getWindowUrl()) //_windows[i].getWindowUrl().replace(/&/g,'%38').replace(/=/g,'%61')  //global replace & and =
					+ "," + (((_windows[i].getWindowX()-dx)/dw)|0)
					+ "," + (((_windows[i].getWindowY()-dy)/dh)|0)
					+ "," + ((_windows[i].getWindowWidth()/dw)|0)
					+ "," + ((_windows[i].getWindowHeight()/dh)|0)
					+ "," + (_windows[i].isMinimized()?"0":(_windows[i].isMaximized()?"2":"1"));
					//+ ", "; //last comma (with space for settings display)					
		}
		//layout += "]";
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
		
		if(_firstCheckOfMailboxes)
		{
			Debug.log("First check of mailboxes!");

			Debug.log("Checking for any shortcut work from get parameters...",Debug.LOW_PRIORITY);
			_firstCheckOfMailboxes = false;
			Desktop.desktop.actOnParameterAction();    //this should be the second running and will always work (first time is at end of Desktop instance creation.. and may fail for opening icon by name)
			
		}
		
		//windows can request a blackout, to avoid logging out (attempt to stop all other tabs by using browser cookie)
		if(_blockSystemCheckMailbox.innerHTML == "1")
		{
			Desktop.desktop.login.blackout(true); 
		}
		else
		{
			Desktop.desktop.login.blackout(false);
		}

		

	    //check _openWindowMailbox to see if a window opening is being requested by a Desktop Window
	    //	From requesting window (check that done=1):
	    // 		innerHTML = requestingWindowId=<window uid>&windowPath=<window path to open>
	    //	Response when done:
	    //		innerHTML = requestingWindowId=<window uid>&done=1
	    if(_openWindowMailbox.innerHTML != "")
	    {
	    	Debug.log("_openWindowMailbox.textContent=" + _openWindowMailbox.textContent);
	    	
	    	//get parameters
	    	var paramsStr = _openWindowMailbox.textContent;
	    	var params = [];
	    	var paramCnt = 5 ;
	    	var spliti, splitiOld = 0; 
	    	for(var i=0;i<paramCnt;++i)
	    	{
	    		if(i == paramCnt-1) //last one take the whole thing (this is path, and could have &'s in it)
	    		{
	    			params.push(paramsStr.substr(splitiOld));
	    			break;
	    		}
	    		//for others, handle like normal get param
	    		spliti = paramsStr.indexOf('&', splitiOld);	    		
	    		params.push(paramsStr.substr(splitiOld,spliti-splitiOld))
				splitiOld = spliti+1;
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
	    		// Note: similar to L1000 in actOnParameterAction() 
	    		Debug.log("_openWindowMailbox.innerHTML=" + _openWindowMailbox.innerHTML);
		    	Debug.log("requestingWindowId=" + requestingWindowId);
		    	Debug.log("windowPath=" + windowPath);
		    	Debug.log("windowName=" + windowName);
		    	Debug.log("windowSubname=" + windowSubname);
		    	Debug.log("windowUnique=" + windowUnique);

		    	var newWin;
		    	
		    	//if only windowName is defined, then attempt to open the icon on the 
		    	//	Desktop with that name (this helps resolve supervisor LIDs, etc.)
		    	if(windowSubname == "undefined" &&
		    			windowUnique == "undefined") //the string undefined is what comes through
		    	{
		    		Debug.log("Opening desktop window... " + windowName);

		    		var pathUniquePair = Desktop.desktop.icons.iconNameToPathMap[windowName];
		    		console.log("Desktop.desktop.icons.iconNameToPathMap",
		    				Desktop.desktop.icons.iconNameToPathMap);

		    		if(pathUniquePair ===
		    				undefined)
		    		{
		    			Debug.log("An error occurred opening the window named '" + 
		    					windowName + "' - it was not found in the Desktop icons. " +
								"Do you have permissions to access this window? Notify admins if the problem persists.",
								Debug.HIGH_PRIORITY);
		    			
		    			//respond done
						var str = "requestingWindowId=" + requestingWindowId;
						str += "&done=1";	
						_openWindowMailbox.innerHTML = str; //indicate done
						
		    			return;
		    		}

					var pathStr = pathUniquePair[0];
					
					if(windowPath != "undefined") //add parameters if defined
					{
						Debug.log("Adding parameter path " + windowPath);
						if(pathStr.indexOf('&') > 0) //then assume already parameters
							pathStr += "&";
						else if(pathStr.length && 
								pathStr[pathStr.lengh-1] != '?') //then assume need ?
							pathStr += '?';
						windowPath = pathStr + windowPath;
					}
					else
						windowPath = pathStr;
					
		    		newWin = Desktop.desktop.addWindow(	//(name,subname,url,unique)
		    				windowName, 
							"",
							windowPath,		//e.g. "http://rulinux03.dhcp.fnal.gov:1983/WebPath/html/ConfigurationGUI.html?urn=280",						
							eval(pathUniquePair[1]));	
		    	}
		    	else
		    	{
		    		newWin = Desktop.desktop.addWindow(	//(name,subname,url,unique)
		    				windowName, 
							windowSubname,
							windowPath,		//e.g. "http://rulinux03.dhcp.fnal.gov:1983/WebPath/html/ConfigurationGUI.html?urn=280",						
							eval(windowUnique));			
		    	}

		    	//delay the setting of the fore window
				setTimeout(function(){ Desktop.desktop.setForeWindow(newWin); }, 200);
		    	
				var str = "requestingWindowId=" + requestingWindowId;
				str += "&done=1";	
				_openWindowMailbox.innerHTML = str; //indicate done
	    	}
	    }
		
	    //==============
		//other things besides opening windows
	    //....
	    
	    if(!Desktop.desktop.login || !Desktop.desktop.login.getCookieCode(true))
	    {
	    	if(_needToLoginMailbox.innerHTML == "1") 
	    		 _needToLoginMailbox.innerHTML = ""; //reset
	    	
	    	return; //don't do things if not logged in
	    }

	    //	check if a window iFrame has taken focus and tampered with z mailbox. If so 'officially' set to fore window
		if(_windowZmailbox.innerHTML > _defaultWindowMaxZindex) 
		{
			Desktop.desktop.setForeWindow(Desktop.desktop.getForeWindow()); //use function just to standardize, do not change current foreground			
			//Debug.log("Desktop Foreground Window Refreshed by Timeout",Debug.LOW_PRIORITY);
		}
	    
	    //check need for login mailbox
	    if(_needToLoginMailbox.innerHTML == "1") 
	    {
	        _needToLoginMailbox.innerHTML = ""; //reset
	        if(!document.getElementById("Desktop-loginDiv") &&
	        		!Desktop.desktop.login.isBlackout())
	        {	
	        	//only signal logout if login div is gone (login complete)
				Debug.log("DesktopContent signaled new login needed!",Debug.HIGH_PRIORITY);
				Desktop.logout();
	        }
	        else
	        	Debug.log("Ignoring desktop content need for login signal due to blackout.");
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
			//windows can request a blackout, to avoid logging out 
			if(_blockSystemCheckMailbox.innerHTML == "1" || 
					Desktop.desktop.login.isBlackout())
			{
				Debug.log("System blackout (likely rebooting)...");
				_sysMsgCounter = 0; // reset since not going to handler
			}
			else
				Desktop.XMLHttpRequest("Request?RequestType=getSystemMessages","",_handleSystemMessages);
		}
	}
	
	//===========================================================
	var _lastSystemMessage = ""; //prevent repeats
	//_handleSystemMessages ~~~
	//	handles request returns periodically (ever _SYS_MSG_MAX_COUNT times through _checkMailboxes)
	var _handleSystemMessages = function(req) {
		//Debug.log("Desktop _handleSystemMessages " + req.responseText,Debug.LOW_PRIORITY);
		
		_sysMsgCounter = 0; //reset system message counter to setup next request

		if(!req) return; //request failed			
		
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
	    //_sysMsgSound.src = _SYS_MSG_SOUND_PATH; // buffers automatically when created
	    //_sysMsgSound.play(); //Muted for now
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
		//	If extraStep == 1, tile windows, if == 2, maximize
		//
		//  returns new window
	this.addWindow = function(name,subname,url,unique,extraStep) {		
		Debug.log(name + " - " + subname + " - " + url + " - " + unique,Debug.LOW_PRIORITY);
		
		if(unique) {
			Debug.log("Adding window uniquely",Debug.LOW_PRIORITY);
			for(var i=0;i<_windows.length;++i)
				if(_windows[i].getWindowName() == name && _windows[i].getWindowSubName() == subname) {
					Debug.log("Window creation failed. Not unique.",Debug.LOW_PRIORITY);
					if(_windows[i].isMinimized()) {
						Debug.log(_windows[i].getWindowSubName() + "was minimized but will now be restored!");
						_windows[i].unminimize(); //restore window
					}
					else
						Desktop.desktop.setForeWindow(_windows[i]); //bring window to front                   
					return;
				}
		}
		
		if(_windows.length + _defaultWindowMinZindex >= _defaultWindowMaxZindex) {
			Debug.log("FAILED -- Desktop Window Added - too many windows!",Debug.HIGH_PRIORITY);
			return;
		}

		if(name == "Security Settings") {
		    window_width  = 730;
		    window_height = 410;
		}
		else if(name == "Edit User Data") {
		    window_width  = 730;
		    window_height = 730;
		}
		else {
		    window_width  = _defaultWidth;
		    window_height = _defaultHeight;
		}
		//KEEP..
        //subname += _winId; //for visual window debugging (but destroys uniqueness)
		//end KEEP.
		var newWin = Desktop.createWindow(_winId++,_windows.length + _defaultWindowMinZindex,name,subname,url,
			window_width,window_height,_dashboard.getDashboardWidth() + _currentLeft,_currentTop);

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
        
    	//usually the foreground happens automatically.. but sometimes
        //	it doesn't (?)
        //... so delay an extra setting of the fore window
        setTimeout(function(){ 
        	Desktop.desktop.setForeWindow(newWin); 
        	Debug.log("extraStep=" + extraStep);
        	switch(extraStep)
        	{
        	case 1: //tile
        		Desktop.desktop.dashboard.windowDashboardOrganize();
        		break;
        	case 2: //maximize
        		Desktop.desktop.toggleFullScreen();
        		break;
        	default:; //do nothing for default
        	}
        		
        }, 200);

		

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

		//setWindowZIndex
		// bringing window to foreground then back to original location for refresh
	

		//setForeWindow ~~~
		//	handle bringing window to front
	this.setForeWindow = function(win) {
		//Debug.log("setForeWindow");
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
		
		if(!found && win)
			Debug.log("Desktop Foreground window not Found!! Window: " + 
				win.getWindowSubName());
		
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
        
        _getForeWindow().isMaximized() ? _getForeWindow().unmaximize(): _getForeWindow().maximize();
        
        Desktop.desktop.redrawDashboardWindowButtons();        
        
        //_dashboard.redrawFullScreenButton();
        //_dashboard.redrawRefreshButton();
        Debug.log("Full Screen Toggled",Debug.LOW_PRIORITY);
    }

	this.redrawDashboardWindowButtons = function() {
	    _dashboard.redrawFullScreenButton();
	    _dashboard.redrawRefreshButton();
	    _dashboard.redrawShowDesktopButton();
	}

	this.refreshWindowById = function(id) {
	    var win = this.getWindowById(id);
	    if(win == -1) return -1;

	    this.setForeWindow(win);
	    this.refreshWindow();
            console.log("Finished refreshWindow() " + id);
  	  }

	this.refreshWindow = function(e) {
	    if(!_getForeWindow()) return;
	    //Debug.log("Windows Length: " + _windows.length);
	    	    
	    var window = _getForeWindow();
	    var id  = window.getWindowId();
	    var z   = window.getWindowZ();
	    var name    = window.getWindowName();
	    var subname = window.getWindowSubName();
	    var url     = window.getWindowUrl();
	    var width   = window.getWindowWidth();
	    var height  = window.getWindowHeight();
	    var x = window.getWindowX();
	    var y = window.getWindowY();
        var isMax = window.isMaximized();
        var isMin = window.isMinimized();
        
	    _closeWindow(window);
	    console.log(window, id, z, name, width, height);
	    
	    var newWindow = this.addWindow(name,subname,url);
	    newWindow.setWindowSizeAndPosition(x,y,width,height);
	    newWindow.setWindowZ(z);
	   
	    if(isMax)
	    	newWindow.maximize();
	    if(isMin)
	    	newWindow.minimize();
	   
	    //Debug.log("Windows Length: " + _windows.length);
	    
	    return newWindow;
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
        
        if(_getForeWindow().isMinimized())
        	_getForeWindow().unminimize();
        else
        	_getForeWindow().minimize();
        Debug.log("Minimize Toggled",Debug.LOW_PRIORITY);
        //_dashboard.updateWindows();

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
		var numOfUserLayouts = 5;
		var numOfSystemLayouts = 5;
	  	if(i >= numOfSystemLayouts+1 && //user layouts
	  			i <= numOfSystemLayouts+1+numOfUserLayouts) 
	  		layoutStr = _login.getUserDefaultLayout(i-(numOfSystemLayouts+1));
	  	else if(i >= 0 && i <= numOfSystemLayouts) //system layouts
	  		layoutStr = _login.getSystemDefaultLayout(i);
	  	else //invalid
	  	{
	  		Debug.log("Invalid layout index: " + i, Debug.HIGH_PRIORITY); 
	  		return;
	  	}
		var layoutArr = layoutStr.split(",");
		
		var numOfFields = 8;
		var numOfWins = parseInt(layoutArr.length/numOfFields);
		
		Debug.log("Desktop defaultLayoutSelect layout numOfFields=" + numOfFields);
		Debug.log("Desktop defaultLayoutSelect layout " + numOfWins + 
				" windows - " + layoutStr);	
		
		//clear all current windows
		Desktop.desktop.closeAllWindows();
		
		//open chosen default layout
		//	Note: represent position in terms of 0-10000 for the entire Desktop Content area
		//		- this should allow for translation to any size Desktop Content area when loaded
		//
		//	layout window fields:
		//		0: _windows[i].getWindowName() 
		//		1: _windows[i].getWindowSubName() 
		//		2: _windows[i].getWindowUrl().replace(/&/g,'%38').replace(/=/g,'%61')  //global replace & and =
		//		3: (((_windows[i].getWindowX()-dx)/dw)|0)
		//		4: (((_windows[i].getWindowY()-dy)/dh)|0)
		//		5: ((_windows[i].getWindowWidth()/dw)|0)
		//		6: ((_windows[i].getWindowHeight()/dh)|0)
		//		7: (_windows[i].isMinimized()?"0":(_windows[i].isMinimized()?"2":"1"))
		var dw = Desktop.desktop.getDesktopContentWidth()/10000.0; //to calc int % 0-10000
		var dh = Desktop.desktop.getDesktopContentHeight()/10000.0;//to calc int % 0-10000
		var dx = Desktop.desktop.getDesktopContentX();
		var dy = Desktop.desktop.getDesktopContentY();
		for(i=0;i<numOfWins;++i) 
		{		
			Debug.log("adding " + layoutArr[i*numOfFields].substr(1) + "-" + layoutArr[i*numOfFields+1],Debug.LOW_PRIORITY);	
			this.addWindow(	//(name,subname,url,unique)
				decodeURIComponent(layoutArr[i*numOfFields]), 
				decodeURIComponent(layoutArr[i*numOfFields+1]),
				decodeURIComponent(layoutArr[i*numOfFields+2]),//.replace(/%38/g,"&").replace(/%61/g,"="), //replace back = and &
				false);				
			_windows[_windows.length-1].setWindowSizeAndPosition(		//(x,y,w,h)		
				layoutArr[i*numOfFields+3]*dw + dx,
				layoutArr[i*numOfFields+4]*dh + dy,
				layoutArr[i*numOfFields+5]*dw,
				layoutArr[i*numOfFields+6]*dh);
			
			if((layoutArr[i*numOfFields+7]|0) == 0) //convert to integer, if 0 then minimize
				_windows[_windows.length-1].minimize();
			else if((layoutArr[i*numOfFields+7]|0) == 2) //convert to integer, if 0 then maximize
				_windows[_windows.length-1].maximize();
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
		_blockSystemCheckMailbox.innerHTML = ""; //reset mailbox
		_sysMsgCounter = 0; //reset system message counter
		
		if(permissions !== undefined) //update icons based on permissions		
			Desktop.desktop.icons.resetWithPermissions(permissions);
		
		////if not logged in -- attempt to fix it
		if(!Desktop.desktop.login || !Desktop.desktop.login.getCookieCode(true))
		{
			Desktop.desktop.login.setupLogin();
			
			window.clearInterval(Desktop.desktop.checkMailboxTimer);
			Desktop.desktop.checkMailboxTimer = setInterval(_checkMailboxes,
					_MAILBOX_TIMER_PERIOD);
		}
		//{
			//re-start timer for checking foreground window changes due to iFrame content code
		
		//	this.login = _login = new Desktop.login(!(this.security == Desktop.SECURITY_TYPE_NONE)); //pass true to enable login
		//				window.clearInterval(Desktop.desktop.checkMailboxTimer);
		//				_checkMailboxes();
		//				Desktop.desktop.checkMailboxTimer = setInterval(_checkMailboxes,_MAILBOX_TIMER_PERIOD);
		//}
	}

	this.refreshDesktop = function() {


		for(var i=0; i<Desktop.desktop.getNumberOfWindows();++i)
       	 	{
            		Desktop.desktop.refreshWindowById(Desktop.desktop.getWindowByIndex(i));
        	}	


	}
		
	//actOnParameterAction() ~~~
	//	called during create desktop to handle any shortcuts to windows being maximized
	this.actOnParameterAction = function() {
//		var params = window.parent.window.location.search.substr(1).split("&");
//		var pair,spliti;
//		
//		var requestingWindowId = "", windowPath = "";
//		var windowName, windowSubname, windowUnique, newWindowOps;
//    	var varPair;
			    	
    	
    	//get parameters
		var paramsStr = window.parent.window.location.search.substr(1); //skip the '?'
		var params = [];
		var paramCnt = 5 + (Desktop.isWizardMode()?1:0); //1 extra param in wiz mode
		var spliti, splitiOld = 0; 
		for(var i=0;i<paramCnt;++i)
		{
			if(i == paramCnt-1) //last one take the whole thing (this is path, and could have &'s in it)
			{
				params.push(paramsStr.substr(splitiOld));
				break;
			}
			//for others, handle like normal get param
			spliti = paramsStr.indexOf('&', splitiOld);	    		
			params.push(paramsStr.substr(splitiOld,spliti-splitiOld))
			splitiOld = spliti+1;
		}
		
		var varPair;
		var requestingWindowId = "", windowPath = "";
		var windowName, windowSubname, windowUnique, newWindowOps;
		for(var i=0;i<params.length;++i)
		{
			spliti = params[i].indexOf('=');
			varPair = [params[i].substr(0,spliti),params[i].substr(spliti+1)];	    		
			if(varPair[0] 		== "requestingWindowId")
				requestingWindowId 	= varPair[1];
			else if(varPair[0] 	== "windowPath")
				windowPath 			= decodeURIComponent(varPair[1]);	
			else if(varPair[0] 	== "windowName")
				windowName 			= varPair[1];	
			else if(varPair[0] 	== "windowSubname")
				windowSubname		= varPair[1];	
			else if(varPair[0] 	== "windowUnique")
				windowUnique 		= varPair[1];	 
		}
		
		if(windowPath.indexOf("newWindowOps") >= 0)
		{
			//extract newWindowOps
			newWindowOps = windowPath.split('&')[1].split('=')[1];
			windowPath = windowPath.split('&')[0];
		}
    		    	
//		for(var i=0;i<params.length;++i)
//		{
//    		spliti = params[i].indexOf('=');
//    		varPair = [params[i].substr(0,spliti),params[i].substr(spliti+1)];	
//			Debug.log(i + ": " + varPair[0] + "=" + varPair[1]);
//			if(varPair[0] 		== "requestingWindowId")
//				requestingWindowId 	= varPair[1];
//			else if(varPair[0] 	== "windowPath")
//				windowPath 			= decodeURIComponent(varPair[1]);	
//			else if(varPair[0] 	== "windowName")
//				windowName 			= varPair[1];	
//			else if(varPair[0] 	== "windowSubname")
//				windowSubname		= varPair[1];	
//			else if(varPair[0] 	== "windowUnique")
//				windowUnique 		= varPair[1];	
//			else if(varPair[0] 	== "newWindowOps")
//				newWindowOps 		= varPair[1];	
//		}
		
		if(requestingWindowId != "" && windowPath != "")
		{
			//have work to do!
			Debug.log("_openWindowMailbox.innerHTML=" + _openWindowMailbox.innerHTML);
			Debug.log("requestingWindowId=" + requestingWindowId);
			Debug.log("windowPath=" + windowPath);
			if(newWindowOps) newWindowOps = newWindowOps.replace(/%22/g, "\"");			
			Debug.log("newWindowOps=" + newWindowOps);
			windowName = windowName.replace(/%20/g, " ");
			Debug.log("windowName=" + windowName);
			windowSubname = windowSubname.replace(/%20/g, " ");
			Debug.log("windowSubname=" + windowSubname);
			Debug.log("windowUnique=" + windowUnique);
			
			var newWin;
			
			
			//check if opening layout 
			if(windowName.indexOf("Desktop.openLayout(") == 0)
			{
				var layoutIndex = windowName.substr(("Desktop.openLayout(").length, 
						windowName.length-1-("Desktop.openLayout(").length) | 0;
				Debug.log("Opening layout... " + layoutIndex);
				
				if(pathUniquePair ===
						undefined)
				{

					if(_firstCheckOfMailboxes)
					{
						Debug.log("Perhaps user layout preferences have not been setup yet, try again at mailbox check.");
						return;
					}
				}		
				
				
				_firstCheckOfMailboxes = false; //no need to check at mailbox check time, we are good to go already!

				Desktop.desktop.dashboard.toggleWindowDashboard(0,false);
				Desktop.desktop.defaultLayoutSelect(layoutIndex);				
				return;
			} //end openLayout handling
			
			//if only windowName is defined, then attempt to open the icon on the 
			//	Desktop with that name (this helps resolve supervisor LIDs, etc.)
			if(windowSubname == "undefined" &&
					windowUnique == "undefined") //the string undefined is what comes through
			{
				Debug.log("Opening desktop window... " + windowName);
				
				
				var pathUniquePair = Desktop.desktop.icons.iconNameToPathMap[windowName];
				console.log("Desktop.desktop.icons.iconNameToPathMap",
						Desktop.desktop.icons.iconNameToPathMap);
				
				if(pathUniquePair ===
						undefined)
				{

					if(_firstCheckOfMailboxes)
					{
						Debug.log("Perhaps icons have not been setup yet, try again at mailbox check.");
						return;
					}
					
					Debug.log("An error occurred opening the window named '" + 
							windowName + "' - it was not found in the Desktop icons. " +
							"Do you have permissions to access this window? Notify admins if the problem persists.",
							Debug.HIGH_PRIORITY);

	    			//respond done
					//clear mailbox string since no window is listening for done when a new desktop begins			
					_openWindowMailbox.innerHTML = "";//str; //indicate done
					return;
				}				
				var pathStr = pathUniquePair[0];
				
				if(windowPath != "undefined") //add parameters if defined
				{
					Debug.log("Adding parameter path " + windowPath);
					
					if(pathStr.indexOf('&amp;') > 0) //then assume already parameters
						pathStr += "&amp;";
					else if(pathStr.indexOf('?') > 0 &&  //then assume & for parameters is good
							pathStr[pathStr.length-1] != '?')
						pathStr += "&";
					else if(pathStr.length && 
							pathStr[pathStr.lengh-1] != '?') //then assume need ?
						pathStr += '?';
					windowPath = pathStr + windowPath;
				}
				else
					windowPath = pathStr;
				
				newWin = Desktop.desktop.addWindow(	//(name,subname,url,unique)
						windowName, 
						"",
						windowPath +		//e.g. "http://rulinux03.dhcp.fnal.gov:1983/WebPath/html/ConfigurationGUI.html?urn=280",
						((windowPath.indexOf('?') < 0)? "?":"&amp;") + //add ? start of get parameters if necessary
						((newWindowOps)?"newWindowOps=" + newWindowOps:""), //add get parameter to path for further operations
						eval(pathUniquePair[1]));
			} //end handling of opening desktop window
			else
			{
				_firstCheckOfMailboxes = false; //no need to check at mailbox check time, we are good to go already!
				newWin = Desktop.desktop.addWindow(	//(name,subname,url,unique)
						windowName, 
						windowSubname,
						windowPath +		//e.g. "http://rulinux03.dhcp.fnal.gov:1983/WebPath/html/ConfigurationGUI.html?urn=280",
						((windowPath.indexOf('?') < 0)? "?":"&amp;") + //add ? start of get parameters if necessary
						((newWindowOps)?"newWindowOps=" + newWindowOps:""), //add get parameter to path for further operations
						eval(windowUnique));
			}

			//set to fore window and full screen
			 
			Desktop.desktop.dashboard.toggleWindowDashboard(0,false);
			
	    	//delay the setting of the fore window and fullscreen
			//	so that the window exists before changing it
			setTimeout(function(){
				Desktop.desktop.setForeWindow(newWin);
				Desktop.desktop.toggleFullScreen();
			}, 200);

			//clear mailbox string since no window is listening for done when a new desktop begins			
			_openWindowMailbox.innerHTML = "";//str; //indicate done
		}
	} //end actOnParameterAction()
	
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

    _blockSystemCheckMailbox = document.createElement("div");
    _blockSystemCheckMailbox.setAttribute("id", "DesktopContent-blockSystemCheckMailbox");
    _blockSystemCheckMailbox.style.display = "none";
    _desktopElement.appendChild(_blockSystemCheckMailbox); 
    
    
    
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
    _windowColorPostbox.innerHTML = this.defaultWindowsFrameColor; //init to color string
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

	Debug.log("Checking for any shortcut work from get parameters...",Debug.LOW_PRIORITY);
	Desktop.desktop.actOnParameterAction();  //first time, _firstCheckOfMailboxes is true (then it will try again in checkMailboxes)   
} //end Desktop constructor

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
	Desktop.desktop.icons.closeFolder();
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

Desktop._mouseMoveSubscribers = [];
//Desktop.mouseMoveSubscriber ~~
Desktop.mouseMoveSubscriber = function(newHandler) {
	Desktop._mouseMoveSubscribers.push(newHandler);	
}

//Desktop.handleBodyMouseMove ~~
//	handle resizing and moving events for desktop
//	Returning true is important for allowing selection of text of Debug popup windows
//		(Does it break anything to return true?)
Desktop.handleBodyMouseMove = function(mouseEvent) {
	
	//call each subscriber
	for(var i=0; i<Desktop._mouseMoveSubscribers.length; ++i)
		Desktop._mouseMoveSubscribers[i](mouseEvent); 
	
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
				
		return true;
	}
			
	if(!Desktop.desktop.getForeWindow()) return true;
	
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

	return true;
} //end Desktop.handleBodyMouseMove()


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

Desktop.handleWindowRefresh = function(mouseEvent){
        Debug.log("Refresh " + this.id.split('-')[1]);
        Desktop.desktop.refreshWindowById(this.id.split('-')[1]);
        return false;

}

Desktop.handleFullScreenWindowRefresh = function(mouseEvent){
        Debug.log("Refresh Full Screen Window");
        
        var foreWindowId = undefined;
        try
        {
        	foreWindowId = Desktop.desktop.getForeWindow().getWindowId();
        }
        catch(e)
        {
        	Debug.log("Could not find foreground window, ignoring.");
        }
        
        
        Desktop.desktop.resetDesktop();
		Desktop.desktop.refreshDesktop();
		
		var foreWindow = undefined;
		var isMaxWindow = undefined;
			

		//		for(var i = 0; i < Desktop.desktop.getNumberOfWindows(); i++)
		//    	{
		//			var window =  Desktop.desktop.getWindowByIndex(i);
		//			var id = window.getWindowId();
		//			var z = window.getWindowZ();
		//			
		//			Debug.log("name: " + i + " " + window.getWindowName());
		//			Debug.log("ID: " + id + " z=" + z);
		//			
		//    	}
		
		//Note: refresh window takes foreground window
		//	and deletes it, then makes a new one that ends up being the
		//	last window in the array... so always take index 0 to iterate through them
		//	but save the encountered current foreWindow
		for(var i = 0; i < Desktop.desktop.getNumberOfWindows(); i++)
    	{
			var window =  Desktop.desktop.getWindowByIndex(0);
			var id = window.getWindowId();
			
			Debug.log("name: " + i + " " + window.getWindowName());
			Debug.log("ID: " + id);
			
			var maximized = window.isMaximized();
				
			
			Desktop.desktop.setForeWindow(window);
			window = Desktop.desktop.refreshWindow();            
            
			if(maximized)
				isMaxWindow = window;

			if(foreWindowId == id)
				foreWindow = window;
	    }

		//		for(var i = 0; i < Desktop.desktop.getNumberOfWindows(); i++)
		//    	{
		//			var window =  Desktop.desktop.getWindowByIndex(i);
		//			var id = window.getWindowId();
		//			var z = window.getWindowZ();
		//			
		//			Debug.log("name: " + i + " " + window.getWindowName());
		//			Debug.log("ID: " + id + " z=" + z);
		//			
		//    	}
		
		if(foreWindow)
			Desktop.desktop.setForeWindow(foreWindow);
		if(isMaxWindow)
			Desktop.desktop.setForeWindow(foreWindow);

		//		for(var i = 0; i < Desktop.desktop.getNumberOfWindows(); i++)
		//    	{
		//			var window =  Desktop.desktop.getWindowByIndex(i);
		//			var id = window.getWindowId();
		//			var z = window.getWindowZ();
		//			
		//			Debug.log("name: " + i + " " + window.getWindowName());
		//			Debug.log("ID: " + id + " z=" + z);
		//			
		//    	}
    	return false;
} //end handleFullScreenWindowRefresh()

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
		        	
		        	Desktop.desktop.resetDesktop(); //give no permissions, to do simple reset
		        	// and re-start timer for checking foreground window changes due to iFrame content code		        	
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

       			Debug.log("The user interface is disconnected from the ots Gateway server.", Debug.HIGH_PRIORITY);
       			//hide user with lock icon (because it usually looks bad when disconnected)
       			document.getElementById("DesktopDashboard-userWithLock").style.display = "none";
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

//getXMLValue ~~
//	returns xml entry value for attribue 'value'
Desktop.getXMLValue = function(req, name) {
	return Desktop.getXMLAttributeValue(req,name,"value");
}

//logout ~~
//	logout and login prompt
Desktop.logout = function () {     
	if(Desktop.desktop && Desktop.desktop.login && 
			!Desktop.desktop.login.isBlackout())
     	Desktop.desktop.login.logout();  
}

//formatTime ~~
Desktop.formatTime = function(t) {
	var date = new Date(t * 1000);
	var mm = date.getMinutes() < 10?"0"+date.getMinutes():date.getMinutes();
	var ss = date.getSeconds() < 10?"0"+date.getSeconds():date.getSeconds();				
	return date.getHours() + ":" + mm + ":" + ss;
}

//closeSystemMessage ~~
Desktop.closeSystemMessage  = function(id) {
	var el = document.getElementById("Desktop-systemMessageBox-" + id);	
	el.parentNode.removeChild(el); //remove from page!
}

//isWizardMode ~~
Desktop.isWizardMode = function() {
	//return true if in --config desktop mode
	Debug.log("Desktop Security: " + Desktop.desktop.security);
	
	return !(!Desktop.desktop.security || 
    		Desktop.desktop.security == Desktop.SECURITY_TYPE_DIGEST_ACCESS ||
			Desktop.desktop.security == Desktop.SECURITY_TYPE_NONE); 
}

//openNewBrowserTab ~~
Desktop.openNewBrowserTab = function(name,subname,windowPath,unique) { 
	

	//for windowPath, need to check lid=## is terminated with /
	// check from = that there is nothing but numbers
	{
		var i = windowPath.indexOf("urn:xdaq-application:lid=") + ("urn:xdaq-application:lid=").length;
		var isAllNumbers = true;
		for(i;i<windowPath.length;++i)
		{
			Debug.log(windowPath[i]);

			if(windowPath[i] < "0" || windowPath[i] > "9")
			{
				isAllNumbers = false;
				break;
			}				
		}
		if(isAllNumbers)
			windowPath += "/";		
	}
	Debug.log("DesktopWindow= " + windowPath);
	
	Debug.log("name= " + name);
	Debug.log("subname= " + subname);
	Debug.log("unique= " + unique);
	var search = window.parent.window.location.search;
	url = window.parent.window.location.pathname;

	var str = "requestingWindowId=Desktop";
		str += "&windowName=" + name;
		str += "&windowSubname=" + subname;
		str += "&windowUnique=" + unique;
		str += "&windowPath=" + encodeURIComponent(windowPath);
		
	//if there is no search, need to check lid=## is terminated with /
	// check from = that there is nothing but numbers
	
	if(!Desktop.isWizardMode()) 
	{
		var i = url.indexOf("urn:xdaq-application:lid=") + ("urn:xdaq-application:lid=").length;
		var isAllNumbers = true;
		for(i;i<url.length;++i)
		{
			Debug.log(url[i]);
			
			if(url[i] < "0" || url[i] > "9")
			{
				isAllNumbers = false;
				break;
			}				
		}
		if(isAllNumbers)
			url += "/";
		url += "?" + str;
	}
	else// if(Desktop.isWizardMode())
	{
		url += search.split('&')[0] + "&" + str; //take first parameter (for wiz mode)
	}
	
	Debug.log("DesktopContent.openNewBrowserTab= " + url);
	
	window.open(url,'_blank');	
}

//call to show desktop tooltip
//	shown for wiz mode and normal mode, e.g.
Desktop.desktopTooltip = function()	{
	
	DesktopContent.tooltip("Desktop Introduction",
			"Welcome to the <i>otsdaq</i> Desktop environment. This is your portal " +
			"to all of the possibilities of <i>otsdaq</i>.\n\n" +
			"Briefly, here are the features:" +

			"\n\t- <b>Desktop Window Icons:</b> " +
			"<INDENT>" +
			"Click the rounded-square icons on your Desktop to open " +
			"a particular window. If you hold down your click (for a second), you " +
			"can open the window in fullscreen, or in a new tab, or even open the window " +
			"then tile it on the screen with all other open windows!" +
			"</INDENT>" +

			"\n\t- <b>Desktop Dashboard (top pane):</b> " +
			"<INDENT>" +
			"Along the top and left margins of the Desktop, you will find the Desktop " +
			"Dashboard - this section is an introduction to the top pane of the Dashboard. " +
			"The top pane of the Dashboard " +
			"is made of buttons and icons going from left to right:" +

			"\n\t- <b>Left Pane Toggler:</b> " +
			"<INDENT>" +
			"The first button you will encounter in the top pane looks like a horizontal double-arrow " +
			"icon. This button toggles the display of the left pane of the Dashboard. Note when you " +
			"refresh the page, the state of the left pane persists!" +
			"</INDENT>" +

			"\n\t- <b>Layouts Menu:</b> " +
			"<INDENT>" +
			"The next button you will encounter in the top pane reads 'Layouts.' " +
			"This button gives you access to your Window Layout Presets. You will see 2 'System' presets there " +
			"which can be setup by uesrs with admin privileges. There are also 3 'User' presets which you can setup for " +
			"yourself in the User Settings window (get there with " +
			"the cog wheel icon in the upper-right of the Desktop)." +
			"</INDENT>" +

			"\n\t- <b>Tile Desktop Windows:</b> " +
			"<INDENT>" +
			"The next button you will encounter in the top pane reads 'Tile.' " +
			"This button will automatically tile all open Desktop Windows to fit in your browser window." +
			"</INDENT>" +

			"\n\t- <b>Show Desktop:</b> " +
			"<INDENT>" +
			"The next button you will encounter in the top pane reads 'Show Desktop.' " +
			"This button will minimize all open Desktop Windows which is nice " +
			"when you want to see all of your Desktop Icons again." +
			"</INDENT>" +

			"\n\t- <b>Full Screen:</b> " +
			"<INDENT>" +
			"The next button you will encounter in the top pane reads 'Full Screen.' " +
			"This button will maximize to full screen the Desktop Window that was last used (i.e. the window that has the focus)." +
			"</INDENT>" +
			"</INDENT>" +

			"\n\t- <b>Desktop Dashboard (left pane):</b> " +
			"<INDENT>" +
			"Along the top and left margins of the Desktop, you will find the Desktop " +
			"Dashboard - this section is an introduction to the left pane of the Dashboard. " +
			"The left pane of the Dashboard " +
			"is a listing of all open Desktop Windows. If you click one of the buttons in the list, " +
			"the associated window " +
			"will be minimized or restored. If you hold down your click (for one second) you can choose " +
			"to minimize, maximize, or close the window!" +
			"</INDENT>" +



			"\n\nRemember, if you would like to take a look at the available online documentation, " +
			"click the question mark at the top-right of the Desktop."
	);	
}


















