


	//	Description of Smart Launch Functionality/Behavior:
	//	
	//		Checkboxes on left to select detector systems to include
	//		Giant button on right to launch
	//
	//		Launch means:
	//			- enable associated contexts
	//			- save new context group, activate (do not alias)
	//			- save user active sessions
	//			- relaunch otsdaq normal mode
	//			- load active sessions
	//			- detect in GUI that system is back alive

//User note:
//	Users can define special launch context groupings by defining
//		global variables: SmartLaunch.subsystems and SmartLaunch.systemToContextMap. 
//	This is demonstrated in otsdaq_demo/UserWebGUI/html/SmartLaunch.html
//		
//	In short, subsystems make up your configuration, and subsystems consist
//		of one or many context records.


//Smart Launch desktop icon from:
//	http://icons.iconarchive.com/icons/bokehlicia/captiva/256/rocket-icon.png


var SmartLaunch = SmartLaunch || {}; //define SmartLaunch namespace

if (typeof Debug == 'undefined')
	throw('ERROR: Debug is undefined! Must include Debug.js before SmartLaunch.js');
else if (typeof Globals == 'undefined')
    throw('ERROR: Globals is undefined! Must include Globals.js before SmartLaunch.js');

SmartLaunch.MENU_PRIMARY_COLOR = "rgb(220, 187, 165)";
SmartLaunch.MENU_SECONDARY_COLOR = "rgb(130, 51, 51)";
	
	
SmartLaunch.launcher; //this is THE SmartLaunch variable
SmartLaunch.doShowContexts = false; //default value for showing contexts or not
SmartLaunch.subsystems = [];
//					   _NAME_STRIPS,
//					   _NAME_MWPC,
//					   _NAME_NIMPLUS
//					   ]; 
SmartLaunch.systemToContextMap = {};
//	SmartLaunch.systemToContextMap[_NAME_STRIPS] = [
//										 "StripTelescopeContext"
//										 ];
//	SmartLaunch.systemToContextMap[_NAME_MWPC] = [
//									   "WireChamberContext",
//									   "ARTDAQBoardReader0", 
//									   "ARTDAQEventBuilder0",
//									   "ARTDAQEventBuilder1",
//									   "ARTDAQAggregator0"
//									   ];
//	SmartLaunch.systemToContextMap[_NAME_NIMPLUS] = [
//										  "NimPlusContext"
//										  ];

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//call createWiz to create instance of a SmartLaunch
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
SmartLaunch.create = function() {


	//functions:			
	//	init()
	//	createElements()
	//	redrawWindow()
	//	readEnabledSubsystems()
	//	this.launch()
	//		localLaunch()
	//		localSetSequenceOfRecords()
	//	this.gatewayLaunchOts()
	//
	//	this.toggleCheckboxDiv(i)
	//	this.handleCheckbox(c)
	
	
	//for display
	var _CHECKBOX_H = 40;
	var _CHECKBOX_W = 240;
	var _LAUNCH_MIN_W = 525;
	var _MARGIN = 40;


	var _needEventListeners = true;

	var _subsetBasePath = "XDAQContextConfiguration";

	var _systemStatusArray = [];
	var _contextRecords = [];

	//////////////////////////////////////////////////
	//////////////////////////////////////////////////
	// end variable declaration
	Debug.log("SmartLaunch.launcher constructed");
	SmartLaunch.launcher = this; 

	init();
	

	//=====================================================================================
	//init ~~
	function init() 
	{						
		Debug.log("Smart Launch init ");
		DesktopContent.tooltip("Smart Launch",
				"Welcome to the Smart Launch user interface. "+
				"Select which pieces of the configuration you want to enable, and then press the launch button!"
		);

		//get all existing contexts
		ConfigurationAPI.getSubsetRecords(
				_subsetBasePath,
				"",//filterList,
				localGetContextRecordsHandler
		);
		
		return;
		
		//////////////////////////////
		function localGetContextRecordsHandler(records)
		{
			//extract context records
			console.log(records);
			_contextRecords = records;
			
			//proceed with rest of init
			createElements();

			if(_needEventListeners)
			{
				window.addEventListener("resize",redrawWindow);
				_needEventListeners = false;
			}

			//redrawWindow();
			readEnabledSubsystems();
		}
		
		

	} //end init()

	//=====================================================================================
	//createElements ~~
	//	called initially to create checkbox and button elements
	function createElements()
	{
		Debug.log("createElements");


		//		<!-- body content populated by javascript -->
		//		<div id='content'>	
		//			
		//			
		//			<div id='subsystemDiv'><div id='subsystemDivContainer'></div></div>
		//		
		//			<a id='launchLink' onclick='SmartLaunch.launcher.launch()'>
		//				<div id='launchDiv'>Launch</div>
		//			</a>	
		//		
		//		</div>
		
		var cel,el,al,sl;
		
		cel = document.createElement("div");
		cel.setAttribute("id","content");

		sl = document.createElement("div");
		sl.setAttribute("id","subsystemDiv");
		el = document.createElement("div");
		el.setAttribute("id","subsystemDivContainer");
		sl.appendChild(el);		
		cel.appendChild(sl);
		
		
		al = document.createElement("a");
		al.setAttribute("id","launchLink");
		al.onclick = function()
				{
			Debug.log("clicked launch");
			SmartLaunch.launcher.launch();
				};
		
		el = document.createElement("div");
		el.setAttribute("id","launchDiv");
		el.innerHTML = "Launch";
		al.appendChild(el);
		
		cel.appendChild(al);		
		document.body.appendChild(cel);
		

	} //end createElements()

	//=====================================================================================
	//readEnabledSubsystems ~~
	//	call redrawWindow when complete
	function readEnabledSubsystems()
	{
		try
		{
			Debug.log("readEnabledSubsystems");
	
			var recordsArray = [];
	
			for(var i=0; i<SmartLaunch.subsystems.length; ++i)
			{ 
				for(var j=0; j<SmartLaunch.systemToContextMap[SmartLaunch.subsystems[i]].length; ++j)
					recordsArray.push(SmartLaunch.systemToContextMap[SmartLaunch.subsystems[i]][j]);
			}
			if(SmartLaunch.doShowContexts)
				for(var i=0; i<_contextRecords.length; ++i)
					recordsArray.push(_contextRecords[i]);
			
			console.log("recordsArray",recordsArray);
	
			ConfigurationAPI.getFieldValuesForRecords(
					_subsetBasePath,
					recordsArray,
					["Status"],
					localStatusForRecordsHandler
			);
			return;
		}
		catch(e)
		{
			Debug.log("There was error reading the status of the configuration subsystems (" +
					"were the subsystem variables setup properly?): " + 
					e, Debug.HIGH_PRIORITY);
			return;
		}
	
		//////////////////////////////
		function localStatusForRecordsHandler(recFieldValues)
		{
			Debug.log("localStatusForRecordsHandler value-length=" + recFieldValues.length)
								var statusArray = []; //clear
			for(var i in recFieldValues)				
				statusArray.push(
						recFieldValues[i].fieldValue == "On"?true:false);//recFieldValues[i].fieldUID .. recFieldValues[i].fieldPath + ": " +

			console.log("statusArray",statusArray);

			_systemStatusArray = []; //clear
			var k=0;
			for(var i=0; i<SmartLaunch.subsystems.length; ++i)
			{ 
				_systemStatusArray.push(true);
				for(var j=0; j<SmartLaunch.systemToContextMap[SmartLaunch.subsystems[i]].length; ++j,++k)
					if(!statusArray[k]) //found one off exception
						_systemStatusArray[i] = false; //so consider entire subsystem off
			}
			if(SmartLaunch.doShowContexts)
				for(var i=0; i<_contextRecords.length; ++i, ++k)
					_systemStatusArray.push(statusArray[k]);

			console.log("_systemStatusArray",_systemStatusArray);

			redrawWindow();

		} //end localStatusForRecordsHandler()
		

	} //end readEnabledSubsystems()


	//=====================================================================================
	//redrawWindow ~~
	//	called when page is resized
	function redrawWindow()
	{
		//adjust link divs to proper size
		//	use ratio of new-size/original-size to determine proper size

		var w = window.innerWidth | 0;
		var h = window.innerHeight | 0;	  

		if(w < _LAUNCH_MIN_W)
			w = _LAUNCH_MIN_W;

		Debug.log("redrawWindow to " + w + " - " + h);	

		var sdiv = document.getElementById("subsystemDiv");
		var ldiv = document.getElementById("launchDiv");

		var chkH = _CHECKBOX_H;
		var chkW = _CHECKBOX_W;

		
		var sdivH = 66 + chkH*SmartLaunch.subsystems.length; //header + chkboxes
		
		if(SmartLaunch.doShowContexts)
		{
			sdivH += 66 + chkH*_contextRecords.length;	//header + chkboxes 	
		}
		if(sdivH > 2/3*h)
			sdivH = 2/3*h; //clip height
		if(sdivH < 100)
			sdivH = 100; //clip min
		
		var sdivW = chkW;
		
		var sdivX = _MARGIN;
		var sdivY = (h-sdivH)/2;
		if(sdivY < _MARGIN)
			sdivY = _MARGIN;

		var ldivX = _MARGIN + chkW;
		var ldivSz = h;		
		if(ldivSz > w - ldivX - _MARGIN*3) //pick min of w/h
			ldivSz = w - ldivX - _MARGIN*3;
		if(ldivSz < 120) //clip min
			ldivSz = 120; 
		var ldivY = (h-ldivSz)/2;


		//draw checkboxes
		{			
			var str = "";
			var statusIndex = 0;
			
			var hideableSubsystems = SmartLaunch.subsystems.length && SmartLaunch.doShowContexts;
			var hideableContexts = SmartLaunch.subsystems.length;
			
			if(SmartLaunch.subsystems.length)
			{			
				if(hideableSubsystems)
					str += "<a onclick='SmartLaunch.launcher.toggleCheckboxDiv(0);' title='Hide/Show Subsystems'>";
				str += "<h3>Subsystems</h3>";
				if(hideableSubsystems)
					str += "</a>";
				
				str += "<div id='ssCheckboxDiv'>";
				for(var i=0;i<SmartLaunch.subsystems.length;++i,++statusIndex)
				{
					str += "<div class='ssCheckbox' style='height:" + chkH + "px;" +
							"' >";
					str += "<div class='pretty p-icon p-round p-smooth' onclick='SmartLaunch.launcher.handleCheckbox(" + 
							statusIndex + ");' >"; //p-smooth for slow transition
					str += " <input type='checkbox' class='subsystemCheckboxes' " +
							(_systemStatusArray[statusIndex]?"checked":"") +
							"/>";
					str += "<div class='state p-success'>";
					str += "<i class='icon mdi mdi-check'></i>";
					str += "<label>" + SmartLaunch.subsystems[i] + "</label>";
					str += "</div>";
					str += "</div>";
					str += "</div>";
					str += "<div id='clearDiv'></div>";
				}
				str += "</div>";
			}
			else if(!SmartLaunch.doShowContexts)
			{
				str += "No subsystem configuration definition found.";
			}
			

			if(SmartLaunch.doShowContexts)
			{
				if(hideableContexts)
					str += "<a onclick='SmartLaunch.launcher.toggleCheckboxDiv(1);' title='Hide/Show Contexts'>";
				str += "<h3>Individual Contexts</h3>";
				if(hideableContexts)
					str += "</a>";
				
				if(!_contextRecords.length)
					str += "No contexts found.";
					

				str += "<div id='ctxCheckboxDiv'>";
				for(var i=0; i<_contextRecords.length; ++i,++statusIndex)
				{
					str += "<div class='ssCheckbox' style='height:" + chkH + "px;" +
							"' >";
					str += "<div class='pretty p-icon p-round p-smooth' onclick='SmartLaunch.launcher.handleCheckbox(" + 
							statusIndex + ");' >"; //p-smooth for slow transition
					str += " <input type='checkbox' class='subsystemCheckboxes' " +
							(_systemStatusArray[statusIndex]?"checked":"") +
							"/>";
					str += "<div class='state p-success'>";
					str += "<i class='icon mdi mdi-check'></i>";
					str += "<label>" + _contextRecords[i] + "</label>";
					str += "</div>";
					str += "</div>";
					str += "</div>";
					str += "<div id='clearDiv'></div>";				
				}
				str += "</div>"
			}


			sdiv.style.left = (sdivX-20) + "px";
			sdiv.style.top = sdivY + "px";
			sdiv.style.height = sdivH + "px";
			sdiv.style.width = sdivW + "px";		
			sdiv.style.display = "block"; 
			

			sdiv = document.getElementById("subsystemDivContainer");
			sdiv.style.left = (sdivX-20) + "px";
			sdiv.style.top = sdivY + "px";
			sdiv.style.height = sdivH + "px";
			sdiv.style.width = sdivW + "px";
			sdiv.innerHTML = "";
			sdiv.innerHTML = str;	
			sdiv.style.display = "block"; 
		}		

		//draw launch button
		{			

			ldiv.style.left = (ldivX + (w - ldivX - _MARGIN*2 - ldivSz)/2) + "px";
			ldiv.style.top = (ldivY+((ldivSz-ldivSz*200/300)/2)) + "px";
			
			ldiv.style.width = ldivSz + "px";
			var fontSize = ldivSz/10;
			if(fontSize < 30) fontSize = 30;
			ldiv.style.fontSize = (fontSize) + "px";
			ldiv.style.paddingTop = (ldivSz*200/300/2 - (fontSize+6)/2) + "px"; //ratio of size minus font size
			ldiv.style.paddingBottom = (ldivSz*200/300/2- (fontSize+6)/2) + "px";
			ldiv.style.borderRadius = (ldivSz*300/100) + "px/" + (ldivSz*200/100) + "px";
			
			ldiv.style.display = "block";
		}

	} //end redrawWindow()

	//=====================================================================================
	//launch ~~
	this.launch = function()
	{
		Debug.log("launch");
		DesktopContent.popUpVerification( 
				"Are you sure you want to relaunch otsdaq?",
				localLaunch,
				0,"#efeaea",0,"#770000");

		//============
		function localLaunch()
		{
			Debug.log("localLaunch");

			var checkboxes = document.getElementsByClassName('subsystemCheckboxes'); 
			console.log("checkboxes",checkboxes);
			var checkedArray = [];
			for(var i=0;i<checkboxes.length;++i)
				checkedArray.push(checkboxes[i].checked);
			console.log("checkedArray",checkedArray);

			var recordsArray = [];
			var valuesArray = [];
			
			//assemble records array
			
			if(SmartLaunch.doShowContexts)
			{
				//just use context checkbox settings
				for(var i=0; i<_contextRecords.length; ++i)
				{					
					recordsArray.push(_contextRecords[i]);
					valuesArray.push(checkedArray[SmartLaunch.subsystems.length+i]?
							"1":"0");
				}
			}
			else
			{
				//extract context settings from system to context map
				for(var i=0; i<SmartLaunch.subsystems.length; ++i)
				{ 
					for(var j=0; j<SmartLaunch.systemToContextMap[SmartLaunch.subsystems[i]].length; ++j)
					{
						recordsArray.push(SmartLaunch.systemToContextMap[SmartLaunch.subsystems[i]][j]);
						valuesArray.push(checkedArray[i]?"1":"0");
	
					}
				}
			}
			console.log("recordsArray",recordsArray);
			console.log("valuesArray",valuesArray);

			var recordIndex = 0;
			var localModifiedTables = undefined;
			//sequentially send request for each record until done
			localSetSequenceOfRecords();

			//===========================
			function localSetSequenceOfRecords()
			{
				ConfigurationAPI.setFieldValuesForRecords(
						_subsetBasePath,
						recordsArray[recordIndex],
						"Status", //fieldArr
						valuesArray[recordIndex], //valueArr
						function(modifiedTables)
						{
					Debug.log("recordIndex = " + recordIndex);
					
					if(modifiedTables.length == 0)
					{
						Debug.log("Something went very wrong. Notify administrators.",
								Debug.HIGH_PRIORITY);
						return;					
					}
					
					++recordIndex;
					if(recordIndex == recordsArray.length)
					{
						Debug.log("Done with sequence.");

						//proceed to save (quietly) tables, groups, aliases
						ConfigurationAPI.saveModifiedTables(modifiedTables,
								/////////////////////
								function(savedTables, savedGroups, savedAliases)
								{
							if(!savedTables.length)
							{
								Debug.log("Something went very wrong. Notify administrators.",
										Debug.HIGH_PRIORITY);
								return;					
							}

							Debug.log("Successfully applied subsystem selections!", Debug.INFO_PRIORITY);


							//relaunch
							Debug.log("Relaunching ots...");
							SmartLaunch.launcher.gatewayLaunchOts();


								}); //end saveModifiedTables handler
						return;
					}

					console.log("setFieldValuesForRecords modifiedTables",modifiedTables);
					localModifiedTables = modifiedTables;
					localSetSequenceOfRecords();

						} //end setFieldValuesForRecords handler
				,localModifiedTables);
			} //end localSetSequenceOfRecords()
		}
	} // end launch()

	//=====================================================================================
	//gatewayLaunchOts ~~
	this.gatewayLaunchOts = function() 
	{
		Debug.log("Relaunching otsdaq!",Debug.INFO_PRIORITY);

		//block check

		DesktopContent._blockSystemCheckMailbox.innerHTML = "1";

		//now in all future requests must ignoreSystemBlock

		DesktopContent.XMLHttpRequest("Request?" + 
				"RequestType=gatewayLaunchOTS", //end get data 
				"", //end post data
				function(req) //start handler
				{
			Debug.log("gatewayLaunchOts handler ");


			var countDown = 10;
			localCountDown();
			//=================
			function localCountDown()
			{
				Debug.log("Waiting " + countDown + " seconds for startup sequence...",
						Debug.INFO_PRIORITY);

				window.setTimeout(function() {
					Debug.log("localCountDown handler ");
					--countDown;
					if(countDown == 0)
					{
						//end blackout
						DesktopContent._blockSystemCheckMailbox.innerHTML = "";
						init();
						Debug.log("And we are back!",Debug.INFO_PRIORITY);
						return;
					}
					localCountDown();
				}, //end blackout-end handler 
				1000/*ms*/);
			}// end localCountDown

				}, //end gatewayLaunchOTS req handler
				0, //handler param
				0,0,true, //progressHandler, callHandlerOnErr, showLoadingOverlay
				true /*targetSupervisor*/, true /*ignoreSystemBlock*/);
	} //end gatewayLaunchOts()


	//=====================================================================================
	//toggleCheckboxDiv(i) ~~
	this.toggleCheckboxDiv = function(i)
	{
		Debug.log("toggleCheckboxDiv(i) " + i);
		
		var el;
		if(i == 0) //toggle subsystems
		{
			el = document.getElementById("ssCheckboxDiv");
			el.style.display = (el.style.display == "none")?"block":"none";
		}
		else if(i == 1) //toggle contextx
		{
			el = document.getElementById("ctxCheckboxDiv");
			el.style.display = (el.style.display == "none")?"block":"none";
		}	
		else throw("Invalid index checkbox div " + i);
	} //end toggleCheckboxDiv()
	
	//=====================================================================================
	//handleCheckbox(i) ~~
	this.handleCheckbox = function(c)
	{
		Debug.log("handleCheckbox(c) " + c);

		var checkboxes = document.getElementsByClassName('subsystemCheckboxes'); 
		console.log("checkboxes",checkboxes);
		var checkedArray = [];
		for(var i=0;i<checkboxes.length;++i)
			checkedArray.push(checkboxes[i].checked);
		console.log("checkedArray",checkedArray);

		Debug.log("set to " + checkedArray[c]);
		
		//create a context map
		var contextEnabledMap = {}; //map to [enabled, index]
		for(var i=0; i<_contextRecords.length; ++i)
			contextEnabledMap[_contextRecords[i]] = [checkedArray[SmartLaunch.subsystems.length+i],
													 SmartLaunch.subsystems.length+i];
				
		//update for each context that was just affected
		if(c < SmartLaunch.subsystems.length)
		{
			//dealing with a subsystem
			for(var j=0; j<SmartLaunch.systemToContextMap[SmartLaunch.subsystems[c]].length; ++j)
			{
				//update map
				contextEnabledMap[ //context name
								  SmartLaunch.systemToContextMap[SmartLaunch.subsystems[c]][j]
																 ][0] = checkedArray[c]; //set to checked value
				//and set checkbox
				checkboxes[contextEnabledMap[ //context name
											  SmartLaunch.systemToContextMap[SmartLaunch.subsystems[c]][j]
																			 ][1]].checked = checkedArray[c];
			}
		}
		else
		{
			//dealing with a context
			
			//update map			
			contextEnabledMap[_contextRecords[c - SmartLaunch.subsystems.length]][0] = checkedArray[c]; //set to checked value			
														  
		}
		console.log("contextEnabledMap",contextEnabledMap);
		
		
		//update all subsystems based on this change
		for(var i=0;i<SmartLaunch.subsystems.length;++i)
		{
			if(i==c) continue; //skip box that was changed

			var enabled = true;
			//if any member contexts are disabled then disable subsystem
			for(var j=0; j<SmartLaunch.systemToContextMap[SmartLaunch.subsystems[i]].length; ++j)
			{
				if(!contextEnabledMap[
									 SmartLaunch.systemToContextMap[SmartLaunch.subsystems[i]][j]][0])
				{
					enabled = false;
					break;
				}
			}
			
			//set checkbox
			checkboxes[i].checked = enabled;			
		}
		
		
	} //end handleCheckbox()


} //end create() SmartLaunch instance











