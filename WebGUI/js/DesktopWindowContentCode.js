//=====================================================================================
//
//	Created Jan, 2013
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	DesktopWindowContentCode.js
//
//  Requirements: 
//   1. paste the following: 
//				
//				<script type="text/JavaScript" src="/WebPath/js/Globals.js"></script>	
//				<script type="text/JavaScript" src="/WebPath/js/Debug.js"></script>	
//				<script type="text/JavaScript" src="/WebPath/js/DesktopWindowContentCode.js"></script>
//
//		...anywhere inside the <head></head> tag of a window content html page
//	 2. for proper functionality certain handlers are used:
//   		cannot overwrite handlers for window: onfocus, onscroll, onblur, onmousemove
//			(if you must overwrite, try to call the DesktopContent handlers from your handlers)
//
//	Recommendations:
//	 1. use Debug to output status and errors, e.g.:
//				Debug.log("this is my status",Debug.LOW_PRIORITY); //LOW_PRIORITY, MED_PRIORITY, HIGH_PRIORITY
//	 2. call window.focus() to bring your window to the front of the Desktop
//
//	The code of Requirement #1 should be inserted in the header of each page that will be 
//  the content of a window in the ots desktop.
//
//  This code handles bringing the window to the front when the content
//  is clicked or scrolled.
//
//	This code also handles server requests and response handlers for the content code:
//		-DesktopContent.XMLHttpRequest(requestURL, data, returnHandler <optional>, reqIndex <optional>)
//			... to make server request, returnHandler is called with response in req and integer reqIndex if user defined
//
//		-DesktopContent.getXMLValue(req, name)
//			... to get string value from XML server response. field name is needed.
//
//  ... or to get array of elements by tag name 
//		var els = req.responseXML.getElementsByTagName(name);
//		for(var i=0;i<els.length;++i)
//			Debug.log(els[i].getAttribute("value"));
//
//
//	Additional Functionality:
//		DesktopContent.popUpVerification(prompt, func [optional], val [optional], bgColor [optional], textColor [optional])
//		DesktopContent.getWindowWidth()
//		DesktopContent.getWindowHeight()
//		DesktopContent.getMouseX()
//		DesktopContent.getMouseY()
//		DesktopContent.getDefaultWindowColor() 	 // returns "rgb(#,#,#)"
//  	DesktopContent.getDefaultDashboardColor()// returns "rgb(#,#,#)"
//		DesktopContent.getDefaultDesktopColor()  // returns "rgb(#,#,#)"
//		DesktopContent.getUsername()
//
//=====================================================================================

var DesktopContent = DesktopContent || {}; //define Desktop namespace

if (typeof Debug == 'undefined') 
	alert('ERROR: Debug is undefined! Must include Debug.js before DesktopWindowContentCode.js');
if (typeof Globals == 'undefined') 
	alert('ERROR: Globals is undefined! Must include Globals.js before DesktopWindowContentCode.js');


//"public" function list: 
//	DesktopContent.XMLHttpRequest(requestURL, data, returnHandler, reqIndex, progressHandler, sequence)
//	DesktopContent.getXMLValue(req, name)
//	DesktopContent.getXMLAttributeValue(req, name, attribute)
//	DesktopContent.popUpVerification(prompt, func, val, bgColor, textColor)
//	DesktopContent.getWindowWidth()
//	DesktopContent.getWindowHeight()
//	DesktopContent.getMouseX()
//	DesktopContent.getMouseY()
//	DesktopContent.getDefaultWindowColor()
//  DesktopContent.getDefaultDashboardColor()
//	DesktopContent.getDefaultDesktopColor()
//	DesktopContent.getUsername()

//"private" function list:
//	DesktopContent.init()
//	DesktopContent.getParameter(index)
//	DesktopContent.handleFocus(e)
//	DesktopContent.handleBlur(e)
//	DesktopContent.handleScroll(e)
//	DesktopContent.mouseMove(mouseEvent)
//	DesktopContent.checkCookieCodeRace()
//	DesktopContent.clearPopUpVerification(func)
//	DesktopContent.parseColor(colorStr)


DesktopContent._isFocused = false;
DesktopContent._theWindow = 0;
DesktopContent._myDesktopFrame = 0;
DesktopContent._zMailbox = 0;
DesktopContent._mouseOverXmailbox = 0;
DesktopContent._mouseOverYmailbox = 0;
DesktopContent._windowMouseX = -1;
DesktopContent._windowMouseY = -1;

DesktopContent._serverUrnLid = 0;
DesktopContent._localUrnLid = 0;

DesktopContent._cookieCodeMailbox = 0;
DesktopContent._updateTimeMailbox = 0;
DesktopContent._needToLoginMailbox = 0;

DesktopContent._lastCookieCode = 0;
DesktopContent._lastCookieTime = 0;


DesktopContent._verifyPopUp = 0;
DesktopContent._verifyPopUpId = "DesktopContent-verifyPopUp";

DesktopContent._windowColorPostbox = 0;
DesktopContent._dashboardColorPostbox = 0;
DesktopContent._desktopColor = 0;


//=====================================================================================
//initialize content's place in the world
// caution when using "window" anywhere outside this function because
//  desktop window can be at different levels depending on page depth (page may be inside frame)
// use instead DesktopContent._theWindow
DesktopContent.init = function() {

	var tmpCnt = 0;
	DesktopContent._theWindow = self;
	while(tmpCnt++ < 5 && DesktopContent._theWindow &&  //while can not find the top window frame in the desktop
			DesktopContent._theWindow.window.name.search("DesktopWindowFrame") < 0)
		DesktopContent._theWindow = DesktopContent._theWindow.parent;
	DesktopContent._theWindow = DesktopContent._theWindow.window;

	DesktopContent._myDesktopFrame        = DesktopContent._theWindow.parent.document.getElementById(DesktopContent._theWindow.name);
	DesktopContent._zMailbox              = DesktopContent._theWindow.parent.document.getElementById("Desktop-windowZmailbox");
	DesktopContent._mouseOverXmailbox     = DesktopContent._theWindow.parent.document.getElementById("Desktop-mouseOverXmailbox");
	DesktopContent._mouseOverYmailbox     = DesktopContent._theWindow.parent.document.getElementById("Desktop-mouseOverYmailbox");

	DesktopContent._cookieCodeMailbox     = DesktopContent._theWindow.parent.document.getElementById("DesktopContent-cookieCodeMailbox");
	DesktopContent._updateTimeMailbox     = DesktopContent._theWindow.parent.document.getElementById("DesktopContent-updateTimeMailbox");
	DesktopContent._needToLoginMailbox    = DesktopContent._theWindow.parent.document.getElementById("DesktopContent-needToLoginMailbox");

	DesktopContent._windowColorPostbox	  = DesktopContent._theWindow.parent.document.getElementById("DesktopContent-windowColorPostbox");
	DesktopContent._dashboardColorPostbox = DesktopContent._theWindow.parent.document.getElementById("DesktopContent-dashboardColorPostbox");
	DesktopContent._desktopColor 		  = DesktopContent._theWindow.parent.document.body.style.backgroundColor;
	
	window.onfocus = DesktopContent.handleFocus;
	window.onmousedown = DesktopContent.handleFocus;
	window.onscroll = DesktopContent.handleScroll;
	window.onblur = DesktopContent.handleBlur;	
	window.onmousemove = DesktopContent.mouseMove; //setup mouse move handler

	DesktopContent._serverUrnLid = ((DesktopContent._theWindow.parent.window.location.search.substr(1)).split('='))[1];
	if(typeof DesktopContent._serverUrnLid == 'undefined')
		Debug.log("ERROR -- Supervisor Application URN-LID not found",Debug.HIGH_PRIORITY);
	Debug.log("Supervisor Application URN-LID #" + DesktopContent._serverUrnLid);
	DesktopContent._localUrnLid = DesktopContent.getParameter(0);
	if(typeof DesktopContent._localUrnLid == 'undefined')
		DesktopContent._localUrnLid = 0;
	Debug.log("Local Application URN-LID #" + DesktopContent._localUrnLid);
}

//DesktopContent.getParameter ~
//	returns the value of the url GET parameter specified by index
DesktopContent.getParameter = function(index) {	
	// Debug.log(window.location)
	var ps = (window.location.search.substr(1)).split('&');
	if(index >= ps.length) return; //return undefined
	var vs = ps[index].split('=');
	if(vs.length < 2) return; //return undefined	
	return vs[1]; //return value
}

DesktopContent.handleFocus = function(e) {	//access z-index mailbox on desktop, increment by 1 and set parent's z-index	

	if(!DesktopContent._myDesktopFrame) return; //only happen if not part of desktop

	//commented below because, at times, desktop window movement led to wrong focus assumptions 
	//if(DesktopContent._isFocused ) {Debug.log("already"); return; }//only focus when unfocused
	DesktopContent._isFocused = true;					
	DesktopContent._myDesktopFrame.parentNode.parentNode.style.zIndex = DesktopContent._zMailbox.innerHTML;
	DesktopContent._zMailbox.innerHTML = parseInt(DesktopContent._zMailbox.innerHTML) + 1;
	return true;
}
DesktopContent.handleBlur = function(e) {			
	DesktopContent._isFocused = false;
}
DesktopContent.handleScroll = function(e) {			
	window.focus();
}
DesktopContent.mouseMove = function(mouseEvent) {			
	if(!DesktopContent._myDesktopFrame) return; //only happens if not part of desktop

	DesktopContent._windowMouseX = parseInt(mouseEvent.clientX);
	DesktopContent._windowMouseY = parseInt(mouseEvent.clientY);

	//add window frame position(absolute) + iframe position within window + mouse position within iframe
	DesktopContent._mouseOverXmailbox.innerHTML = parseInt(DesktopContent._myDesktopFrame.parentNode.parentNode.offsetLeft) +
			parseInt(DesktopContent._myDesktopFrame.offsetLeft) + DesktopContent._windowMouseX;
	DesktopContent._mouseOverYmailbox.innerHTML = parseInt(DesktopContent._myDesktopFrame.parentNode.parentNode.offsetTop) + 
			parseInt(DesktopContent._myDesktopFrame.offsetTop) + DesktopContent._windowMouseY;
}

DesktopContent.init(); //initialize handlers


//=====================================================================================
// XML request helpers
//=====================================================================================

//used by DesktopContent.XMLHttpRequest to reject multiple calls to same handler 
//(this is a result of bad error handling in the desktop window page.. so this is meant to inform the developer to fix the issue)
DesktopContent._arrayOfFailedHandlers = new Array();


//=====================================================================================
//DesktopContent.XMLHttpRequest
// forms request properly for ots server, POSTs data
// and when request is returned, returnHandler is called with 
// req result on success, if failure (e.g. due to bad url) called with 0 and
// error description in errStr.
//
// For proper request need: urnLid, cookieCode
//
// Handler always receives standard xml response if server is ok:
//  <ROOT>
//      <HEADER><CookieCode value=''/></HEADER>
//      <DATA> ... </DATA>
//  </ROOT>
//
// Where CookieCode and DisplayName can change upon every server response
//
// reqIndex is used to give the returnHandler an index to route responses to.
// Sequence is used as an alternative approach to cookieCode (e.g. ots Config Wizard).
//
DesktopContent.XMLHttpRequest = function(requestURL, data, returnHandler, 
		reqIndex, progressHandler, sequence) {

	var errStr = "";
	var req;

	//check if already marked the mailbox.. and do nothing because we know something is wrong
	if(DesktopContent._needToLoginMailbox && DesktopContent._needToLoginMailbox.innerHTML == "1")		
	{
		errStr = "Something is still wrong.";
		errStr += " (Try refreshing the page, or alert ots admins if problem persists.)";
		Debug.log("Error: " + errStr,Debug.HIGH_PRIORITY);
		req = 0; //force to 0 to indicate error
		var found = false;
		if(DesktopContent._arrayOfFailedHandlers.length < 2) //only give pop up behavior for first 2 failures (then go quiet)
		{
			for(var rh in DesktopContent._arrayOfFailedHandlers)
				if(DesktopContent._arrayOfFailedHandlers[rh] == returnHandler) 
				{
					errStr = "Blocking multiple error responses to same handler. \nPoor error handling (Developer should fix) by returnHandler: " + returnHandler;
					Debug.log(errStr.substr(0,200) + "...",Debug.HIGH_PRIORITY);
					found = true; break;
				}
		}
		else 
		{
			errStr = "Quiet Mode. Blocking multiple error responses to ALL handlers. \nPoor error handling (Developer should fix) by returnHandler: " + returnHandler;
			Debug.log(errStr.substr(0,200) + "...",Debug.HIGH_PRIORITY);
			found = true;
		}

		if(!found) DesktopContent._arrayOfFailedHandlers.push(returnHandler);

		//only call return handler once
		if(returnHandler && !found) returnHandler(req, reqIndex, errStr); 
		return;
	}

	req = new XMLHttpRequest();

	if(progressHandler) req.upload.addEventListener("progress", progressHandler, false); //add progress listener if defined

	req.onreadystatechange = function() {
		if (req.readyState==4) 
		{  //when readyState=4 return complete, status=200 for success, status=400 for fail
			if(req.status==200)
			{				
				Debug.log("Request Response Text " + req.responseText + " ---\nXML " + req.responseXML,Debug.LOW_PRIORITY);

				DesktopContent._lastCookieTime = parseInt((new Date()).getTime()); //in ms

				//check if failed due to cookieCode and go to login prompt
				if(req.responseText == Globals.REQ_NO_PERMISSION_RESPONSE) 
				{
					errStr = "Request failed do to insufficient account permissions."; 
					//return;
				}
				else if(req.responseText == Globals.REQ_USER_LOCKOUT_RESPONSE) 
				{					
					errStr = "Request failed because another user has locked ots. Put your mouse over the lock icon at the top of the dashboard to see who.";
					//return;
				}				
				else if(req.responseText == Globals.REQ_NO_LOGIN_RESPONSE) 
				{
					errStr = "Login has expired.";

					if(DesktopContent._needToLoginMailbox) //if login mailbox is valid, force login
						DesktopContent._needToLoginMailbox.innerHTML = "1"; //force to login screen on server failure                        
					//return;
				}
				else if(!sequence)
				{    

					if(!req.responseXML) //invalid XML received 
					{
						errStr = "Request response is invalid XML!";
						//return;
					}
					else
					{
						//handle cookie code mailbox
						DesktopContent._lastCookieCode = DesktopContent.getXMLValue(req,'CookieCode');
						if (typeof DesktopContent._lastCookieCode == 'undefined') 
						{ //clear req, server failed
							errStr = "Request Failed - Missing Cookie in Response.";

							if(DesktopContent._needToLoginMailbox) //if login mailbox is valid, force login
								DesktopContent._needToLoginMailbox.innerHTML = "1"; //force to login screen on server failure                        

						}
						else 
						{ //check if should update cc mailbox

							//check twice to handle race conditions with other content code
							if(parseInt(DesktopContent._updateTimeMailbox.innerHTML) < DesktopContent._lastCookieTime) //then current code is newer
							{
								DesktopContent._updateTimeMailbox.innerHTML = DesktopContent._lastCookieTime;
								DesktopContent._cookieCodeMailbox.innerHTML = DesktopContent._lastCookieCode;

								setTimeout(DesktopContent.checkCookieCodeRace, Math.random()*1000|0+500); //random wait (500-1500ms) before checking if race conditions occured
							}
						}
					}
				}
			}
			else if(req.status == 0)  //request was interrupted (probably window was closed)
			{
				Debug.log("Status=0. Likely this means a window was closed in the middle of a request. " +
						"\n(It also could mean 'potential security risk' like a cross-domain request) ",Debug.MED_PRIORITY);
				return;
			}
			else //bad address response
			{
				
				errStr = "Request Failed (code: " + req.status + ") - Bad Address:\n" + requestURL;

				if(DesktopContent._needToLoginMailbox) //if login mailbox is valid, force login
					DesktopContent._needToLoginMailbox.innerHTML = "1"; //force to login screen on server failure

				//handle multiple failed handlers
				var found = false;
				if(DesktopContent._arrayOfFailedHandlers.length < 2) //only give pop up behavior for first 2 failures (then go quiet)
				{
					for(var rh in DesktopContent._arrayOfFailedHandlers)
						if(DesktopContent._arrayOfFailedHandlers[rh] == returnHandler) 
						{
							errStr = "Blocking multiple error responses to same handle. \nPoor error handling (Developer should fix) by returnHandler: " + returnHandler;
							Debug.log(errStr.substr(0,200) + "...",Debug.HIGH_PRIORITY);
							found = true; break;
						}
				}
				else 
				{
					errStr = "Quiet Mode. Blocking multiple error responses to ALL handles. \nPoor error handling (Developer should fix) by returnHandler: " + returnHandler;
					Debug.log(errStr.substr(0,200) + "...",Debug.HIGH_PRIORITY);
					found = true;
				}

				if(!found) DesktopContent._arrayOfFailedHandlers.push(returnHandler);
				if(found) return; //do not call handler for failed server for user code multiple times..
			}

			if(errStr != "")
			{
				errStr += "\n\n(Try refreshing the page, or alert ots admins if problem persists.)";
				Debug.log("Error: " + errStr,Debug.HIGH_PRIORITY);
				//alert(errStr);
				req = 0; //force to 0 to indicate error
			}
			if(returnHandler) returnHandler(req, reqIndex, errStr);
		}
	}

	if(!sequence)
	{        
		var cc = DesktopContent._cookieCodeMailbox?DesktopContent._cookieCodeMailbox.innerHTML:""; //get cookie code from mailbox if available
		data = "CookieCode="+cc+"&"+data;
	}
	else
	{   	
		data = "sequence="+sequence+"&"+data;
	}
	var urn = DesktopContent._localUrnLid?DesktopContent._localUrnLid:DesktopContent._serverUrnLid;

	requestURL = "/urn:xdaq-application:lid="+urn+"/"+requestURL;
	Debug.log("Post " + requestURL + "\n\tData: " + data);
	req.open("POST",requestURL,true);
	req.setRequestHeader("Content-Type", "text/plain;charset=UTF-8");
	req.send(data);	
}

//check cookie code race conditions
DesktopContent.checkCookieCodeRace = function() {
	//Debug.log("Checking cookie race conditions");
	if(parseInt(DesktopContent._updateTimeMailbox.innerHTML) < DesktopContent._lastCookieTime) //then current code is newer
	{
		Debug.log("Cookie race occured!");

		DesktopContent._updateTimeMailbox.innerHTML = DesktopContent._lastCookieTime;
		DesktopContent._cookieCodeMailbox.innerHTML = DesktopContent._lastCookieCode;
	}
}

//=====================================================================================
//returns xml entry value for an attribute
DesktopContent.getXMLAttributeValue = function(req, name, attribute) {
	var els;
	if(req && req.responseXML && (els = req.responseXML.getElementsByTagName(name)).length > 0)
		return els[0].getAttribute(attribute);
	else
		return undefined;
}

//=====================================================================================
//returns xml entry value for attribue 'value'
DesktopContent.getXMLValue = function(req, name) {
	return DesktopContent.getXMLAttributeValue(req,name,"value");
}

//=====================================================================================
//popUpVerification ~~
//	asks user if sure
//	replace REPLACE in prompt with value passed as val (e.g. pass document.getElementById("myElementId").value)
//	call func (if exists) when user selects "Yes" 
//
//	Can change background color and text color with strings bgColor and textColor (e.g. "rgb(255,0,0)" or "red")
//		Default is yellow bg with black text if nothing passed.
DesktopContent.popUpVerification = function(prompt, func, val, bgColor, textColor) {		

	//	Debug.log("X: " + DesktopContent._mouseOverXmailbox.innerHTML + 
	//			" Y: " + DesktopContent._mouseOverYmailbox.innerHTML + 
	//			" W: " + DesktopContent.getWindowWidth() + 
	//			" H: " + DesktopContent.getWindowHeight());					


	//remove pop up if already exist
	if(DesktopContent._verifyPopUp) 
		DesktopContent._verifyPopUp.parentNode.removeChild(DesktopContent._verifyPopUp);

	//replace REPLACE with val
	if(val != undefined)
		prompt = prompt.replace(/REPLACE/g, val); 


	//create popup and add to body


	//setup style first
	if(!bgColor) bgColor = "rgb(255,241,189)";	//set default
	if(!textColor) textColor = "black";	//set default
	var css = "";
	//pop up div style
	css += "#" + DesktopContent._verifyPopUpId + " " +
			"{position: absolute; border-radius: 5px; padding: 10px;" +			
			"background-color: " + bgColor + "; border: 2px solid rgb(0,0,0);" +
			"color: " + textColor + ";text-align: center;" +
			"}\n\n";
	//pop up text style 
	css += "#" + DesktopContent._verifyPopUpId + "-text " +
			"{" +
			"color: " + textColor + ";width: 200px; padding-bottom: 10px;" +
			"}\n\n";
	//..and anything in the text div
	css += "#" + DesktopContent._verifyPopUpId + "-text *" +
			"{" +
			"color: " + textColor + ";" +
			"}\n\n";

	//add style element to HEAD tag
	var style = document.createElement('style');

	if (style.styleSheet) {
		style.styleSheet.cssText = css;
	} else {
		style.appendChild(document.createTextNode(css));
	}

	document.getElementsByTagName('head')[0].appendChild(style);


	var body = document.getElementsByTagName("BODY")[0];

	var el = document.createElement("div");
	el.setAttribute("id", DesktopContent._verifyPopUpId);				
	var str = "<div id='" + DesktopContent._verifyPopUpId + "-text'>" + 
			prompt + "</div>" +
			"<input type='submit' value='Yes'> " + //onmouseup added below so func can be a function object (and not a string)
			"&nbsp;&nbsp;&nbsp;" + 
			"<input type='submit' onmouseup='DesktopContent.clearPopUpVerification();' value='Cancel'>";
	el.innerHTML = str;

	//onmouseup for "Yes" button
	el.getElementsByTagName('input')[0].onmouseup = 
			function(){DesktopContent.clearPopUpVerification(func);};

	Debug.log(prompt);
	DesktopContent._verifyPopUp = el;
	el.style.left = "-1000px"; //set off page so actual dimensions can be determined, and then div relocated
	body.appendChild(el);


	//determine position
	var w = el.offsetWidth; 
	var h = el.offsetHeight;
	var x = DesktopContent.getMouseX();
	var y = DesktopContent.getMouseY(); 

	Debug.log("X: " + x + 
			" Y: " + y + 
			" W: " + w + 
			" H: " + h);		
	while(x+w > DesktopContent.getWindowWidth())
		x -= w;		
	while(y+h > DesktopContent.getWindowHeight())
		y -= h;

	if(x <= 0) x = 10;
	if(y <= 0) y = 10;

	Debug.log("X: " + x + 
			" Y: " + y + 
			" W: " + w + 
			" H: " + h);		
	//var 
	el.style.left = x + "px";
	el.style.top = y + "px";
}
//=====================================================================================
//clearPopUpVerification ~~
//	call func after clearing, if exists
DesktopContent.clearPopUpVerification = function(func) {
	//remove pop up if already exist
	if(DesktopContent._verifyPopUp) DesktopContent._verifyPopUp.parentNode.removeChild(DesktopContent._verifyPopUp);
	DesktopContent._verifyPopUp = 0;
	if(func) func();
}

//=====================================================================================
//http://stackoverflow.com/questions/11068240/what-is-the-most-efficient-way-to-parse-a-css-color-in-javascript
// except the solution is broken.. unless you add element to page
DesktopContent.parseColor = function(colorStr) { 
    //used to ignore the alpha in the color when returning to user
	
	//in general need to create an element.. but since all the color strings are rgb or rgba from settings, can simplify
//	var div = document.createElement('div'), m;
//    div.style.color = colorStr;
//    div.style.display = "none";
//    document.body.appendChild(div);
//    m = getComputedStyle(div).color.split("(")[1].split(")")[0].split(",");
//	  document.body.removeChild(div);
	var m = colorStr.split("(")[1].split(")")[0].split(",");
    if( m) return "rgb("+m[0]+","+m[1]+","+m[2]+")";    
    else throw new Error("Color "+colorStr+" could not be parsed.");
}


DesktopContent.getWindowWidth = function() { return window.innerWidth; }
DesktopContent.getWindowHeight = function() { return window.innerHeight; }
DesktopContent.getMouseX = function() { return DesktopContent._windowMouseX | 0; } //force to int
DesktopContent.getMouseY = function() { return DesktopContent._windowMouseY | 0; } //force to int
DesktopContent.getDefaultWindowColor = function() {
	//return the alpha mix of window color and desktop color
    wrgba = DesktopContent._windowColorPostbox.innerHTML.split("(")[1].split(")")[0].split(",");
    drgb = DesktopContent._desktopColor.split("(")[1].split(")")[0].split(",");
    for(var i in drgb)
    	drgb[i] = (drgb[i]*(1-wrgba[3]) + wrgba[i]*wrgba[3])|0; //floor of blend
    return "rgb("+drgb[0]+","+drgb[1]+","+drgb[2]+")"; 
}
DesktopContent.getDefaultDashboardColor = function() { return DesktopContent.parseColor(DesktopContent._dashboardColorPostbox.innerHTML); }
DesktopContent.getDefaultDesktopColor = function() { return DesktopContent._desktopColor;} 
DesktopContent.getUsername = function() { 
	var dispName = DesktopContent._theWindow.parent.document.getElementById("DesktopDashboard-user-displayName").innerHTML
	return dispName.substr(dispName.indexOf(",")+2);	
}
