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
//  ... or to get element by tag name 
//		var els = req.responseXML.getElementsByTagName(name);
//		for(var i=0;i<els.length;++i)
//			Debug.log(els[i].getAttribute("value"));
//
//=====================================================================================

var DesktopContent = DesktopContent || {}; //define Desktop namespace

if (typeof Debug == 'undefined') 
	alert('ERROR: Debug is undefined! Must include Debug.js before DesktopWindowContentCode.js');
if (typeof Globals == 'undefined') 
	alert('ERROR: Globals is undefined! Must include Globals.js before DesktopWindowContentCode.js');

DesktopContent._isFocused = false;
DesktopContent._theWindow = 0;
DesktopContent._myDesktopFrame = 0;
DesktopContent._zMailbox = 0;
DesktopContent._mouseOverXmailbox = 0;
DesktopContent._mouseOverYmailbox = 0;

DesktopContent._serverUrnLid = 0;
DesktopContent._localUrnLid = 0;

DesktopContent._cookieCodeMailbox = 0;
DesktopContent._updateTimeMailbox = 0;
DesktopContent._needToLoginMailbox = 0;

DesktopContent.lastCookieCode = 0;
DesktopContent.lastCookieTime = 0;
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

	//add window frame position(absolute) + iframe position within window + mouse position within iframe
	DesktopContent._mouseOverXmailbox.innerHTML = parseInt(DesktopContent._myDesktopFrame.parentNode.parentNode.offsetLeft) +
			parseInt(DesktopContent._myDesktopFrame.offsetLeft) + parseInt(mouseEvent.clientX);
	DesktopContent._mouseOverYmailbox.innerHTML = parseInt(DesktopContent._myDesktopFrame.parentNode.parentNode.offsetTop) + 
			parseInt(DesktopContent._myDesktopFrame.offsetTop) + parseInt(mouseEvent.clientY);
}

DesktopContent.init(); //initialize handlers


//=====================================================================================
// XML request helpers
//=====================================================================================

//used by DesktopContent.XMLHttpRequest to reject multiple calls to same handler 
//(this is a result of bad error handling in the desktop window page.. so this is meant to inform the developer to fix the issue)
DesktopContent._arrayOfFailedHandlers = new Array();

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
DesktopContent.XMLHttpRequest = function(requestURL, data, returnHandler, reqIndex, progressHandler, sequence) {

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

				DesktopContent.lastCookieTime = parseInt((new Date()).getTime()); //in ms

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
					//handle cookie code mailbox
					DesktopContent.lastCookieCode = DesktopContent.getXMLValue(req,'CookieCode');
					if (typeof DesktopContent.lastCookieCode == 'undefined') 
					{ //clear req, server failed
						errStr = "Request Failed - Missing Cookie in Response.";

						if(DesktopContent._needToLoginMailbox) //if login mailbox is valid, force login
							DesktopContent._needToLoginMailbox.innerHTML = "1"; //force to login screen on server failure                        

					}
					else 
					{ //check if should update cc mailbox

						//check twice to handle race conditions with other content code
						if(parseInt(DesktopContent._updateTimeMailbox.innerHTML) < DesktopContent.lastCookieTime) //then current code is newer
						{
							DesktopContent._updateTimeMailbox.innerHTML = DesktopContent.lastCookieTime;
							DesktopContent._cookieCodeMailbox.innerHTML = DesktopContent.lastCookieCode;

							setTimeout(DesktopContent.checkCookieCodeRace, Math.random()*1000|0+500); //random wait (500-1500ms) before checking if race conditions occured
						}
					}
				}
			}
			else //bad address response
			{
				errStr = "Request Failed - Bad Address:\n" + requestURL;

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
				alert(errStr);
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
	if(parseInt(DesktopContent._updateTimeMailbox.innerHTML) < DesktopContent.lastCookieTime) //then current code is newer
	{
		Debug.log("Cookie race occured!");

		DesktopContent._updateTimeMailbox.innerHTML = DesktopContent.lastCookieTime;
		DesktopContent._cookieCodeMailbox.innerHTML = DesktopContent.lastCookieCode;
	}
}

//returns xml entry value for an attribute
DesktopContent.getXMLAttributeValue = function(req, name, attribute) {
	var els;
	if(req && req.responseXML && (els = req.responseXML.getElementsByTagName(name)).length > 0)
		return els[0].getAttribute(attribute);
	else
		return undefined;
}

//returns xml entry value for attribue 'value'
DesktopContent.getXMLValue = function(req, name) {
	return DesktopContent.getXMLAttributeValue(req,name,"value");
}

