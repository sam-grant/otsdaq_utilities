//=====================================================================================
//
//	Created Dec, 2012
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	DesktopLogin.js
//
//	This is the desktop code for the user interface for ots. ots is the DAQ
// 		and control software for the Fermi Strips Telescope.
//
//	The desktop consists of a dashboard and an arbitrary amount of windows and a login
//
//=====================================================================================





if (typeof Debug == 'undefined') 
	console.log('ERROR: Debug is undefined! Must include Debug.js before Desktop.js');
	
if (typeof Desktop == 'undefined') 
	console.log('ERROR: Desktop is undefined! Must include Desktop.js before DesktopLogin.js');
else {

	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	//define Desktop.login to return dashboard class	
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	Desktop.login = function(enabled) {
        if(false === (this instanceof Desktop.login)) {
			//here to correct if called as "var v = Desktop.login();"
			//	instead of "var v = new Desktop.login();"
	        return new Desktop.login(enabled);
	    }
	    
	    //------------------------------------------------------------------
		//create private members variables ----------------------
		//------------------------------------------------------------------
		
		var _DEFAULT_SESSION_STRING_LEN = 512;
		var _DEFAULT_COOKIE_STRING_LEN = 512;
		var _DEFAULT_COOKIE_DURATION_DAYS = 1;
		var _DEFAULT_REMEMBER_ME_DURATION_DAYS = 30;
		var _DEFAULT_PASSWORD_MIN_LEN = 4;
		var _DEFAULT_PASSWORD_MAX_LEN = 50;
		var _DEFAULT_USER_MIN_LEN = 4;
		var _DEFAULT_USER_MAX_LEN = 50;
		
		var _cookieCodeStr = "otsCookieCode";
		var _cookieUserStr = "otsCookieUser";
		var _cookieRememberMeStr = "otsRememberMeUser";
		var _user = "";
		var _displayName = "No-Login";
		var _permissions = 0;
		var _cookieCode = 0;
		var _cookieTime = 0;
		var _sessionId = 0; 
		var _uid = 0;
		var _badSessionIdCount = 0;
		
		var _areLoginInputsValid = false;
		var _killLogoutInfiniteLoop = false;
		var _keepFeedbackText = false;
		var _keptFeedbackText = "";
		
		var _loginDiv;
		
		//user preferences
		var _userPref_bgColor, _userPref_dbColor, _userPref_winColor, _userPref_layout;
		var _applyUserPreferences;
		
	    //------------------------------------------------------------------
		//create public members variables ----------------------
		//------------------------------------------------------------------
   	    
		this.loginDiv;
		   	    
	    //------------------------------------------------------------------
		//create PRIVATE members functions ----------------------
		//------------------------------------------------------------------
		
		//_closeLoginPrompt
		//	pass isLoginSuccess=true if called upon successful login!
			//	if true refresh user display name and desktop icons will be created 
		var _closeLoginPrompt = function(isLoginSuccess) {
		
			//remove Login
			var ldiv = document.getElementById("Desktop-loginDiv");
			if(ldiv) { //should not be any login div so delete 
			
				Debug.log("found login div and deleted",Debug.LOW_PRIORITY);
				ldiv.parentNode.removeChild(ldiv);
			}
			
			if(isLoginSuccess) {
				
				//update display name
				ldiv = document.getElementById("DesktopDashboard-user-displayName");
				var tmpStr = "Welcome to ots, " + _displayName;
				
				if(ldiv.innerHTML != tmpStr) {
					//if _display name is different then close all windows!
					Debug.log("Desktop.desktop.closeAllWindows() for new user",Debug.LOW_PRIORITY);
					Desktop.desktop.closeAllWindows();
				}
				ldiv.innerHTML = tmpStr;				
				
				//reset desktop based on user's permissions
				Desktop.desktop.resetDesktop(_permissions);			
			}
		}
		
		//_loginPrompt --
			// this function brings up login welcome screen
			// and logs out. This should be called on manual
			// "logout" also.
		var _loginPrompt = function() {
		
			Debug.log("loginPrompt " + _keepFeedbackText,Debug.LOW_PRIORITY);
		
			_deleteCookies(); //local logout
			
			//refresh screen to nothing and then draw login
			_closeLoginPrompt();
			
			var ldiv;			
			//create prompt functionality
			ldiv = document.createElement("div");
			ldiv.setAttribute("id", "Desktop-loginDiv"); //this div holds everything else and is what is delete at start
			ldiv.style.width = Desktop.desktop.getDesktopWidth() + "px";
			ldiv.style.height = Desktop.desktop.getDesktopHeight() + "px";
		
				//create table just to center content easily, especially on window resize
			var str;
			str = "<table width='100%' height='100%'><td valign='middle' align='center'>";
			str += "<b><u>Welcome to ots!</u></b><br /><br />";
			str += "<table><td align='right'><div id='Desktop-loginContent'></div></td></table></td></table>";
			ldiv.innerHTML = str;
			_loginDiv.appendChild(ldiv); //add centering elements to page
			
			//now have centered in page div as ldiv
			ldiv = document.getElementById("Desktop-loginContent");

			str = "";
			var rememberMeName = _getCookie(_cookieRememberMeStr); //if remember me cookie exists, then use saved name assume remember me checked
			str += "Username: <input id='loginInput0' type='email' spellcheck='false' value='" + 
				(rememberMeName?rememberMeName:"") + "'/>";
			str += "<br />";
			str += "<div id='loginInputRememberMeDiv'><input type='checkbox' id='loginInputRememberMe' " +
				(rememberMeName?"checked":"") + "> Remember me</div>";
			str += "Password: <input id='loginInput1' type='password' /><br />";
			str += "<div id='loginRetypeDiv' style='display:none' >Re-type Password: <input id='loginInput2' type='password' /><br /></div>";
			str += "<div id='newAccountCodeDiv' style='display:none' >New Account Code: <input id='loginInput3' type='text' /><br /></div>";
			str += "<a target='_blank' href='" + 
				"https://docs.google.com/document/d/1Mw4HByYfLo1bO5Hy9npDWkD4CFxa9xNsYZ5pJ7qwaTM/edit?usp=sharing" + 
				"' title='Click to open Help documentation' ><img src='../images/dashboardImages/icon-Help.png'></a>";
			str += "<a href='#' onmouseup='Desktop.desktop.login.promptNewUser(this); return false;' style='margin:0 100px 0 50px'>New User?</a>";
			str += "<input type='submit' class='DesktopDashboard-button' value='    Login    ' onmouseup='Desktop.desktop.login.attemptLogin();' /><br />"
			
			str += "<div id='loginFeedbackDiv'>" + (_keepFeedbackText?_keptFeedbackText:"") + "</div>";
			_keepFeedbackText = false;
			
			if(!window.chrome)
				str += "<a href='http://www.google.com/chrome'>ots works best in the Chrome web browser - you should switch!</a>"; 
			ldiv.innerHTML = str; //add login forms to page
						   		
				//if rememberMe then set password as focus
				//else set user name as focus, 
				//and capture enter button in the text fields
			if(rememberMeName)
				document.getElementById('loginInput1').focus();
			else
				document.getElementById('loginInput0').focus();
			
			for(var i=0;i<4;++i) {
				document.getElementById('loginInput'+i).onkeydown = function(e) {
					if(e.keyCode == 13) Desktop.desktop.login.attemptLogin(); //if enter, login
					else if(e.keyCode == 9) {		//if tab focus on next box						
						var newFocusIndex = parseInt(this.id[this.id.length-1])+(e.shiftKey?-1:1);
						if(newFocusIndex != 0 && newFocusIndex != 1 && 
								document.getElementById('loginRetypeDiv').style.display == "none") //skip 2nd input if not displayed
							newFocusIndex += e.shiftKey?2:-2; //take 2 steps back (from 1 step forward)	
						newFocusIndex = (newFocusIndex + 4)%4;
						document.getElementById('loginInput'+newFocusIndex).focus();
					}
					else if((e.keyCode >= 48 && e.keyCode <= 57) || //accept numbers and shifted numbers
						(e.keyCode >= 96 && e.keyCode <= 105) ||	//accept keypad numbers
						(e.keyCode >= 65 && e.keyCode <= 90) || //accept letters and shifted letters
						e.keyCode == 46 || e.keyCode == 8 || //delete, backspace
						e.keyCode == 35 || e.keyCode == 36 ||  //home and end
						(e.keyCode >= 37 && e.keyCode <= 40)) { //arrows are accepted 
						return true;
					}
					return false; //to stop tab and enter (and unaccepted chars) from propagating to window
				}; 	
				
				document.getElementById('loginInput'+i).onkeyup = _checkLoginInputs;
			}
		}
		
		//_checkLoginInputs --
		//		verify that the three inputs are valid when creating an account
		// 		if not creating account, let LoginAttempt validate
		var _checkLoginInputs = function() {
		
			var x = [];
			for(var i=0;i<3;++i) x[i] = document.getElementById('loginInput'+i).value;
			
			document.getElementById('loginFeedbackDiv').style.color = ""; //reset color to default inherited style			
			if(document.getElementById('loginRetypeDiv').style.display != "none") { //3 input case, so check

				_areLoginInputsValid = false; //assume invalid 
				
				if(x[1] != x[2]) {
					document.getElementById('loginFeedbackDiv').innerHTML = "Passwords do not match"; return;
				}
				if(x[1].length < _DEFAULT_PASSWORD_MIN_LEN) {
					document.getElementById('loginFeedbackDiv').innerHTML = "Passwords must be at least " + _DEFAULT_PASSWORD_MIN_LEN + " characters"; return;
				}
				if(x[1].length > _DEFAULT_PASSWORD_MAX_LEN) {
					document.getElementById('loginFeedbackDiv').innerHTML = "Passwords must be at most " + _DEFAULT_PASSWORD_MAX_LEN + " characters"; return;
				}
				if(x[0].length < _DEFAULT_USER_MIN_LEN) {
					document.getElementById('loginFeedbackDiv').innerHTML = "User name must be at least " + _DEFAULT_USER_MIN_LEN + " characters"; return;
				}
				if(x[0].length > _DEFAULT_USER_MAX_LEN) {
					document.getElementById('loginFeedbackDiv').innerHTML = "User name must be at most " + _DEFAULT_USER_MAX_LEN + " characters"; return;
				}					
				
				_areLoginInputsValid = true;
				document.getElementById('loginFeedbackDiv').innerHTML = "Passwords are valid!";
				document.getElementById('loginFeedbackDiv').style.color = "RGB(100,255,150)";
			}		
				
		}
		
		//_setCookie --
		// 		use this as only method for refreshing and setting login cookie!!
		//		sets 2 cookies, cookieCode to code and userName to _user
		var _setCookie = function(code) {
			if(_user == "" || !code.length || code.length < 2) return; //only refresh/set cookies if valid user and cookie code
			_cookieCode = code;	//set local cookie code values
			var exdate = new Date();
			exdate.setDate(exdate.getDate() + _DEFAULT_COOKIE_DURATION_DAYS);
			var c_value;
			c_value = escape(code) + ((_DEFAULT_COOKIE_DURATION_DAYS==null) ? "" : "; expires="+exdate.toUTCString());
			document.cookie= _cookieCodeStr + "=" + c_value;
			c_value = escape(_user) + ((_DEFAULT_COOKIE_DURATION_DAYS==null) ? "" : "; expires="+exdate.toUTCString());
			document.cookie= _cookieUserStr + "=" + c_value;
            
			var ccdiv = document.getElementById("DesktopContent-cookieCodeMailbox");
            ccdiv.innerHTML = _cookieCode; //place in mailbox for desktop content
			ccdiv = document.getElementById("DesktopContent-updateTimeMailbox");
            ccdiv.innerHTML = (new Date()).getTime(); //set last time of cookieCode update
            _cookieTime = parseInt(ccdiv.innerHTML);
		} 
		
		//_getCookie --
			// get login cookie
		var _getCookie = function(c_name) {
			var i,x,y,ARRcookies=document.cookie.split(";");
			for (i=0;i<ARRcookies.length;i++)
			{
				x=ARRcookies[i].substr(0,ARRcookies[i].indexOf("="));
				y=ARRcookies[i].substr(ARRcookies[i].indexOf("=")+1);
				x=x.replace(/^\s+|\s+$/g,"");
				if (x==c_name)
				{
					return unescape(y);
				}
			}  
		}
		
		//_deleteCookies --
		// 		delete login cookies, effectively logout
		var _deleteCookies = function() {
			_cookieCode = 0;
			Debug.log("Delete cookies",Debug.LOW_PRIORITY);
			var c_value;
			c_value = "; expires=Thu, 01 Jan 1970 00:00:01 GMT;";
			document.cookie= _cookieCodeStr + "=" + c_value;
			c_value = "; expires=Thu, 01 Jan 1970 00:00:01 GMT;";
			document.cookie= _cookieUserStr + "=" + c_value;
		}
		
		//_getUniqueUserId --
			//generate a unique user ID (UUID)
		var _getUniqueUserId = function() {
			return '4xxx'.replace(/[x]/g, function(c) {
			    var r = Math.random()*16|0, v = r;
			    return v.toString(16);
			});		
		}
		
		//_checkCookieLogin --
			// Can only be called if sessionId is already set
			// if cookie, check with server that it is valid
			// if no cookie open login prompt
		var _checkCookieLogin = function() {					
			if(_sessionId.length != _DEFAULT_SESSION_STRING_LEN) return; //if no session id, fail			
	
			var code = _getCookie(_cookieCodeStr);
			_user = _getCookie(_cookieUserStr);
			if ((code != null && code != "") &&
				(_user != null && _user != "")) {
				//if cookie found, submit cookieCode and jumbled user to server to check if valid					
				Desktop.XMLHttpRequest("LoginRequest?RequestType=checkCookie","uuid="+_uid+"&ju="+_jumble(_user,_sessionId)+"&cc="+code,_handleCookieCheck);
			}
			else {					
				Debug.log("No cookie found",Debug.LOW_PRIORITY);
				_loginPrompt();		//no cookie, so prompt user  
			}
		}
		
		//_handleLoginAttempt --
			// handler for login request from server
			// current cookie code and display name is returned on success
			// on failure, go to loginPrompt
		var _handleLoginAttempt = function(req) {
			Debug.log("Received login attempt back",Debug.LOW_PRIORITY);
			
			var cookieCode = Desktop.getXMLValue(req,"CookieCode");
			_displayName = Desktop.getXMLValue(req,"DisplayName");
			_permissions = Desktop.getXMLValue(req,"desktop_user_permissions");
			if(cookieCode && _displayName && cookieCode.length == _DEFAULT_COOKIE_STRING_LEN) { 	//success!
				Debug.log("Login Successful",Debug.LOW_PRIORITY);
				_setCookie(cookieCode); //update cookie					
				_applyUserPreferences(req);
				
				var activeSessionCount = parseInt(Desktop.getXMLValue(req,"user_active_session_count"));
				if(activeSessionCount)
				{
					Debug.log("Found other active sessions: " + activeSessionCount,Debug.LOW_PRIORITY);			
					_offerActiveSessionOptions(activeSessionCount);
				}
				else				
					_closeLoginPrompt(1); //clear login prompt
			}
			else { //login failed
				Debug.log("Login failed " + cookieCode + " - " + _displayName,Debug.LOW_PRIORITY);				
					
				//set and keep feedback text
				if(cookieCode == "1") //invalid uuid
					_keptFeedbackText = "Sorry, your session has expired. Try again.";
				else if(req)
					_keptFeedbackText = "Username/Password not correct.";
				else
					_keptFeedbackText = "ots Server failed.";
				_keepFeedbackText = true;
	      		for(var i=1;i<3;++i) document.getElementById('loginInput'+i).value = ""; //clear input boxes

	      		//refresh session id
	    		_uid = _getUniqueUserId();
				Desktop.XMLHttpRequest("LoginRequest?RequestType=sessionId","uuid="+_uid,_handleGetSessionId);
	      	}
		}
		
		//_handleCookieCheck --
			// handler for cookie check request from server
			// current cookie code and display name is returned on success
			// on failure, go to loginPrompt
		var _handleCookieCheck = function(req) {			
			var cookieCode = Desktop.getXMLValue(req,"CookieCode");
			_displayName = Desktop.getXMLValue(req,"DisplayName");
			_permissions = Desktop.getXMLValue(req,"desktop_user_permissions");
			if(cookieCode && _displayName && cookieCode.length == _DEFAULT_COOKIE_STRING_LEN) { 	//success!
				Debug.log("Cookie is good",Debug.LOW_PRIORITY);
				_setCookie(cookieCode); //update cookie	
				_applyUserPreferences(req);
				_closeLoginPrompt(1); //clear login prompt
			}
			else {
				Debug.log("Cookie is bad " + cookieCode.length + _displayName,Debug.LOW_PRIORITY);
				_loginPrompt();		//no cookie, so prompt user 
			}
		}
		
		//_handleGetSessionId --
		//	handler called when session id is returned from server
		//	set _sessionId
		//	the chain is passed on to check cookie login from here
		var _handleGetSessionId = function(req) {
			_sessionId = 0;
			if(!req || req.responseText.length != _DEFAULT_SESSION_STRING_LEN) { //sessionId failed
				Debug.log("Invalid session ID",Debug.HIGH_PRIORITY);

                if(!req) 
                {
                	_loginPrompt();
                	return; //do nothing, because server failed
                }
				//try again
				_uid = _getUniqueUserId();
				
				Debug.log("UUID: " + _uid);
				++_badSessionIdCount;
				if (_badSessionIdCount < 10)
	                Desktop.XMLHttpRequest("LoginRequest?RequestType=sessionId","uuid="+_uid,_handleGetSessionId); //if disabled, then cookieCode will return 0 to desktop
				else
					alert("Cannot establish session ID - failed 10 times");
				
				return;
			} 
			_badSessionIdCount = 0;

			//successfully received session ID			
			_sessionId = req.responseText;			
			_checkCookieLogin();
			_killLogoutInfiniteLoop = false;
		}

		//_offerActiveSessionOptions --
			// prompt user with option to close other sessions
		var _offerActiveSessionOptions = function(cnt) {
		
			ldiv = document.getElementById("Desktop-loginContent");

			str = "";
			str += "<center>Warning! You currently have " + cnt + " other active session" + (cnt > 1?"s":"") + ".<br />";
			str += "<div id='loginFeedbackDiv'>You can opt to force logout the other session" + (cnt > 1?"s":"") + ", " + 
				"or alternatively leave your other session" + (cnt > 1?"s":"") + " active and continue.</div><br />";
			str += "<input type='submit' class='DesktopDashboard-button' value=' Logout Other Sessions ' " + 
				"onmouseup='Desktop.desktop.login.activeSessionLogoutOption();' />";
			str += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
			str += "<input type='submit' class='DesktopDashboard-button' value=' Ignore and Continue ' " +
				"onmouseup='Desktop.desktop.login.activeSessionIgnoreOption();' /></center>";
			ldiv.innerHTML = str; //add login forms to page
		}
		
		var _jumble = function (u,s) {    
		    if(s.length%2) return "";	
		    var x = [];
		    var l = s.length/2;
		    var ss = l;
		    var p = 0;
		    var c = parseInt(s[p*2]+s[p*2+1],16);		    
		    for(var i=0;i<l;++i) x[i] = 0;		    
		    var i = 0,h;    
		    var s0 = s[p*2],s1 = s[p*2+1];
		    while(l > 0) {
		        p = (p + parseInt(s0+s1,16)) % ss;
		        while(x[p]) p = (p+1) % ss;
		        x[p] = 1; s0 = s[p*2]; s1 = s[p*2+1];
		        c = (c + parseInt(s[p*2]+s[p*2+1],16) + parseInt((i==0)?u.length:u.charCodeAt(i-1))) % ss;        
		        h = c.toString(16); if(h.length == 1) h = "0" + h;
		        s = s.substr(0,p*2) + h + s.substr(p*2+2,s.length-p*2-2);
		        --l; if(i==u.length) i = 1; else ++i;
		    }
		    return s;
		}

		var _saveUsernameCookie = function () {    
			Debug.log("Desktop _saveUsernameCookie _user " + _user);  			
			var exdate = new Date();
			exdate.setDate(exdate.getDate() + _DEFAULT_REMEMBER_ME_DURATION_DAYS);
			var c_value;
			c_value = escape(_user) + ((_DEFAULT_REMEMBER_ME_DURATION_DAYS==null) ? "" : "; expires="+exdate.toUTCString());
			document.cookie= _cookieRememberMeStr + "=" + c_value;
		}
		
		var _deleteUsernameCookie = function () {  
			Debug.log("Desktop _deleteUsernameCookie _user " + _user); 	
			var c_value;
			c_value = "; expires=Thu, 01 Jan 1970 00:00:01 GMT;";		
			document.cookie= _cookieRememberMeStr + "=" + c_value;
		}
				
		//------------------------------------------------------------------
		//create PUBLIC members functions ----------------------
		//------------------------------------------------------------------

		//logout ~
		//	Public logout function. Logs out at server and locally.
		this.logout = function() {
       		Debug.log("Desktop Logout occured " + _killLogoutInfiniteLoop,Debug.HIGH_PRIORITY);  
       		
       		if(_cookieCode && !_killLogoutInfiniteLoop)
       			Desktop.XMLHttpRequest("LoginRequest?RequestType=logout"); //server logout
			_deleteCookies(); //local logout			
			
       		//start new session
			if(!_killLogoutInfiniteLoop)
			{
	     		_uid = _getUniqueUserId();
				Desktop.XMLHttpRequest("LoginRequest?RequestType=sessionId","uuid="+_uid,_handleGetSessionId);
				Debug.log("UUID: " + _uid)
			}
			
         	_killLogoutInfiniteLoop = true; //prevent infinite logout requests, on server failure
		}
		
		//getCookieCode --
		// The public getCookieCode function does not actually check the cookie
		// it is the server which controls if a cookieCode is still valid.
		// This function just refreshes the cookie and returns the local cookieCode value.
		this.getCookieCode = function() {
			_setCookie(_cookieCode); //refresh cookies
			return _cookieCode;
		}
        
		this.updateCookieFromContent = function(newTime) {
            _cookieTime = newTime; //set immediately so timer doesn't trip same issue
            var ccdiv = document.getElementById("DesktopContent-cookieCodeMailbox");
            _setCookie(ccdiv.innerHTML);
        }
        
		this.getCookieTime = function() {return _cookieTime;}		
		this.getUserDisplayName = function() {return _displayName;}
		this.getUsername = function() {return _user;}
		
		this.redrawLogin = function() {
			var ldiv = document.getElementById("Desktop-loginDiv");
			if(!ldiv) return;
				
			ldiv.style.width = Desktop.desktop.getDesktopWidth() + "px";
			ldiv.style.height = Desktop.desktop.getDesktopHeight() + "px";
		}
		
		//toggle re-type password for new user
		this.promptNewUser = function(linkObj) {
			document.getElementById('loginFeedbackDiv').innerHTML = ""; //clear feedback text
       		Debug.log("Desktop Login Prompt New User",Debug.LOW_PRIORITY);
      		document.getElementById('loginRetypeDiv').style.display = 
      			document.getElementById('loginRetypeDiv').style.display == "none"?"inline":"none";
      		document.getElementById('newAccountCodeDiv').style.display = 
      			document.getElementById('newAccountCodeDiv').style.display == "none"?"inline":"none";      			
      			 
	      	for(var i=1;i<4;++i) document.getElementById('loginInput'+i).value = ""; //clear input boxes
      		
      		linkObj.innerHTML = document.getElementById('loginRetypeDiv').style.display == "none"?"New User?":"Have an Account?";      		
		}
		
		this.attemptLogin = function() {
       		Debug.log("Desktop Login Prompt Attempt Login ",Debug.LOW_PRIORITY);    	
       		    			       		
			var x = [];
			for(var i=0;i<3;++i) x[i] = document.getElementById('loginInput'+i).value;
       		
       		if(document.getElementById('loginRetypeDiv').style.display != "none" && !_areLoginInputsValid) {
       			Debug.log("Invalid Inputs on new login attempt",Debug.LOW_PRIORITY); return;
       		}
       					
			document.getElementById('loginFeedbackDiv').innerHTML = ""; //clear feedback text
			document.getElementById('loginFeedbackDiv').style.color = ""; //reset color to default inherited style
			
       		if(x[0] == "" || x[1] == "") {
				document.getElementById('loginFeedbackDiv').innerHTML = "Some fields were left empty."; return;
			}
			_user = x[0]; //set local user		
			
			//if remember me checked, refresh cookie.. else delete whatever is there
       		if(document.getElementById('loginInputRememberMe').checked) _saveUsernameCookie(); 
       		else _deleteUsernameCookie();
       		
       		Desktop.XMLHttpRequest("LoginRequest?RequestType=login","uuid="+_uid+"&nac="+document.getElementById('loginInput3').value
       			+"&ju="+_jumble(x[0],_sessionId)+"&jp="+_jumble(x[1],_sessionId),_handleLoginAttempt);        		
		}
			
		//_applyUserPreferences
		//	apply user preferences based on req if provided
		//		window color should always have alpha of 0.9
		_applyUserPreferences = this.applyUserPreferences = function(req) {
		
			if (typeof req != 'undefined') {
					//update user pref if req is defined as xml doc
				_userPref_bgColor = Desktop.getXMLValue(req,"pref_bgcolor");
				_userPref_dbColor = Desktop.getXMLValue(req,"pref_dbcolor");
				_userPref_winColor = Desktop.getXMLValue(req,"pref_wincolor");
				_userPref_layout = Desktop.getXMLValue(req,"pref_layout");	
			}					
			
       		Desktop.desktop.dashboard.setDefaultDashboardColor(_userPref_dbColor);
       		Desktop.desktop.setDefaultWindowColor(_userPref_winColor);
       		document.body.style.backgroundColor = _userPref_bgColor;
		}
		
		this.getUserDefaultLayout = function(i) { return _userPref_layout.split(";")[i];}
		
		this.activeSessionLogoutOption = function() {
			Debug.log("Desktop activeSessionLogoutOption");
       		Desktop.XMLHttpRequest("LoginRequest?RequestType=logout","LogoutOthers=1"); //server logout of other active sessions
			_closeLoginPrompt(1); //clear login prompt - pass 1 just so it will check new username
		
		}
		this.activeSessionIgnoreOption = function() {
			Debug.log("Desktop activeSessionIgnoreOption");
			_closeLoginPrompt(1); //clear login prompt - pass 1 just so it will check new username
		}
		
		//------------------------------------------------------------------
		//handle class construction ----------------------
		//------------------------------------------------------------------
		
		
		this.loginDiv = _loginDiv = document.createElement("div"); //create holder for anything login			
			
		//initially 
			//submit unique uid and get session ID from server using GetSessionId request
			//check cookie to see if still valid
				//send isStillActive request to server to see if CookieCode is still valid
			//if no valid cookie, prompt user
				//user can login or (if first time) set password
				//user and jumbled password sent to server for login
			//if login successful, loginDiv is removed from desktop and cookieCode used by client

		_uid = _getUniqueUserId();
		if(enabled)	Desktop.XMLHttpRequest("LoginRequest?RequestType=sessionId",
						"uuid="+_uid,_handleGetSessionId); //if disabled, then cookieCode will return 0 to desktop
		Debug.log("UUID: " + _uid);
        Debug.log("Desktop Login created",Debug.LOW_PRIORITY);
	}	
}