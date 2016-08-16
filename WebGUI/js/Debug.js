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
//	Note: A pop up box will occur so that users see Debug.HIGH_PRIORITY log messages.
//
//=====================================================================================

var Debug = Debug || {}; //define Debug namespace

Debug.mode = 1; 		//0 - debug off, 1 - debug on
Debug.simple = 0; 		//0 - use priority (more detail in printout), 1 - simple, no priority
Debug.level = 100;		//priority level, (100 should be all, 0 only high priority)
							//all logs with lower priority level are printed

//setup default priorities
Debug.HIGH_PRIORITY = 0;
Debug.MED_PRIORITY = 50;
Debug.LOW_PRIORITY = 100;


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
					
					console.log("Priority-" + num + 
							 ":\t " + (new Error).stack.split("\n")[2] + ":\n\t" +
							str); 
					//console.log("" + str, "\n\n===   Debug Priority " + num + "   ===\n\t" + 
					//		(new Error).stack.split("\n")[2]); //gets calling source from call stack
					
					if(num == Debug.HIGH_PRIORITY) //show all high priorities as popup!
						Debug.errorPop(str);
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
//Show the error string err in the error popup on the window
// create error div if not yet created
Debug.errorPop = function(err) {
	
	//check if Debug._errBox has been set
	if(!Debug._errBox)
	{	
		//check if there is already an error box with same id and share
		var el = document.getElementById(Debug._errBoxId);
		if(!el) //element doesn't already exist, so we need to create the element
		{
			var body = document.getElementsByTagName("BODY")[0];
			
			//create the element
			el = document.createElement("div");			
			el.setAttribute("id", Debug._errBoxId);
			el.style.display = "none";
			var str = "<a href='javascript:Debug.closeErrorPop();'>Close Errors</a><br><br>" + 
				"<div id='" + 
				Debug._errBoxId +
				"-err'></div>";
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
					"}\n\n";
			
			//error box style
			css += "#" + Debug._errBoxId +
					"{" +
					"position: absolute; display: none; border: 2px solid gray;" +
					"background-color: rgba(153,0,51,0.8); overflow-y: scroll;" +
					"overflow-x: hidden;	padding: 5px; -moz-border-radius: 2px;" +
					"-webkit-border-radius: 2px;	border-radius: 2px;" +
					"font-size: 18px; z-index: 2147483647;" + //max 32 bit number z-index
					"font-family: 'Comfortaa', arial; text-align: center;" +
					"left: 8px; top: 8px; margin-right: 8px;" +
					"}\n\n";			

			//error box err text style
			css += "#" + Debug._errBoxId + "-err" +
					"{" +					
					"color: rgb(255,200,100); font-size: 18px;" +
					"font-family: 'Comfortaa', arial;" +
					"left: 8px, top: 8px; margin-right: 8px;" +
					"}\n\n";

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
	
	//add new err to top of errors
	if(str.length)
		err += "<br>...<br>";
	var tstr = d.toLocaleTimeString();
	tstr = tstr.substring(0,tstr.lastIndexOf(' ')) + //convert AM/PM to am/pm with no space
			(tstr[tstr.length-2]=='A'?"am":"pm");
	str = d.toLocaleDateString() + " " + tstr + ": " +
			err.replace(/\n/g , "<br>") + str;
		
	el.innerHTML = str;
	Debug._errBox.style.display = "block";
}


//=====================================================================================
//Close the error popup on the window
Debug.closeErrorPop = function() {
	Debug._errBox.style.display = "none";
	document.getElementById(Debug._errBoxId + "-err").innerHTML = ""; //clear string
}



