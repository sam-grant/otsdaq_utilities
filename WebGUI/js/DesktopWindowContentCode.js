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
//		-DesktopContent.XMLHttpRequest(requestURL, data, returnHandler <optional>, reqParam <optional>, progressHandler <optional>, callHandlerOnErr <optional>, showLoadingOverlay <optional>, targetSupervisor <optional>)
//			... to make server request, returnHandler is called with response in req and reqParam if user defined
//			... here is a returnHandler declaration example:
//		
//					function returnHandler(req,reqParam,errStr)
//					{
//						if(errStr != "") return; //error occured!
//
//						var err = DesktopContent.getXMLValue(req,"Error"); //example application level error
//						if(err) 
//						{
//							Debug.log(err,Debug.HIGH_PRIORITY);	//log error and create pop-up error box
//							return;
//						}
//
//						if(reqParam = 0)
//						{ //... do something }
//						else if(reqParam = 1)
//						{ //... do something else}
//						else
//						{ //... do something else}
//						
//						//..do more things
//		
//					}
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
//		DesktopContent.popUpVerification(prompt, func [optional], val [optional], bgColor [optional], textColor [optional], borderColor [optional])
//		DesktopContent.tooltip(uid,tip)
//		DesktopContent.getWindowWidth()
//		DesktopContent.getWindowHeight()
//		DesktopContent.getWindowScrollLeft()
//		DesktopContent.getWindowScrollTop()
//		DesktopContent.getMouseX()
//		DesktopContent.getMouseY()
//		DesktopContent.getDefaultWindowColor() 	 // returns "rgb(#,#,#)"
//  	DesktopContent.getDefaultDashboardColor()// returns "rgb(#,#,#)"
//		DesktopContent.getDefaultDesktopColor()  // returns "rgb(#,#,#)"
//		DesktopContent.getUsername()
//		DesktopContent.openNewWindow(name,subname,windowPath,unique,completeHandler)
//		DesktopContent.openNewBrowserTab(name,subname,windowPath,unique,completeHandler)
//
//=====================================================================================

var DesktopContent = DesktopContent || {}; //define Desktop namespace

if (typeof Debug == 'undefined') 
	alert('ERROR: Debug is undefined! Must include Debug.js before DesktopWindowContentCode.js');
if (typeof Globals == 'undefined') 
	alert('ERROR: Globals is undefined! Must include Globals.js before DesktopWindowContentCode.js');


//"public" function list: 
//	DesktopContent.XMLHttpRequest(requestURL, data, returnHandler, reqParam, progressHandler, callHandlerOnErr, showLoadingOverlay)
//	DesktopContent.getXMLValue(req, name)
//	DesktopContent.getXMLNode(req, name)
//	DesktopContent.getXMLDataNode(req)
//	DesktopContent.getXMLAttributeValue(req, name, attribute)
//	DesktopContent.popUpVerification(prompt, func, val, bgColor, textColor, borderColor)
//	DesktopContent.tooltip(uid,tip)
//	DesktopContent.getWindowWidth()
//	DesktopContent.getWindowHeight()
//	DesktopContent.getWindowScrollLeft()
//	DesktopContent.getWindowScrollTop()
//	DesktopContent.getMouseX()
//	DesktopContent.getMouseY()
//	DesktopContent.getDefaultWindowColor()
//  DesktopContent.getDefaultDashboardColor()
//	DesktopContent.getDefaultDesktopColor()
//	DesktopContent.getUsername()
//	DesktopContent.openNewWindow(name,subname,windowPath,unique,completeHandler)
//	DesktopContent.mouseMoveSubscriber(newHandler) 
//	DesktopContent.openNewBrowserTab(name,subname,windowPath,unique,completeHandler)
//	DesktopContent.getParameter(index, name)
//	DesktopContent.getDesktopParameter(index, name)
//	DesktopContent.getDesktopWindowTitle()

//"private" function list:
//	DesktopContent.init()
//	DesktopContent.handleFocus(e)
//	DesktopContent.handleBlur(e)
//	DesktopContent.handleScroll(e)
//	DesktopContent.mouseMove(mouseEvent)
//	DesktopContent.checkCookieCodeRace()
//	DesktopContent.clearPopUpVerification(func)
//	DesktopContent.parseColor(colorStr)
//	DesktopContent.tooltipNeverShow(srcFunc,srcFile,srcId)

DesktopContent._isFocused = false;
DesktopContent._theWindow = 0;
DesktopContent._myDesktopFrame = 0;
DesktopContent._zMailbox = 0;
DesktopContent._mouseOverXmailbox = 0;
DesktopContent._mouseOverYmailbox = 0;
DesktopContent._windowMouseX = -1;
DesktopContent._windowMouseY = -1;

DesktopContent._serverOrigin = "";
DesktopContent._localOrigin = "";
DesktopContent._serverUrnLid = 0;
DesktopContent._localUrnLid = 0;

DesktopContent._cookieCodeMailbox = 0;
DesktopContent._updateTimeMailbox = 0;
DesktopContent._needToLoginMailbox = 0;
DesktopContent._openWindowMailbox = 0;

DesktopContent._lastCookieCode = 0;
DesktopContent._lastCookieTime = 0;


DesktopContent._verifyPopUp = 0;
DesktopContent._verifyPopUpId = "DesktopContent-verifyPopUp";

DesktopContent._windowColorPostbox = 0;
DesktopContent._dashboardColorPostbox = 0;
DesktopContent._desktopColor = 0;

DesktopContent._sequence = 0;

DesktopContent._mouseMoveSubscribers = [];


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
	DesktopContent._openWindowMailbox	  = DesktopContent._theWindow.parent.document.getElementById("DesktopContent-openWindowMailbox");

	DesktopContent._windowColorPostbox	  = DesktopContent._theWindow.parent.document.getElementById("DesktopContent-windowColorPostbox");
	DesktopContent._dashboardColorPostbox = DesktopContent._theWindow.parent.document.getElementById("DesktopContent-dashboardColorPostbox");
	
	if(DesktopContent._theWindow.parent.document.body)
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
	DesktopContent._serverOrigin = DesktopContent._theWindow.parent.window.location.origin;
	Debug.log("Supervisor Application Origin = " + DesktopContent._serverOrigin);
	
	DesktopContent._localUrnLid = DesktopContent.getParameter(0,"urn");
	if(typeof DesktopContent._localUrnLid == 'undefined')
		DesktopContent._localUrnLid = 0;
	Debug.log("Local Application URN-LID #" + DesktopContent._localUrnLid);
	DesktopContent._localOrigin = window.location.origin;
	Debug.log("Local Application Origin = " + DesktopContent._localOrigin);

	//get Wizard sequence (if in Wizard mode)
	DesktopContent._sequence = DesktopContent.getDesktopParameter(0,"code");
	if(!DesktopContent._sequence || DesktopContent._sequence == "")
		DesktopContent._sequence = 0; //normal desktop mode
	else
		Debug.log("In Wizard Mode with Sequence=" + DesktopContent._sequence);
}

//DesktopContent.getParameter ~
//	returns the value of the url GET parameter specified by index
//	if using name, then (mostly) ignore index
DesktopContent.getParameter = function(index,name) {	
	// Debug.log(window.location)
	var params = (window.location.search.substr(1)).split('&');
	var spliti, vs;
	//if name given, make it the priority
	if(name)
	{
		for(index=0;index<params.length;++index)
		{
			spliti = params[index].indexOf('=');
			if(spliti < 0) continue; //poorly formed parameter?	
			vs = [params[index].substr(0,spliti),params[index].substr(spliti+1)];
			if(vs[0] == name)
				return decodeURIComponent(vs[1]);
		}
		return; //return undefined .. name not found
	}

	//using index 
	if(index >= params.length) return; //return undefined
	
	spliti = params[index].indexOf('=');
	if(spliti < 0) return; //return undefined	
	vs = [params[index].substr(0,spliti),params[index].substr(spliti+1)];
	return decodeURIComponent(vs[1]); //return value
}

//DesktopContent.getDesktopParameter ~
//	returns the value of the url GET parameter specified by index of the Desktop url
//	if using name, then (mostly) ignore index
DesktopContent.getDesktopParameter = function(index, name) {	
	// Debug.log(window.location)
	var params = (DesktopContent._theWindow.parent.parent.window.location.search.substr(1)).split('&');
	if(index >= params.length) return; //return undefined
	var spliti, vs;
	//if name given, make it the priority
	if(name)
	{
		for(var index=0;index<params.length;++index)
		{
			spliti = params[index].indexOf('=');
			if(spliti < 0) continue; //poorly formed parameter?	
			vs = [params[index].substr(0,spliti),params[index].substr(spliti+1)];
			if(vs[0] == name)
				return decodeURIComponent(vs[1]);
		}
		return; //return undefined .. name not found
	}

	spliti = params[index].indexOf('=');
	if(spliti < 0) return; //return undefined	
	vs = [params[index].substr(0,spliti),params[index].substr(spliti+1)];
	return decodeURIComponent(vs[1]); //return value
}

//DesktopContent.handleFocus ~
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
	//call each subscriber
	for(var i=0; i<DesktopContent._mouseMoveSubscribers.length; ++i)
		DesktopContent._mouseMoveSubscribers[i](mouseEvent); 
	
	if(!DesktopContent._myDesktopFrame) return; //only happens if not part of desktop

	DesktopContent._windowMouseX = parseInt(mouseEvent.clientX);
	DesktopContent._windowMouseY = parseInt(mouseEvent.clientY);

	//add window frame position(absolute) + iframe position within window + mouse position within iframe
	DesktopContent._mouseOverXmailbox.innerHTML = parseInt(DesktopContent._myDesktopFrame.parentNode.parentNode.offsetLeft) +
			parseInt(DesktopContent._myDesktopFrame.offsetLeft) + DesktopContent._windowMouseX;
	DesktopContent._mouseOverYmailbox.innerHTML = parseInt(DesktopContent._myDesktopFrame.parentNode.parentNode.offsetTop) + 
			parseInt(DesktopContent._myDesktopFrame.offsetTop) + DesktopContent._windowMouseY;	
}

DesktopContent.mouseMoveSubscriber = function(newHandler) {
	DesktopContent._mouseMoveSubscribers.push(newHandler);
}
	

DesktopContent.init(); //initialize handlers


//=====================================================================================
// XML request helpers
//=====================================================================================

//used by DesktopContent.XMLHttpRequest to reject multiple calls to same handler 
//(this is a result of bad error handling in the desktop window page.. so this is meant to inform the developer to fix the issue)
DesktopContent._arrayOfFailedHandlers = new Array();

//=====================================================================================
//=====================================================================================
//Loading pop up helpers
DesktopContent._loadBox = 0;
DesktopContent._loadBoxId = "DesktopContent-load-box";
DesktopContent._loadBoxTimer = 0;
DesktopContent._loadBoxRequestStack = 0; //load box is not removed until back to 0

//=====================================================================================
DesktopContent.showLoading = function()	{
	
	Debug.log("DesktopContent.showLoading " + DesktopContent._loadBoxRequestStack);
	
	if(DesktopContent._loadBoxRequestStack++) //box should still be open, add to stack
		return;
	
	//check if DesktopContent._loadBox has been set
	if(!DesktopContent._loadBox)
	{	
		//check if there is already an error box with same id and share
		var el = document.getElementById(DesktopContent._loadBoxId);
		if(!el) //element doesn't already exist, so we need to create the element
		{
			var body = document.getElementsByTagName("BODY")[0];
			if(!body) //maybe page not loaded yet.. so wait to report
			{
				//try again in 1 second
				window.setTimeout(function() { Debug.errorPop(err,severity)}, 1000);
				return;
			}

			//create the element
			el = document.createElement("div");			
			el.setAttribute("id", DesktopContent._loadBoxId);
			el.style.display = "none";
			var str = "";
			
			str += "<table height='100%' width='100%'><td id='" + 
					DesktopContent._loadBoxId + "-td'>Loading...</td></table>";
			el.innerHTML = str;
			body.appendChild(el); //add element to body of page


			//add style for loading to page HEAD tag			
			var css = "";

			
			//load box style
			css += "#" + DesktopContent._loadBoxId +
					"{" +
					"position: absolute; display: none; border: 2px solid gray;" +
					"background-color: rgba(0,0,0,0.8); overflow-y: auto;" +
					"overflow-x: auto;	padding: 5px; -moz-border-radius: 2px;" +
					"-webkit-border-radius: 2px;	border-radius: 2px;" +
					"font-size: 18px; z-index: 2147483647;" + //max 32 bit number z-index
					"color: white; " +
					"font-family: 'Comfortaa', arial; text-align: left;" +
					"left: 8px; top: 8px; margin-right: 8px; height:400px; " +
					"}\n\n";			

			//load box text style
			//			css += "#" + DesktopContent._loadBoxId + "-td" +
			//					"{" +					
			//					"color: white; font-size: 18px;" +
			//					"font-family: 'Comfortaa', arial;" +
			//					"text-align: center;" +
			//					"}\n\n";

			//add style element to HEAD tag
			var style = document.createElement('style');

			if (style.styleSheet) {
				style.styleSheet.cssText = css;
			} else {
				style.appendChild(document.createTextNode(css));
			}

			document.getElementsByTagName('head')[0].appendChild(style);
		}
		DesktopContent._loadBox = el;	
	}	
	
	//have load popup element now, so display it at center of page
	
	var W = 100;
	var H = 60;
	
	var WW,WH; //window width and height
	//get width and height properly 
	if(typeof DesktopContent != 'undefined') //define width using DesktopContent
	{
		WW = DesktopContent.getWindowWidth();
		WH = DesktopContent.getWindowHeight();
	}
	else if(typeof Desktop != 'undefined' && Desktop.desktop) //define width using Desktop
	{
		WW = DesktopContent.getDesktopWidth();
		WH = DesktopContent.getDesktopHeight();
	}
	
	var X = DesktopContent.getWindowScrollLeft() + (WW - W - 4)/2; //for 2px borders
	var Y = DesktopContent.getWindowScrollTop() + (WH - H -4)/2; //for 2px borders
	
	//show the load box whereever the current scroll is	
	DesktopContent._loadBox.style.left = (X) + "px";	
	DesktopContent._loadBox.style.top = (Y) + "px";
	DesktopContent._loadBox.style.width = (W) + "px";
	DesktopContent._loadBox.style.height = (H) + "px";

	DesktopContent._loadBox.style.display = "block";
	
	//===================
	//setup Loading.. animation
	var loadBoxStr = "..";
	var el = document.getElementById(DesktopContent._loadBoxId + "-td");
	var loadBoxAnimationFunction = function() {
		if(loadBoxStr.length > 3) loadBoxStr = "";
		else
			loadBoxStr += ".";
		el.innerHTML = "Loading" + loadBoxStr;
	}; 
	
	window.clearInterval(DesktopContent._loadBoxTimer);
	DesktopContent._loadBoxTimer = window.setInterval(loadBoxAnimationFunction, 300);
}
//=====================================================================================
DesktopContent.hideLoading = function()	{
	
	if(--DesktopContent._loadBoxRequestStack) //subtract from stack, but dont hide if stack remains
		return;
	
	window.clearInterval(DesktopContent._loadBoxTimer); //kill loading animation
	Debug.log("DesktopContent.hideLoading");
	DesktopContent._loadBox.style.display = "none";
}
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
// reqParam is used to give the returnHandler an index to route responses to.
// progressHandler can be given to receive progress updates (e.g. for file uploads)
// callHandlerOnErr can be set to true to have handler called with errStr parameter
//	otherwise, handler will not be called on error.
//
DesktopContent.XMLHttpRequest = function(requestURL, data, returnHandler, 
		reqParam, progressHandler, callHandlerOnErr, showLoadingOverlay, targetSupervisor) {

	// Sequence is used as an alternative approach to cookieCode (e.g. ots Config Wizard).
	var sequence = DesktopContent._sequence;
	var errStr = "";
	var req;
	
	if(showLoadingOverlay)
		DesktopContent.showLoading();

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
		if(returnHandler && !found && callHandlerOnErr) returnHandler(req, reqParam, errStr); 
		return;
	}

	req = new XMLHttpRequest();

	if(progressHandler) req.upload.addEventListener("progress", progressHandler, false); //add progress listener if defined

	//set timeout to detect infinite loops or long waits
	var timeoutTimer;	
	var timeoutFunction = function() 
					{
						Debug.log("It has been 60 seconds.. still waiting for a response. " +
							"Is there an infinite loop occuring at the server? " +
							"Or is this just a really long request..",
							Debug.HIGH_PRIORITY);
						timeoutTimer = window.setTimeout(timeoutFunction, 60000); 
					}
	timeoutTimer = window.setTimeout(timeoutFunction, 60000);
	
	//setup response handler
	req.onreadystatechange = function() {
		//Debug.log(req.readyState + " " + req.status);
		if (req.readyState==4) 
		{  //when readyState=4 return complete, status=200 for success, status=400 for fail
			window.clearTimeout(timeoutTimer);
			
			if(showLoadingOverlay)
				DesktopContent.hideLoading();
			
			if(req.status==200)
			{				
				//Debug.log("Request Response Text " + req.responseText + " ---\nXML " + req.responseXML,Debug.LOW_PRIORITY);

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
				else if(req.responseText == Globals.REQ_LOCK_REQUIRED_RESPONSE) 
				{
					errStr = "Request failed because the request requires the user to lockout the system. Please take over the lock in the Settings area to proceed.";
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
				Debug.log("Status=0. Likely this means a window was closed, or the server crashed, in the middle of a request. " +
						"\n(It also could mean 'potential security risk' like a cross-domain request) ",Debug.HIGH_PRIORITY);
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

			//success, call return handler
			if(returnHandler && (errStr=="" || callHandlerOnErr)) 
				returnHandler(req, reqParam, errStr);
		}
	}

	if(!sequence)
	{        
		var cc = DesktopContent._cookieCodeMailbox?DesktopContent._cookieCodeMailbox.innerHTML:""; //get cookie code from mailbox if available
		data = "CookieCode="+cc+((data===undefined)?"":("&"+data));
	}
	else
	{   	
		data = "sequence="+sequence+"&"+((data===undefined)?"":("&"+data));
	}
	
	
	var urn = DesktopContent._localUrnLid;
	var origin = DesktopContent._localOrigin;
	
	if(!urn || targetSupervisor) //desktop supervisor, instead of local application
	{
		urn = DesktopContent._serverUrnLid;
		origin = DesktopContent._serverOrigin;
	}
	
	requestURL = origin+"/urn:xdaq-application:lid="+urn+"/"+requestURL;
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
	var el;
	if(el = DesktopContent.getXMLNode(req,name))
		return el.getAttribute(attribute);
	else if(name == "Error" && //make sure Error gives an error if the response is bad
			(!req || !req.responseXML)) 
		return "Unknown error occured " +
				"(XML response may have been illegal)!";
	else
		return undefined;
}

//=====================================================================================
//returns xml entry value for attribue 'value'
//	if !name assume req is xml node already
DesktopContent.getXMLValue = function(req, name) {
	if(!name)
		return req.getAttribute("value");	
	return DesktopContent.getXMLAttributeValue(req,name,"value");
}

//=====================================================================================
//returns xml entry node (first node with name)
//	Note: could be any depth (not just depth 1)
//	...uses getElementsByTagName(name)
DesktopContent.getXMLNode = function(req, name) {
	var els;
	if(req && req.responseXML) //to allow for arbitrary starting xml node
		req = req.responseXML;
	if(req)
	{
		//find DATA
		var i;
		els = req.getElementsByTagName(name);//req.responseXML.childNodes[0].childNodes;
		if(els.length)
			return els[0];
		//reverted to former way

		//for depth 1 sometime?
		//		for(var i=0;i<els.length;++i)
		//			if(els[i].nodeName == "DATA")
		//			{
		//				els = req.responseXML.childNodes[0].childNodes[i].childNodes;
		//				
		//				for(i=0;i<els.length;++i)
		//					if(els[i].nodeName == name)			
		//						return els[i];
		//				break;
		//			}			
	}

	return undefined;	
}

//=====================================================================================
//returns xml entry node (first node with name DATA)
DesktopContent.getXMLDataNode = function(req, name) {
	return DesktopContent.getXMLNode("DATA");	
}

//=====================================================================================
//tooltip ~~
//	use uid to determine if tip should still be displayed
//		- ask server about user/file/function/id combo
//		- if shouldShow:
//			then show 
//			add checkbox to never show again
DesktopContent.tooltip = function(id,tip) {
	
	var srcStackString = (new Error).stack.split('\n')[2];
	var srcFunc = srcStackString.trim().split(' ')[1];
	var srcFile = srcStackString.substr(srcStackString.lastIndexOf('/')+1);
	if(srcFile.indexOf('?') >= 0)
		srcFile = srcFile.substr(0,srcFile.indexOf('?'));
	if(srcFile.indexOf(':') >= 0)
		srcFile.substr(0,srcFile.indexOf(':'));
	
	DesktopContent.XMLHttpRequest(
			"TooltipRequest?RequestType=check" + 
			"&srcFunc=" + srcFunc +
			"&srcFile=" + srcFile +
			"&srcId=" + id
			,""
			, function(req) {

		var showTooltip = DesktopContent.getXMLValue(req,"ShowTooltip");
		Debug.log("showTooltip: " + showTooltip);

		if(showTooltip|0)
		{			
			tip = "<br><center><b style='color:inherit'>'" + id + "' Tooltip:</b></center><br>" + tip;
			tip += "<br><br>";
			tip += "<input type='checkbox' id='DesktopContent-tooltip-SetNeverShowCheckbox' " +
					"onclick='" + "var el = document.getElementById(\"DesktopContent-tooltip-SetNeverShowCheckbox\");" +					
					"DesktopContent.tooltipSetNeverShow(\"" + 
					srcFunc + "\",\"" +
					srcFile + "\",\"" +
					id + "\", el.checked);" + "'>";
			tip += "<a href='#' onclick='" +
					"var el = document.getElementById(\"DesktopContent-tooltip-SetNeverShowCheckbox\");" +
					"el.checked = !el.checked;" +
					"DesktopContent.tooltipSetNeverShow(\"" + 
					srcFunc + "\",\"" +
					srcFile + "\",\"" +
					id + "\", el.checked);" +
					"'>";
			tip += "Never show the above Tooltip again.";
			tip += "</a>";
			tip +="</input>";
					
			Debug.log(tip,Debug.TIP_PRIORITY);
		}
	},0,0,0,true,true); //show loading, and target supervisor
	
}

//=====================================================================================
//tooltipSetNeverShow ~~
//	set value of never show for target tip to 1/0 based on doNeverShow
DesktopContent.tooltipSetNeverShow = function(srcFunc,srcFile,id,doNeverShow) {
	Debug.log("doNeverShow = " + doNeverShow);
	DesktopContent.XMLHttpRequest(
			"TooltipRequest?RequestType=setNeverShow" + 
			"&srcFunc=" + srcFunc +
			"&srcFile=" + srcFile +
			"&srcId=" + id + 
			"&doNeverShow=" + (doNeverShow?1:0)
			,""
			,0,0,0,0,true,true); //show loading, and target supervisor
}
		
//=====================================================================================
//popUpVerification ~~
//	asks user if sure
//	replace REPLACE in prompt with value passed as val (e.g. pass document.getElementById("myElementId").value)
//	call func (if exists) when user selects "Yes" 
//
//	Can change background color and text color with strings bgColor and textColor (e.g. "rgb(255,0,0)" or "red")
//		Default is yellow bg with black text if nothing passed.
DesktopContent.popUpVerification = function(prompt, func, val, bgColor, textColor, borderColor) {		

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
	if(!borderColor) borderColor = "black";
	
	var css = "";
	//pop up div style
	//note: z-index of 2000 is above config gui treeview
	css += "#" + DesktopContent._verifyPopUpId + " " +
			"{position: absolute; z-index: 2000; border-radius: 5px; padding: 10px;" +			
			"background-color: " + bgColor + "; border: 2px solid " + borderColor + ";" +
			"color: " + textColor + ";text-align: center; overflow: auto;" +
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
			"<input type='submit' value='Yes' " +
			"onclick='event.stopPropagation();' " + 
			"> " + //onmouseup added below so func can be a function object (and not a string)
			"&nbsp;&nbsp;&nbsp;" + 
			"<input type='submit' " +
			"onmouseup='event.stopPropagation();" +
			"DesktopContent.clearPopUpVerification();' " +
			"onclick='event.stopPropagation();' " +
			"value='Cancel'>";
	el.innerHTML = str;

	//onmouseup for "Yes" button
	el.getElementsByTagName('input')[0].onmouseup = 
			function(event){event.stopPropagation(); DesktopContent.clearPopUpVerification(func);};

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


//=====================================================================================
//get window and mouse info ~~
DesktopContent.getWindowWidth = function() { return window.innerWidth; }
DesktopContent.getWindowHeight = function() { return window.innerHeight; }
DesktopContent.getWindowScrollLeft = function() { return document.documentElement.scrollLeft || document.body.scrollLeft || 0; }
DesktopContent.getWindowScrollTop = function() { return document.documentElement.scrollTop || document.body.scrollTop || 0; }
DesktopContent.getMouseX = function() { return DesktopContent._windowMouseX | 0; } //force to int
DesktopContent.getMouseY = function() { return DesktopContent._windowMouseY | 0; } //force to int
DesktopContent.getDefaultWindowColor = function() {
	//return the alpha mix of window color and desktop color
	if(!DesktopContent._windowColorPostbox || !DesktopContent._desktopColor)
	{
		//likely in wizard mode
		Debug.log("Color post boxes not setup! So giving default.",Debug.MED_PRIORITY);
		return "rgb(178,210,240)";
	}

	wrgba = DesktopContent._windowColorPostbox.innerHTML.split("(")[1].split(")")[0].split(",");
	drgb = DesktopContent._desktopColor.split("(")[1].split(")")[0].split(",");
	for(var i in drgb)
		drgb[i] = (drgb[i]*(1-wrgba[3]) + wrgba[i]*wrgba[3])|0; //floor of blend
	return "rgb("+drgb[0]+","+drgb[1]+","+drgb[2]+")"; 
}

//=====================================================================================
//get color scheme ~~
DesktopContent.getDefaultDashboardColor = function() { return DesktopContent.parseColor(DesktopContent._dashboardColorPostbox.innerHTML); }
DesktopContent.getDefaultDesktopColor = function() { 
	if(!DesktopContent._desktopColor)
	{
		//likely in wizard mode
		Debug.log("Color post boxes not setup! So giving default.",Debug.MED_PRIORITY);
		return "rgb(15,34,105)";
	}
	return DesktopContent._desktopColor;
} 

//=====================================================================================
//getUsername ~~
DesktopContent.getUsername = function() { 
	var dispName = DesktopContent._theWindow.parent.document.getElementById("DesktopDashboard-user-displayName").innerHTML
			return dispName.substr(dispName.indexOf(",")+2);	
}


//=====================================================================================
//openNewWindow ~~
//	first wait for mailbox to be clear
//	then take mailbox
DesktopContent.openNewWindow = function(name,subname,windowPath,unique,completeHandler) {	
	
	Debug.log("DesktopContent.openNewWindow= " + windowPath);
	Debug.log("name= " + name);
	Debug.log("subname= " + subname);
	Debug.log("unique= " + unique);
	
	//extract params from DesktopContent._openWindowMailbox
	//get parameters
	var paramsStr = DesktopContent._openWindowMailbox.innerHTML;
	
	if(paramsStr != "") //then wait
	{
		Debug.log("Window creation is busy, trying again soon!");
		setTimeout(function(){ DesktopContent.openNewWindow(windowPath); }, 100);
		return;
	}

	//free to attempt to open window!
	var str = "requestingWindowId=" + DesktopContent._myDesktopFrame.id.split('-')[1];
	str += "&windowName=" + name;
	str += "&windowSubname=" + subname;
	str += "&windowUnique=" + unique;
	str += "&windowPath=" + windowPath;
	DesktopContent._openWindowMailbox.innerHTML = str;

	Debug.log("Waiting for complete...");

	var timeoutHandler = function() { 
		Debug.log("Checking for complete...");
		//extract params from DesktopContent._openWindowMailbox
		//get parameters
		var paramsStr = DesktopContent._openWindowMailbox.innerHTML;
		var spliti = paramsStr.indexOf('&amp;');
		params = [paramsStr.substr(0,spliti),paramsStr.substr(spliti+5)];
		var varPair;
		var requestingWindowId = "", done = "";
		for(var i=0;i<params.length;++i)
		{
			spliti = params[i].indexOf('=');
			varPair = [params[i].substr(0,spliti),params[i].substr(spliti+1)];	    		
			if(varPair[0] 		== "requestingWindowId")
				requestingWindowId 	= varPair[1];
			else if(varPair[0] 	== "done")
				done 				= varPair[1];	 
		}

		if(paramsStr = "" || (requestingWindowId != "" && done != ""))
		{
			//assume done!
			Debug.log("requestingWindowId=" + requestingWindowId);
			Debug.log("done=" + done);
			if(requestingWindowId != DesktopContent._myDesktopFrame.id.split('-')[1])
				Debug.log("There was a mismatch in window id!",Debug.MED_PRIORITY);

			//clear mailbox 
			DesktopContent._openWindowMailbox.innerHTML = "";

			if(completeHandler)
				completeHandler(); //call parameter handler
			return;
		}
		else //try again
			setTimeout(timeoutHandler, 100); //try again

	};	//end setTimeout handler
	
	setTimeout(timeoutHandler,
			100); //end setTimeout

}

//=====================================================================================
//openNewBrowserTab ~~
//	first wait for mailbox to be clear
//	then take mailbox
DesktopContent.openNewBrowserTab = function(name,subname,windowPath,unique) {	
	
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

	var search = DesktopContent._theWindow.parent.parent.window.location.search;
	url = DesktopContent._theWindow.parent.parent.window.location.pathname;
		
	var str = "requestingWindowId=" + DesktopContent._myDesktopFrame.id.split('-')[1];
	str += "&windowName=" + name;
	str += "&windowSubname=" + subname;
	str += "&windowUnique=" + unique;
	str += "&windowPath=" + windowPath;
		
	//if there is no search, need to check lid=## is terminated with /
	// check from = that there is nothing but numbers
	
	if(search == "") 
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
	else
		url += search + "&" + str;
	
	Debug.log("DesktopContent.openNewBrowserTab= " + url);
	
	window.open(url,'_blank');	
}

//getDesktopWindowTitle ~~
//	returns the text in header of the current desktop window
DesktopContent.getDesktopWindowTitle = function() {
	return DesktopContent._theWindow.parent.document.getElementById(
			"DesktopWindowHeader-" + 
			DesktopContent._theWindow.name.split('-')[1]).innerHTML;
}







