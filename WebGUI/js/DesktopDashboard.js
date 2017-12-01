//=====================================================================================
//
//	Created Dec, 2012
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	DesktopDashboard.js
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
	console.log('ERROR: Desktop is undefined! Must include Desktop.js before DesktopDashboard.js');
else {

		
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	//define Desktop.createDashboard to return dashboard class	
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	Desktop.createDashboard = function(z) {
        if(false === (this instanceof Desktop.createDashboard)) {
			//here to correct if called as "var v = Desktop.createDesktop();"
			//	instead of "var v = new Desktop.createDesktop();"
	        return new Desktop.createDashboard(z);
	    }
        
        

    	//------------------------------------------------------------------
    	//list of members functions ----------------------
    	//------------------------------------------------------------------
    	//private:
    	//public:
        	//displayConnectionStatus(connected) //bool connected
        
        
        
        
        
        
        //------------------------------------------------------------------
		//create private members variables ----------------------
		//------------------------------------------------------------------
        var _defaultDashboardColor = "#3C404B";
		
		
		
		//all units in pixels unless otherwise specified
		
		var _defaultDashboardHeight = 41;
		var _defaultWindowDashboardWidth = 200;
		var _defaultWindowDashboardMinWidth = 50;
        
        var _windowDashboardWidth = _defaultWindowDashboardWidth;
        
        Debug.log("window.parent.window.location.hash=" + window.parent.window.location.hash,
        		Debug.MED_PRIORITY);
        var _displayWindowDashboard = //default window dashboard view
        		window.parent.window.location.hash[1]? 
        				(window.parent.window.location.hash[1] | 0):1; 
        var _windowDashboard,_topBar,_fullScreenBtn;
        
        var _windowDashboardWindowCSSRule;  //e.g. _var.style.width = "100px"
        
        var _layoutDropDownDisplayed = false;
		var _layoutMenuItems = ["System Preset-1","System Preset-2","---","User Preset-1","User Preset-2","User Preset-3"];
        
        var _dashboardElement, _dashboardColorPostbox;
        
        var _deepClickTimer = 0;
		//------------------------------------------------------------------
		//create public members variables ----------------------
		//------------------------------------------------------------------
		this.dashboardElement;
        

		
		//------------------------------------------------------------------
		//create PRIVATE members functions ----------------------
		//------------------------------------------------------------------
        var _toggleWindowDashboard = function(event,setValue) {        	
        	
        	if(setValue !== undefined)
        		_displayWindowDashboard = setValue;
        	else //toggle
        		_displayWindowDashboard = !_displayWindowDashboard;

			var newURL = window.parent.window.location.pathname +
									window.parent.window.location.search +
									"#"+
									(_displayWindowDashboard?"1":"0"); 
			
			//update browser url so refresh will give same desktop experience
			if(!Desktop.isWizardMode()) 
				window.parent.window.history.replaceState('ots', 'ots', newURL);    
			else
				window.parent.window.history.replaceState('ots wiz', 'ots wiz', newURL); 
			
            _windowDashboard.style.display = _displayWindowDashboard?"inline":"none";
            Desktop.desktop.redrawDesktop();
        }
        
        	//_refreshTitle() ~~~
			//	private function to refresh title name based on dashboard size
			// 	clip text if too long
		var _refreshTitle = function() {
		
			var el,winIndex;
			var hdrW = _windowDashboardWidth - 14; //2*7px padding
			for(var i=0;i<Desktop.desktop.getNumberOfWindows();++i) {
				el = document.getElementById('DesktopDashboard-windowDashboard-win'+i);
				winIndex = document.getElementById('DesktopDashboard-windowDashboard-winIndex'+i).innerHTML;
				el.innerHTML = Desktop.desktop.getWindowNameByIndex(winIndex) + 
					(Desktop.desktop.getWindowSubNameByIndex(winIndex)==""?"":" - ") +
                    Desktop.desktop.getWindowSubNameByIndex(winIndex);
				while(el.scrollWidth > hdrW && el.innerHTML.length > 4)
					el.innerHTML = el.innerHTML.substr(0,el.innerHTML.length-4)+"...";
				
				el.innerHTML += "<div class='hiddenDiv' " +  //add hidden window index back in for future use
                    "id='DesktopDashboard-windowDashboard-winIndex"+i+"'>"+ winIndex + "</div>";
            }
		}
        
        var _redrawDashboard = function() {            
            
            _topBar.style.left = Desktop.desktop.getDesktopX()+"px";
            _topBar.style.top = Desktop.desktop.getDesktopY()+"px";
            _topBar.style.width = Desktop.desktop.getDesktopWidth()+"px";
            _topBar.style.height = _defaultDashboardHeight+"px";
            
            _windowDashboard.style.left = Desktop.desktop.getDesktopX()+"px";
            _windowDashboard.style.top = Desktop.desktop.getDesktopY()+_defaultDashboardHeight+"px";
            _windowDashboard.style.width = _windowDashboardWidth+"px";
            _windowDashboardWindowCSSRule.style.width = (_windowDashboardWidth-34)+"px"; //2*7px padding, 2*10px margin
            _refreshTitle();
            _windowDashboard.style.height = Desktop.desktop.getDesktopHeight()-(Desktop.desktop.getDesktopY()+_defaultDashboardHeight)+"px";
            
        }
        
        //get CSS style rule fpr dasboard window boxes
        var _getDashboardWindowWidthCSSRule = function() {
            
	        var i,j;
	        for(i=0;i<document.styleSheets.length;++i) {
	        	Debug.log(document.styleSheets[i].href);
	        	if(document.styleSheets[i].href && document.styleSheets[i].href.split('/').pop() == 
	        			"Desktop.css") {
	        		for(j=0;j<document.styleSheets[i].cssRules.length;++j) {
	        			if(document.styleSheets[i].cssRules[j].selectorText ==
	        					"#Desktop .DesktopDashboard-windowDashboard-win")
	        				return document.styleSheets[i].cssRules[j]; //success!!
	        		}
	        		break; //failed
	        	}	        		
	        }
	        if(i == document.styleSheets.length) Debug.log("FAIL -- Could not locate CSS Rule for Dashboard Window.",Debug.HIGH_PRIORITY);
	        return 0;
		}
		
		var _windowOrganizeMode = 0;
		var _windowDashboardOrganize = function() {
		
			var win;
			
			var dx = Desktop.desktop.getDesktopContentX(), dy = Desktop.desktop.getDesktopContentY(),
				dw = Desktop.desktop.getDesktopContentWidth(), dh = Desktop.desktop.getDesktopContentHeight();
			var xx, yy;
			
			switch(_windowOrganizeMode) {
			case 0: //tile
				var rows = 1;
				var ww = Math.floor(dw/Desktop.desktop.getNumberOfWindows());
				var wh = dh;
				
				while(ww*2 < wh) {				
					//Debug.log("Desktop Dashboard Organize " + ww + " , " + wh,Debug.LOW_PRIORITY);
					ww = Math.floor(dw/Math.ceil(Desktop.desktop.getNumberOfWindows()/++rows)); wh = Math.floor(dh/rows);				
				}  //have too much height, so add row
				xx = dx; yy = dy;
				//Debug.log("Desktop Dashboard Organize " + ww + " , " + wh,Debug.LOW_PRIORITY);
				var cols = Math.ceil(Desktop.desktop.getNumberOfWindows()/rows);
				Debug.log("Desktop Dashboard Organize r" + rows + " , c" + cols,Debug.LOW_PRIORITY);
						
				for(var i=0;i<Desktop.desktop.getNumberOfWindows();++i) {					
					win = Desktop.desktop.getWindowByIndex(
							document.getElementById('DesktopDashboard-windowDashboard-winIndex'+i).innerHTML);

					win.setWindowSizeAndPosition(xx,yy,ww,wh);
					if(win.isMinimized()) win.minimize();
					if(win.isMaximized()) win.maximize();
					
					xx += ww;
					if((i+1)%cols==0){xx = dx; yy += wh;} //start new row			
					//Debug.log("Desktop Dashboard Organize i:" + (i+1)%cols + " -  " + xx + " , " + yy,Debug.LOW_PRIORITY);	
				}
				break;
			case 1: //offset arrange
				for(var i=0;i<Desktop.desktop.getNumberOfWindows();++i) {
					win = Desktop.desktop.getWindowByIndex(winIndex);
				}
				break;
			default:
				Debug.log("Desktop Dashboard Organize Impossible Mode",Debug.LOW_PRIORITY);
			}
			Debug.log("Desktop Dashboard Organize Mode: " + _windowOrganizeMode,Debug.LOW_PRIORITY);
		}
		
		var _windowDashboardMinimizeAll = function() {
				var win;
				for(var i=0;i<Desktop.desktop.getNumberOfWindows();++i) {
					win = Desktop.desktop.getWindowByIndex(i);
					win.minimize();	if(!win.isMinimized()) win.minimize(); //minimize twice, in case was mazimized
				}
		}
		
		//_windowDashboardLayoutsDropDown ~
		//	toggles default layout drop down menu
		var _windowDashboardLayoutsDropDown = function() {		
			_layoutDropDownDisplayed = !_layoutDropDownDisplayed;
			Debug.log("Desktop _windowDashboardDefaultsDropDown " + _layoutDropDownDisplayed,Debug.LOW_PRIORITY);
			
			var el;
			//remove drop down if already present
			el = document.getElementById("DesktopDashboard-defaults-dropdown");
			if(el) { //should not be any DesktopDashboard-defaults-dropdown div so delete 
			
				Debug.log("found DesktopDashboard-defaults-dropdown div and deleted",Debug.LOW_PRIORITY);
				el.parentNode.removeChild(el);
			}
			if(!_layoutDropDownDisplayed) return; //do not create if closing
			
			//create default dropdown menu element
	        el = document.createElement("div");	        
			el.setAttribute("id", "DesktopDashboard-defaults-dropdown");
        	el.style.backgroundColor = _defaultDashboardColor;
        	el.innerHTML = "";
        	for(var i=0;i<_layoutMenuItems.length;++i) 
        		if(_layoutMenuItems[i] == "---") //horizontal line
	        		el.innerHTML += "<center><hr width='75%' style='border:1px solid; margin-top:5px'/></center>";
	        	else {
	        		el.innerHTML += "<a href='#' onmouseup='Desktop.desktop.dashboard.windowDashboardLayoutsDropDown(); "
	        			+ "Desktop.desktop.defaultLayoutSelect("+i+"); return false;'>"
	        			+ _layoutMenuItems[i] + "</a>";
	        	   
	        	   	if(i<_layoutMenuItems.length-1) 
	        			el.innerHTML += "<br/>";        		
        		}
			_dashboardElement.appendChild(el);
		}
		
		//------------------------------------------------------------------
		//create PUBLIC members functions ----------------------
		//------------------------------------------------------------------
        this.getDashboardHeight = function() { return _defaultDashboardHeight;}
        this.getDashboardWidth = function() { return _displayWindowDashboard?_windowDashboardWidth:0;}
        this.setDashboardWidth = function(w) { if(w > _defaultWindowDashboardMinWidth) _windowDashboardWidth = w; Desktop.desktop.redrawDesktop();}
    
        this.toggleWindowDashboard = _toggleWindowDashboard; 
        this.redrawDashboard = _redrawDashboard;        
        this.windowDashboardLayoutsDropDown = _windowDashboardLayoutsDropDown;
        this.windowDashboardOrganize = _windowDashboardOrganize;
                
        this.updateWindows = function() {
            
            _windowDashboard.innerHTML = "";
            
            var mySortArrId = [];
            var mySortArrIndex = [];
            for(var i=0;i<Desktop.desktop.getNumberOfWindows();++i) {
                mySortArrId.push(Desktop.desktop.getWindowByIndex(i).getWindowId());
                mySortArrIndex.push(i);
            }
            
            //sort by id
            for(var i=0;i<mySortArrId.length-1;++i) {
                var min = i;
                for(var j=i+1;j<mySortArrId.length;++j)
                    if(mySortArrId[j] < mySortArrId[min])
                        min = j;
                
                //have min, swap to i
                var tmp;
                tmp = mySortArrId[i];
                mySortArrId[i] = mySortArrId[min];
                mySortArrId[min] = tmp;
                tmp = mySortArrIndex[i];
                mySortArrIndex[i] = mySortArrIndex[min];
                mySortArrIndex[min] = tmp;
            }
            
            //have sorted by id, create window buttons for dashboard
            var tmpClassStr, defClassStr = "DesktopDashboard-windowDashboard-win";
            
            for(var i=0;i<Desktop.desktop.getNumberOfWindows();++i) {
                
                tmpClassStr = defClassStr +  //if foreground window, append class
                	((mySortArrIndex[i] == Desktop.desktop.getNumberOfWindows()-1)?
                			" DesktopDashboard-windowDashboard-foreWin":"");
                _windowDashboard.innerHTML += 
                		"<div " +
						"onmouseup='Desktop.desktop.dashboard.handleDashboardWinMouseUp(event," +
						//"onmouseup='Desktop.desktop.clickedWindowDashboard(" +
						mySortArrId[i] + ");' " + 
						"onmousedown='Desktop.desktop.dashboard.handleDashboardWinMouseDown(event," +
						//"onmouseup='Desktop.desktop.clickedWindowDashboard(" +
						mySortArrId[i] + ");' " + 
						//"onmousedown='Desktop.handleWindowButtonDown(event);' " +   //eat window button down event
						"class='" + tmpClassStr + "' " +
						"id='DesktopDashboard-windowDashboard-win"+i+"'>" +
						Desktop.desktop.getWindowNameByIndex(mySortArrIndex[i]) + " - " +
						Desktop.desktop.getWindowSubNameByIndex(mySortArrIndex[i]) +
						"<div class='hiddenDiv' " +
						"id='DesktopDashboard-windowDashboard-winIndex"+i+"'>"+ mySortArrIndex[i] + 
						"</div>" +
						"</div>\n";
            }      
            
		   	_refreshTitle(); 
        }        
        
        this.redrawFullScreenButton = function() {
            _fullScreenBtn.innerHTML = "<a href='#' title='Click to toggle full screen mode for current window'>" +
            	((Desktop.desktop.getForeWindow() &&
                                        Desktop.desktop.getForeWindow().isMaximized())?"Exit Full Screen":"Full Screen") + "</a>"; 
                                        		
        }
        
        this.getDefaultDashboardColor = function() { return _defaultDashboardColor; }
        
        this.setDefaultDashboardColor = function(color) {
        	_defaultDashboardColor = color;
            _dashboardColorPostbox.innerHTML = _defaultDashboardColor; //set to color string
            
        	_topBar.style.backgroundColor = _defaultDashboardColor;
        	_windowDashboard.style.backgroundColor = _defaultDashboardColor;
        }

        var _oldUserNameWithLock = "";
        this.displayUserLock = function(usernameWithLock, el) {      
        	if(!el)
        		el = document.getElementById("DesktopDashboard-userWithLock");

   			el.style.display = "block";
        	
        	var user = Desktop.desktop.login.getUsername();
        	var data = "";
        	data += "lock=" + ((!usernameWithLock || usernameWithLock == "")?"1":"0") + "&";
        	data += "username=" + user;

        	var jsReq =   			
        			"Desktop.XMLHttpRequest(\"" +
					"Request?RequestType=setUserWithLock&accounts=1\"," +
					"\"" + data + "\",Desktop.desktop.dashboard.handleSetUserWithLock)";

       		if(!usernameWithLock || usernameWithLock == "") 
       		{		
       			//nobody has lock
       			var str = "";       			
       			str += "<a href='javascript:" + jsReq + "'>";
       			str += "<img " +
       					"src='/WebPath/images/dashboardImages/icon-Settings-Unlock.png' " +
       					"title='Click to lockout the system and take the ots Lock'>";
       			str += "</a>";
       			el.innerHTML = str; 
       			_oldUserNameWithLock = "";       			
       			return; 
       		}  	
       		
       		if(_oldUserNameWithLock == usernameWithLock) return; //stop graphics flashing of lock

       		var str = "";       			
       		if(usernameWithLock != user) //not user so cant unlock
       			str = "<img src='/WebPath/images/dashboardImages/icon-Settings-LockDisabled.png' " +
       					"title='User " + 
       					usernameWithLock + " has the ots Lock'>"; 
       		else //this is user so can unlock
       		{
				str += "<a href='javascript:" + jsReq + "'>";
				str += "<img " +	
						"src='/WebPath/images/dashboardImages/icon-Settings-Lock.png' " +
						"title='Click to unlock the system and release the ots Lock'>";
				str += "</a>";
       		}
       		
   			el.innerHTML = str; 
   			
       		_oldUserNameWithLock = usernameWithLock; 
        }        
        
        this.handleSetUserWithLock = function(req) {
        	Debug.log(req);
			//extract alert from server
			var serverAlert = Desktop.getXMLValue(req,"server_alert");
			if(serverAlert) Debug.log("Message from Server: " + serverAlert, Debug.HIGH_PRIORITY);

        	Desktop.desktop.dashboard.displayUserLock(
        			Desktop.getXMLValue(req,"username_with_lock"));        	

        	Desktop.desktop.resetDesktop(); //soft reset attempt
        }
        

        //displayConnectionStatus ~~
        //	bool connected
        this.displayConnectionStatus = function(connected) {    
       		var el = document.getElementById("DesktopDashboard-serverConnectionStatus");
       		
       		if(connected) 
       		{		
       			el.style.display = "none";
       			el.innerHTML = "";
       		}  	
       		else
       		{
       			el.innerHTML = "*** <a onclick='Desktop.desktop.resetDesktop();//soft reset attempt' " + 
       					"style='cursor:pointer; color:rgb(255,150,0);'>Disconnected</a> ***";
       			el.style.display = "block";       			
       		}
        }
        
        //handleDashboardWinMouseUp ~~
        this.handleDashboardWinMouseUp = function(event, winId) {  
			if(_deepClickTimer)
			{
				window.clearTimeout(_deepClickTimer);
				_deepClickTimer = 0;
			}
        	Desktop.desktop.clickedWindowDashboard(winId);
        }
        
        //handleDashboardWinMouseDown ~~
        this.handleDashboardWinMouseDown = function(event, winId) {
			event.cancelBubble = true; //prevent default behavior 
			event.preventDefault();
			_deepClickTimer = window.setTimeout(function() {

				var targetWin = Desktop.desktop.getWindowById(winId);
				Debug.log("Create Dashboard Window Menu " +
						targetWin.isMaximized() + "-" + targetWin.isMinimized());				
				
				var menuItems = [
								 targetWin.isMaximized()?
										"Restore Window":"Maximize Window",
								targetWin.isMinimized()?
										"Restore Window":"Minimize Window",
								 "Close Window"
								 ];
				var menuItemHandlers = [
										"Desktop.desktop.maximizeWindowById("+ winId + ")",
										"Desktop.desktop.minimizeWindowById("+ winId + ")",
										"Desktop.desktop.closeWindowById("+ winId + ")",					 
										];
				Debug.log("createEditTableMenu()");
				SimpleContextMenu.createMenu(
						menuItems,
						menuItemHandlers,
						"DesktopIconContextMenu",		//element ID
						event.pageX-1,event.pageY-1, 	//top left position
						Desktop.desktop.dashboard.getDefaultDashboardColor(), 	//primary color
						"white"				//secondary color
				);

			},500); //end timeout handler
        }        
        
		//------------------------------------------------------------------
		//handle class construction ----------------------
		//------------------------------------------------------------------
        
        this.dashboardElement = _dashboardElement = document.createElement("div");
		this.dashboardElement.setAttribute("class", "DesktopDashboard");
		this.dashboardElement.setAttribute("id", "DesktopDashboard");
        
        //create top bar
        _topBar = document.createElement("div");
		_topBar.setAttribute("class", "DesktopDashboard-topBar");
	   	_topBar.style.position = "absolute";
	   	_topBar.style.zIndex = z;
        _topBar.style.backgroundColor = _defaultDashboardColor;

        var tmpBtn = document.createElement("div");
		tmpBtn.setAttribute("class", "DesktopDashboard-button DesktopDashboard-button-left");
        tmpBtn.innerHTML = "<a href='#' title='Click to show/hide Window Bar'>" + 
        	"<img id='dashboard_bi_arrow' src='/WebPath/images/dashboardImages/icon-Bi-arrow.png'></a>";
        tmpBtn.onmouseup = _toggleWindowDashboard;
        _topBar.appendChild(tmpBtn);
        
        tmpBtn = document.createElement("div");
		tmpBtn.setAttribute("class", "DesktopDashboard-button DesktopDashboard-button-left");
        tmpBtn.innerHTML = "<a href='#' title='Click to open default window layouts'>Layouts</a>";
        tmpBtn.onmouseup = _windowDashboardLayoutsDropDown;
        _topBar.appendChild(tmpBtn);
        
        tmpBtn = document.createElement("div");
		tmpBtn.setAttribute("class", "DesktopDashboard-button DesktopDashboard-button-left");
        tmpBtn.innerHTML = "<a href='#' title='Click to automatically arrange and tile windows'>Tile</a>";
        tmpBtn.onmouseup = _windowDashboardOrganize;
        _topBar.appendChild(tmpBtn);
        
        tmpBtn = document.createElement("div");
		tmpBtn.setAttribute("class", "DesktopDashboard-button DesktopDashboard-button-left");
        tmpBtn.innerHTML = "<a href='#' title='Click to toggle minimize/restore all windows'>Show Desktop</a>";
        tmpBtn.onmouseup = _windowDashboardMinimizeAll;
        _topBar.appendChild(tmpBtn);

        _fullScreenBtn = document.createElement("div");
		_fullScreenBtn.setAttribute("class", "DesktopDashboard-button DesktopDashboard-button-left");
        this.redrawFullScreenButton();
        _fullScreenBtn.onmouseup = Desktop.desktop.toggleFullScreen;
        _topBar.appendChild(_fullScreenBtn);
        
        
        //user with lock on far right.. because it is the highest priority for user to see
		tmpBtn = document.createElement("div");
		tmpBtn.setAttribute("class", "DesktopDashboard-button-right");
		tmpBtn.setAttribute("id", "DesktopDashboard-serverConnectionStatus");
		tmpBtn.setAttribute("title", "Click to attempt to reconnect the server. You could also try refreshing the page, or if the problem persists contact the ots admins.");
		tmpBtn.style.display = "none";
		tmpBtn.style.color = "rgb(255,150,0)";
		_topBar.appendChild(tmpBtn);
        		
        //user with lock on far right.. because it is the highest priority for user to see
        tmpBtn = document.createElement("div");
		tmpBtn.setAttribute("class", "DesktopDashboard-button-right");
		tmpBtn.setAttribute("id", "DesktopDashboard-userWithLock");
		tmpBtn.style.display = "none";
		_topBar.appendChild(tmpBtn);
                
        tmpBtn = document.createElement("div");
		tmpBtn.setAttribute("class", "DesktopDashboard-button-right");
        tmpBtn.innerHTML = "<a target='_blank' href='" + 
        	"https://cdcvs.fnal.gov/redmine/projects/otsdaq/wiki/Otsdaq_User_Manual" +
        	" 'title='Click to open ots documentation in a new tab' ><img src='/WebPath/images/dashboardImages/icon-Help.png'></a>";
        _topBar.appendChild(tmpBtn);
        
        if(Desktop.desktop.security == Desktop.SECURITY_TYPE_DIGEST_ACCESS || 
        		Desktop.desktop.security == Desktop.SECURITY_TYPE_NONE) //dont show features if in wizard mode
        {   
			tmpBtn = document.createElement("div");
			tmpBtn.setAttribute("class", "DesktopDashboard-button-right DesktopDashboard-user-account DesktopDashboard-user-logout");
			tmpBtn.innerHTML = "<a href='#' title='Click to sign out of your account'>Sign out</a>";
			tmpBtn.onmouseup = Desktop.logout;
			_topBar.appendChild(tmpBtn);
			
			tmpBtn = document.createElement("div");
			tmpBtn.setAttribute("class", "DesktopDashboard-button-right");
			tmpBtn.setAttribute("id", "DesktopDashboard-settings-icon");
			tmpBtn.innerHTML = "<a href='Javascript:var win = Desktop.desktop.addWindow(\"Settings\",Desktop.desktop.login.getUsername()," +
				"\"/WebPath/html/UserSettings.html\",true);'  title='Click to open settings window'><img src='/WebPath/images/dashboardImages/icon-Settings.png'></a>";
			_topBar.appendChild(tmpBtn);
					
			tmpBtn = document.createElement("div");
			tmpBtn.setAttribute("class", "DesktopDashboard-user-account");
			tmpBtn.setAttribute("id", "DesktopDashboard-user-displayName");
			tmpBtn.innerHTML = "";
			_topBar.appendChild(tmpBtn);
        }    
        else
        	Debug.log("Desktop Dashboard is in Wizard mode",Debug.LOW_PRIORITY);
        
        this.dashboardElement.appendChild(_topBar);
        Debug.log("Desktop Dashboard Top Bar created",Debug.LOW_PRIORITY);
        
        //create window dashboard
        _windowDashboard = document.createElement("div");
        _windowDashboard.setAttribute("id", "DesktopDashboard-windowDashboard");
        _windowDashboard.style.position = "absolute";
        _windowDashboard.style.zIndex = z;        
        _windowDashboard.style.backgroundColor = _defaultDashboardColor;
        _toggleWindowDashboard(0,_displayWindowDashboard); //set initial value
        this.updateWindows();
        this.dashboardElement.appendChild(_windowDashboard);
        

        _dashboardColorPostbox = document.createElement("div");
        _dashboardColorPostbox.setAttribute("id", "DesktopContent-dashboardColorPostbox");
        _dashboardColorPostbox.style.display = "none";
        _dashboardColorPostbox.innerHTML = _defaultDashboardColor; //init to color string
        this.dashboardElement.appendChild(_dashboardColorPostbox);
        
        
	   		//add mouse handlers
	   	_windowDashboard.onmousemove = Desktop.handleWindowMouseMove;
	   	_windowDashboard.onmousedown = Desktop.handleWindowMouseDown;
	   	_windowDashboard.onmouseup = Desktop.handleWindowMouseUp;

	   	_windowDashboardWindowCSSRule = _getDashboardWindowWidthCSSRule(); //get CSS rule for the dashboard window divs
	   	        	   	
        Debug.log("Desktop Window Dashboard created",Debug.LOW_PRIORITY);

        Debug.log("Desktop Dashboard created",Debug.LOW_PRIORITY);
    }

}










