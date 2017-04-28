//=====================================================================================
//
//	Created Dec, 2012
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	Debug.js
//
//	Since different browser have different console print statements, all ots code
// 		should use Debug.log(str,num[optional, default=0]). Where str is the string to print to console and
// 		num is the priority number 0 being highest.
//
//	Debug.log() will print to console if Debug.mode = 1 and if num < Debug.level
//
//	Note: An error pop up box will occur so that users see Debug.HIGH_PRIORITY log messages.
//	Note: A warning pop up box will occur so that users see Debug.WARN_PRIORITY log messages.
//	Note: An info pop up box will occur so that users see Debug.INFO_PRIORITY log messages.
//
//=====================================================================================

var Debug = Debug || {}; //define Debug namespace

Debug.mode = 1; 		//0 - debug off, 1 - debug on
Debug.simple = 0; 		//0 - use priority (more detail in printout), 1 - simple, no priority
Debug.level = 100;		//priority level, (100 should be all, 0 only high priority)
							//all logs with lower priority level are printed

Debug.lastLog = "";
Debug.lastLogger = "";

Debug.prependMessage = ""; //use to have a message always show up before log messages

//setup default priorities
Debug.HIGH_PRIORITY = 0;
Debug.WARN_PRIORITY = 1;
Debug.INFO_PRIORITY = 2;
Debug.TIP_PRIORITY = 3;
Debug.MED_PRIORITY = 50;
Debug.LOW_PRIORITY = 100;


//determine if chrome or firefox or other
//	0:other, 1:chrome, 2:firefox
Debug.BROWSER_TYPE = 0;
{
	var tmp = (new Error).stack; 
	if(tmp[0] == 'E')
		Debug.BROWSER_TYPE = 1;
	else if(tmp[0] == '@')
		Debug.BROWSER_TYPE = 2;
}


if (Debug.mode) //IF DEBUG MODE IS ON!
{
	if (Debug.simple)
	{
		//If want default console.log use this:
		Debug.log = console.log.bind(window.console);
	}
	else
	{
		//For fancy priority management use this:
		Debug.log = function(str,num) { 		
				//make num optional and default to lowest priority
				if(num === undefined) num = Debug.LOW_PRIORITY;
				
				if(Debug.level < 0) Debug.level = 0; //check for crazies, 0 is min level
				if(Debug.mode && num <= Debug.level)
				{				
					str = Debug.prependMessage + str; //add prepend message
					
					var type = num < 4?
							(num==0?"High":(num==1?"Warn":(num==2?"Info":"Tip")))
							:(num<99?"Med":"Low");
					
					if(Debug.BROWSER_TYPE == 1) //chrome
					{
						Debug.lastLogger = (new Error).stack.split("\n")[2];						
						Debug.lastLog = Debug.lastLogger.slice(0,Debug.lastLogger.indexOf(' ('));
						Debug.lastLogger = Debug.lastLogger.slice(Debug.lastLog.length+2,
								Debug.lastLogger.length-1);
					}
					else if(Debug.BROWSER_TYPE == 2) //firefox
					{
						Debug.lastLogger = (new Error).stack.split("\n")[1];						
						Debug.lastLog = Debug.lastLogger.slice(0,Debug.lastLogger.indexOf('@'));
						Debug.lastLogger = Debug.lastLogger.slice(Debug.lastLog.length+1,
								Debug.lastLogger.length);
					}
					
					console.log("%c" + type + "-Priority" +  
							 ":\t " + Debug.lastLog + ":\n" +
							 Debug.lastLogger + "::\t" + str,							 
							 num == 0?"color:#F30;"	//chrome/firefox allow css styling
									 :(num < 99?"color:#092":"color:#333")); 
					Debug.lastLog = str;
					
					if(num < 4) //show all high priorities as popup!
						Debug.errorPop(str,num);
				}
			}
	}
}
else	//IF DEBUG MODE IS OFF!
{	//do nothing with log functions
	console.log = function(){}
	Debug.log = function(){}
}

// living and breathing examples:
Debug.log("Debug mode is on at level: " + Debug.level);
Debug.log("This is an example for posterity that is not printed due to debug priority.",Debug.level+1);



//=====================================================================================
//=====================================================================================
//Error pop up helpers

Debug._errBox = 0;
Debug._errBoxId = "Debug-error-box";

//=====================================================================================
Debug.errorPopConditionString = function(str) {
	return str.replace(/\n/g , "<br>").replace(/\t/g,"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
}

//=====================================================================================
//Show the error string err in the error popup on the window
// create error div if not yet created
Debug.errorPop = function(err,severity) {
				
	var errBoxAlpha = "1.0";
	
	//check if Debug._errBox has been set
	if(!Debug._errBox)
	{	
		//check if there is already an error box with same id and share
		var el = document.getElementById(Debug._errBoxId);
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
			el.setAttribute("id", Debug._errBoxId);
			el.style.display = "none";
			var str = "<a class='" + 
				Debug._errBoxId + 
				"-header' onclick='javascript:Debug.closeErrorPop();event.stopPropagation();' onmouseup='event.stopPropagation();'>Close Errors</a>";
			str = str + "<br>" + 
				"<div style='color:white;font-size:16px;'>Note: Newest messages are at the top.</div><br>" +
				"<div id='" + 
				Debug._errBoxId +
				"-err'></div>" + 
				"<br>" + str;
			el.innerHTML = str;
			body.appendChild(el); //add element to body of page
			
			
			//add style for error to page HEAD tag			
			var css = "";
			
			//error close link style
			css += "#" + Debug._errBoxId + " a" +
					"{color: white; text-decoration: none; font-weight: 800;" +
					"font-size: 18px; font-family: 'Comfortaa', arial;" +
					"}\n\n";
			css += "#" + Debug._errBoxId + " a:hover" +
					"{text-decoration: underline;" +
					"cursor:pointer;" +
					"}\n\n";
			
			//error box style
			css += "#" + Debug._errBoxId +
					"{" +
					"position: absolute; display: none; border: 2px solid gray;" +
					"background-color: rgba(153,0,51, " + errBoxAlpha + "); overflow-y: scroll;" +
					"overflow-x: auto;	padding: 5px; -moz-border-radius: 2px;" +
					"-webkit-border-radius: 2px;	border-radius: 2px;" +
					"font-size: 18px; z-index: 2147483647;" + //max 32 bit number z-index
					"font-family: 'Comfortaa', arial; text-align: center;" +
					"left: 8px; top: 8px; margin-right: 8px; height:400px; " +
					"}\n\n";			

			//error box err text style
			css += "#" + Debug._errBoxId + "-err" +
					"{" +					
					"color: rgb(255,200,100); font-size: 18px;" +
					"font-family: 'Comfortaa', arial;" +
					"left: 8px, top: 8px; margin-right: 8px;" +
					"text-align: left;" +
					"}\n\n";
			
			css += "#" + Debug._errBoxId + "-err i" +
					",#" + Debug._errBoxId + "-err b" + 
					",#" + Debug._errBoxId + "-err u" + 
					",#" + Debug._errBoxId + "-err div" + 
					"{" +					
					"color: rgb(255,200,100); font-size: 18px;" +
					"font-family: 'Comfortaa', arial;" +
					"left: 8px, top: 8px; margin-right: 8px;" +
					"text-align: left;" +
					"}\n\n";

			//add style element to HEAD tag
			var style = document.createElement('style');

			if (style.styleSheet) {
			    style.styleSheet.cssText = css;
			} else {
			    style.appendChild(document.createTextNode(css));
			}

			document.getElementsByTagName('head')[0].appendChild(style);
		}
		Debug._errBox = el;	
	}	
	
	//have error popup element now, so fill it with new error
	
	var el = document.getElementById(Debug._errBoxId + "-err");
	var str = el.innerHTML; //keep currently displayed errors				
	var d = new Date();
	var wasAlreadyContent = false;
	
	//add new err to top of errors
	if(str.length)
		wasAlreadyContent = true;
	
	var tstr = d.toLocaleTimeString();
	tstr = tstr.substring(0,tstr.lastIndexOf(' ')) + //convert AM/PM to am/pm with no space
			(tstr[tstr.length-2]=='A'?"am":"pm");
	
	if(severity == Debug.TIP_PRIORITY) //put oldest at top so it reads like a document
		str = str + 
			(wasAlreadyContent?"<br>...<br>":"") +
			"<label style='color:white;font-size:16px;'>" + 
			d.toLocaleDateString() +
			" " + tstr + ":</label><br>" +
			Debug.errorPopConditionString(err);	
	else //normally put newest at top since likely highest priority
		str = "<label style='color:white;font-size:16px;'>" + 
			d.toLocaleDateString() +
			" " + tstr + ":</label><br>" +
			Debug.errorPopConditionString(err) + 
			(wasAlreadyContent?"<br>...<br>":"") +
			str;

	el.innerHTML = str;

	//show the error box whereever the current scroll is
	{
		var offX = document.documentElement.scrollLeft || document.body.scrollLeft || 0;
		var offY = document.documentElement.scrollTop || document.body.scrollTop || 0;
		var w;
		
		//and, set width properly so error box is scrollable for long winded errors
		if(typeof DesktopContent != 'undefined') //define width using DesktopContent
			w = (DesktopContent.getWindowWidth()-16-14); //scroll width is 14px		
		else if(typeof Desktop != 'undefined' && Desktop.desktop) //define width using Desktop
			w = (Desktop.desktop.getDesktopWidth()-16-14); //scroll width is 14px
		
		if(w > 900) //clip to 850 and center (for looks)
		{
			offX += (w-850)/2;
			w = 850;
		}			
		
		Debug._errBox.style.width = (w) + "px";
		Debug._errBox.style.left = (offX + 8) + "px";
		Debug._errBox.style.top = (offY + 8) + "px";
	}
	
	Debug._errBox.style.display = "block";
	
	//change color based on info
	
	var els = document.getElementsByClassName(Debug._errBoxId + "-header");
	el = els[0];
	switch(severity)
	{
	case Debug.TIP_PRIORITY:
		//don't change color or header for info, if there are still errors displayed
	if(wasAlreadyContent && 
			(el.innerHTML == "Close Errors" ||
					el.innerHTML == "Close Warnings" ||
					el.innerHTML == "Close Info"))
			return;
		el.innerHTML = "Close Tooltip";		
		Debug._errBox.style.backgroundColor = "rgba(0, 79, 160, " + errBoxAlpha + ")";	
		break;
	case Debug.INFO_PRIORITY:
		//don't change color or header for info, if there are still errors displayed
		if(wasAlreadyContent && 
				(el.innerHTML == "Close Errors" ||
						el.innerHTML == "Close Warnings"))
			return;
		el.innerHTML = "Close Info";		
		Debug._errBox.style.backgroundColor = "rgba(0,153,51, " + errBoxAlpha + ")";
		break;
	case Debug.WARN_PRIORITY:
		//don't change color or header for info, if there are still errors displayed
		if(wasAlreadyContent && 
				el.innerHTML == "Close Errors")
			return;
		el.innerHTML = "Close Warnings";		
		Debug._errBox.style.backgroundColor = "rgba(160, 79, 0, " + errBoxAlpha + ")";	
		break;
	default: //Debug.HIGH_PRIORITY
		el.innerHTML = "Close Errors";
		Debug._errBox.style.backgroundColor = "rgba(153,0,51, " + errBoxAlpha + ")";
	}
	els[1].innerHTML = el.innerHTML;	
}


//=====================================================================================
//Close the error popup on the window
Debug.closeErrorPop = function() {
	document.getElementById(Debug._errBoxId).style.display = "none";
	document.getElementById(Debug._errBoxId + "-err").innerHTML = ""; //clear string
}



