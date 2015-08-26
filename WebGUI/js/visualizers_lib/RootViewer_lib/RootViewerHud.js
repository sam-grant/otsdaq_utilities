//Hud "class" for RootViewer


RootViewer.createHud = function() {
		
	var hudMouseOverDiv;
	var animationTargetTop, isDropDownAnimating, isDropDownDown;
	
	var hudDirBrowserDiv;
	var hudAdminSettingsDiv;
	var hudPopUpDiv = 0;
		
	var displayingAdminControls = false;
	var PRE_MADE_ROOT_CFG_DIR = "Pre-made Views";
	var adminControlsPath;
	
	var DIR_BRW_HDR_MAX_SIZE = 30;
	var DIR_DISP_TAB_SZ = 16;
	var TUPLE_TYPE = 0, TUPLE_NAME = 1, TUPLE_CONTENT = 2, TUPLE_PARENT = 3;
	var TUPLE_TYPE_FILE = 1, TUPLE_TYPE_DIR = 1<<1, TUPLE_TYPE_DIR_EXPANDED = 1<<2; //bit mask 
	var dirStruct = [[TUPLE_TYPE_DIR,"",0,0]]; //always start with root existing and define from there
									//structure is embedded tuples [<file type>,<name>,<embedded directory contents> or <root file content> for file, <parent ptr>]
	var currDirPtr = dirStruct[0]; //pointer to the directory level that is currently displayed
	
	this.handleWindowResize = function() {
		//Debug.log("RootViewer Hud handleWindowResize");
		
		if(RootViewer.hudAutoHide)
			this.hudMouseOverDiv.style.left = window.innerWidth - this.hudMouseOverDiv.offsetWidth - RootViewer.HUD_MARGIN_RIGHT + "px";
		else
		{
			this.hudMouseOverDiv.style.left = window.innerWidth - this.hudMouseOverDiv.offsetWidth + "px";
			this.hudMouseOverDiv.style.top = -15 + "px";			
		}
		
		hudDirBrowserDiv.style.width = this.hudDiv.offsetWidth - 45 + "px";
		hudDirBrowserDiv.style.height = window.innerHeight - 190 + "px";
				
		if(RootViewer.userPermissions >= RootViewer.ADMIN_PERMISSIONS_THRESHOLD)
			document.getElementById("RootViewer-hudAdminControlsIcon").style.display = "block";
		else
			document.getElementById("RootViewer-hudAdminControlsIcon").style.display = "none";
	}
	
	this.checkboxUpdate = function(i) {
		var chk = document.getElementById("hudCheckbox" + i);
		Debug.log("RootViewer Hud checkboxUpdate " + i + "=" + chk.checked);
		
		if(i==0) RootViewer.autoRefreshDefault = chk.checked; 	//auto refresh
		else if(i==1) 
		{
			RootViewer.hudAutoHide = chk.checked; 				//auto hide
			RootViewer.handleWindowResize();
		}
		else if(i==2) 
		{
			RootViewer.pauseRefresh = chk.checked; 	//pause auto refresh
			
			//reset auto refresh array with re-activation of auto refresh
			//	just in case...
			if(!RootViewer.pauseRefresh) RootViewer.autoRefreshMatchArr = []; 
		}
				
	}
	
	this.handlerRefreshPeriodChange = function(v) {
		v = parseInt(v);
		if(!v || v < 1) v = 1;
		if(v > 9999999) v = 9999999;
		Debug.log("RootViewer Hud handlerRefreshPeriodChange " + v);
		document.getElementById("hudAutoRefreshPeriod").value = v;
		RootViewer.autoRefreshPeriod = v;
		if(RootViewer.autoRefreshTimer) window.clearInterval(RootViewer.autoRefreshTimer);
		RootViewer.autoRefreshTimer = window.setInterval(RootViewer.autoRefreshTick,
			RootViewer.autoRefreshPeriod);
	}
	
	this.radioSelect = function(i) {
		Debug.log("RootViewer Hud radioSelect " + i);
		RootViewer.nextObjectMode = i;
	}
	
	this.handleDirContents = function(req) {
		//Debug.log("RootViewer Hud handleDirContents " + req.responseText);
		
		var path = DesktopContent.getXMLValue(req,'path');
		if(!path) 
		{
			Debug.log("RootViewer Hud handleDirContents no path returned");
			return;
		}
		
		//add results into directory structure
		//var paths = path.split("/");
		//Debug.log("RootViewer Hud handleDirContents " + paths.length + ":" + paths);
		
		//find path
		var baseDir = findDir(path);
		if(!baseDir)
		{
			Debug.log("RootViewer Hud handleDirContents path not found");
			return;
		}		
		//Debug.log("RootViewer Hud handleDirContents baseDir " + baseDir);
		
		
		baseDir[TUPLE_CONTENT] = []; //clear all current content
		baseDir[TUPLE_TYPE]	|= TUPLE_TYPE_DIR_EXPANDED;		//expand the directory
		
		var dirs = req.responseXML.getElementsByTagName("dir");
		var files = req.responseXML.getElementsByTagName("file");
		
		for(var i=0;i<dirs.length;++i) //add dirs
			baseDir[TUPLE_CONTENT][baseDir[TUPLE_CONTENT].length] = [TUPLE_TYPE_DIR,dirs[i].getAttribute("value").replace(/[\/]+/g, ''),0,baseDir];
		
		for(var i=0;i<files.length;++i) //add files
			baseDir[TUPLE_CONTENT][baseDir[TUPLE_CONTENT].length] = [TUPLE_TYPE_FILE,files[i].getAttribute("value").replace(/[\/]+/g, ''),0,baseDir];

		//Debug.log("RootViewer Hud handleDirContents baseDir " + baseDir);
		
		redrawDirectoryDisplay();
	}
	
	//return tuple to path, if not found return 0
	//	recursive function
	// 	calling syntax is
	//			var baseDir = findDir(path);
	var findDir = function(path,currDir,currPath) {
		if(!currDir)
		{
			currDir = dirStruct[0];
			currPath = currDir[TUPLE_NAME] + "/";
			//Debug.log("RootViewer Hud findDir " + currPath);
			//Debug.log("RootViewer Hud findDir path to find " + path);
		}

		if(currDir[TUPLE_TYPE] & TUPLE_TYPE_DIR == 0) return 0; //current path is not a directory, path not found
		if(path == currPath) return currDir; //path is found		
		if(!currDir[TUPLE_CONTENT]) return 0;	//no structure to current path, path not found
		
		//traverse directory content
		var retVal = 0;
		for(var i=0;i<currDir[TUPLE_CONTENT].length;++i)
		{			
			if(currDir[TUPLE_CONTENT][i][TUPLE_TYPE] & TUPLE_TYPE_DIR == 0) continue; //child is not a directory so skip
			
			retVal = findDir(path,currDir[TUPLE_CONTENT][i],currPath + currDir[TUPLE_CONTENT][i][TUPLE_NAME] + "/");
			if(retVal) return retVal;
		}
	}
	
	//return complete path of tuplePtr, return "/" if failure
	var getPath = function(tuplePtr) {
		if(!tuplePtr) return "/";
		var path = tuplePtr[TUPLE_NAME] + "/";
		while(tuplePtr[TUPLE_PARENT])
		{
			path = tuplePtr[TUPLE_PARENT][TUPLE_NAME] + "/" + path;
			tuplePtr = tuplePtr[TUPLE_PARENT];
		}
		return path;
	}
	
	
	//draw for current directory, currDir, as starting level
	//	recursive function
	//	calling syntax redrawDirectoryDisplay(); //will redraw current directory
	var redrawDirectoryDisplay = function(currDir,tabSz,path,str) {
		
		var applyStr = false;
		var locPath;
		var dirClr;
		if(!currDir) //init first time
		{
			hudDirBrowserDiv.innerHTML = ""; //clear all
			str = "";
			currDir = currDirPtr;
			tabSz = 0;
			path = getPath(currDirPtr);
			applyStr = true;
			//Debug.log("RootViewer Hud redrawDirectoryDisplay FIRST path " + path);

			locPath = path.length>DIR_BRW_HDR_MAX_SIZE?("..." + path.substr(path.length-DIR_BRW_HDR_MAX_SIZE+3)):path;
			str += "<div id='RootViewer-hudDirBrowser-header'>";
			str += "<a title='Refresh " + path + "' href='Javascript:RootViewer.hud.changeDirectory(\"" + 
				path + "\");'>" + locPath + "</a>";
			str += "<a title='Change to Parent Directory' style='float:right' href='Javascript:RootViewer.hud.changeDirectory(\"" + 
				getPath(currDirPtr[TUPLE_PARENT]) + "\");'> cd .. </a>";
			str += "</div>";
		}
				
		for(var i=0;currDir[TUPLE_CONTENT] && i<currDir[TUPLE_CONTENT].length;++i)
		{			
			locPath = path + currDir[TUPLE_CONTENT][i][TUPLE_NAME];
			if(currDir[TUPLE_CONTENT][i][TUPLE_TYPE] & TUPLE_TYPE_DIR) locPath += "/"; //if directory add slash
						
			str += "<div class='RootViewer-hudDirBrowser-item' style='margin-left:" + tabSz + "px;'>"; //item container			
			
			dirClr = currDir[TUPLE_CONTENT][i][TUPLE_NAME].indexOf(".root") >= 0?"#B9E6E6":"gray";
			if(currDir[TUPLE_CONTENT][i][TUPLE_TYPE] & TUPLE_TYPE_DIR_EXPANDED)  //dir currently expanded, so action is to minimize it
			{
				str += "<a title='Collapse Directory' href='Javascript:RootViewer.hud.collapseDirectory(\"" + locPath + "\");'> + </a> ";
				
				str += "<a title='Change Directory' style='color:" + dirClr + "' href='Javascript:RootViewer.hud.changeDirectory(\"" + locPath + "\");'>" + currDir[TUPLE_CONTENT][i][TUPLE_NAME] + "</a>";
			}
			else if(currDir[TUPLE_CONTENT][i][TUPLE_TYPE] & TUPLE_TYPE_DIR)  //dir currently minimized, so action is to expand it
			{
				str += "<a title='Expand Directory' style='color:gray' href='Javascript:RootViewer.getDirectoryContents(\"" + locPath + "\");'> - </a> ";
				
				str += "<a title='Change Directory' style='color:" + dirClr + "' href='Javascript:RootViewer.hud.changeDirectory(\"" + locPath + "\");'>" + currDir[TUPLE_CONTENT][i][TUPLE_NAME] + "</a>";
			}
			else if(currDir[TUPLE_CONTENT][i][TUPLE_TYPE] & TUPLE_TYPE_FILE)	//file, so action is to launch it
			{
				if(locPath.indexOf(".root") > 0) //root file
				{
					str += "<a title='Open Root File' href='Javascript:RootViewer.rootReq(\"" + locPath + "\");'>" +
							"<img style='margin:2px 2px -2px 0;' src='/WebPath/js/visualizers_lib/RootViewer_lib/img/histo.png'>";
					str += currDir[TUPLE_CONTENT][i][TUPLE_NAME] + "</a>";
				}
				else if(locPath.indexOf(".rcfg") > 0) //root config file
				{
					str += "<a title='Open Root File' href='Javascript:RootViewer.rootConfigReq(\"" + locPath + "\");'>" +
							"<img style='margin:2px 2px -2px 0;' src='/WebPath/js/visualizers_lib/RootViewer_lib/img/histo3d.png'>";
					str += currDir[TUPLE_CONTENT][i][TUPLE_NAME] + "</a>";
				}
				else
					Debug.log("RootViewer Hud redrawDirectoryDisplay unknown file extension");
			}
			else
				alert("Impossible DIRECTORY error!! Notify admins");
			
			str += "</div>"; //close item container
				
			//recursive call
			if(currDir[TUPLE_CONTENT][i][TUPLE_TYPE] & TUPLE_TYPE_DIR_EXPANDED)
				str = redrawDirectoryDisplay(currDir[TUPLE_CONTENT][i],tabSz+DIR_DISP_TAB_SZ,
						path + currDir[TUPLE_CONTENT][i][TUPLE_NAME] + "/",str);
		}
		
		//if admin pre-made view directory, add ability to MAKE directory or SAVE new premade view
		if(RootViewer.userPermissions >= RootViewer.ADMIN_PERMISSIONS_THRESHOLD &&
				path.indexOf(PRE_MADE_ROOT_CFG_DIR) >= 0)
		{
			Debug.log("RootViewer Hud redrawDirectoryDisplay path " + path);

			var iconArr = ["folderopen","page","remove"];
			var captionArr = ["Make New Directory","Save New View","Delete Pre-made File/Folder!"];
			for(var i=0;i<3;++i)
			{
				str += "<div class='RootViewer-hudDirBrowser-item' style='margin-left:" + tabSz + "px;'>"; //item container	
				str += "<a style='color:gray' title='Admin action: " + captionArr[i] + "' href='Javascript:RootViewer.hud.toggleAdminControls(" + i + ",\"" + path + "\");'>" + 
					"<img style='margin:2px 2px -2px 0;' src='/WebPath/js/visualizers_lib/RootViewer_lib/img/" + iconArr[i] + ".gif'>";
				str += captionArr[i] + "</a>";	
				str += "</div>"; //close item container
			}			
		}
		
		if(applyStr)
			hudDirBrowserDiv.innerHTML = str;
		else 
			return str;
	}
	

	//minimize directory is done by removing the structure from dirPath
	this.collapseDirectory = function(dirPath) {
		Debug.log("RootViewer Hud collapseDirectory  " + dirPath);	
		
		var baseDir = findDir(dirPath);

		baseDir[TUPLE_CONTENT] = 0; //clear array to 0
		baseDir[TUPLE_TYPE] &= ~TUPLE_TYPE_DIR_EXPANDED; //unset dir expanded flag
		        
		redrawDirectoryDisplay(); //redraw current directory			
	}
	
	this.changeDirectory = function(dirPath) {
		Debug.log("RootViewer Hud changeDirectory  " + dirPath);
		currDirPtr = findDir(dirPath);	
		RootViewer.getDirectoryContents(dirPath);
	}
	
	
	// animateDropDown ~~
	var animateDropDown = function() {			
		var dir = (animationTargetTop - hudMouseOverDiv.offsetTop > 0)? 1: -1;
		
		var tmpTop = hudMouseOverDiv.offsetTop + dir*RootViewer.HUD_DROP_DOWN_SPEED;
		if(Math.abs(tmpTop - animationTargetTop) <= RootViewer.HUD_DROP_DOWN_SPEED) //done
		{
			hudMouseOverDiv.style.top = animationTargetTop + "px";
			isDropDownAnimating = false;
			return;
		}
		//else still going towards target
		hudMouseOverDiv.style.top = tmpTop + "px";
		window.setTimeout(animateDropDown,30);
	}

	// mouseOverDropDown ~~
	var mouseOverDropDown = function() {
		
		if(isDropDownAnimating) return; //do nothing if animating currently

		if(!RootViewer.hudAutoHide) return; //if not autohide lock size
		
		if(!isDropDownDown) //start animation
		{
			isDropDownDown = true;
			isDropDownAnimating = true;
			animationTargetTop = -15;
			window.setTimeout(animateDropDown,30);
		}		
	}
	// mouseOutDropDown ~~
	var mouseOutDropDown = function(event) {		
		if(isDropDownAnimating) return; //do nothing if animating currently
		
		if(event)
		{
			var e = event.toElement || event.relatedTarget;			
			while(e)	//if moving within drop down menu ignore
			{
				if(e == this) return;
				e = e.parentNode;
			} 
		}		        

		if(!RootViewer.hudAutoHide) return RootViewer.hud.handleWindowResize(); //if not autohide lock size
		
		if(isDropDownDown) //start animation
		{
			isDropDownDown = false;
			isDropDownAnimating = true;
			animationTargetTop = 15 - hudMouseOverDiv.offsetHeight;
			window.setTimeout(animateDropDown,30);
		}
	}
	
	//types
	//	0 - make directory
	//  1 - save file
	//  2 - delete
	this.toggleAdminControls = function(type, path) {
		displayingAdminControls = !displayingAdminControls;
		Debug.log("RootViewer Hud toggleAdminControls  " + displayingAdminControls);
		
		if(displayingAdminControls) //show admin controls in browser window
		{
			Debug.log("RootViewer Hud toggleAdminControls  " + type + ": " + path);
			
			adminControlsPath = path;
			hudDirBrowserDiv.innerHTML = ""; //clear all
			
			var str = "";
			
			if(type == 0) //make directory
			{
				str += "Make a new ROOT Viewer<br>Configuration Directory<br>at path:<br><br>" + path + "<br>";
				str += "<input type='text' id='hudAdminControlField' onkeyup='document.getElementById(\"hudAdminControlStatus\").innerHTML=\"\";' size='20' value=''><br>";
				str += "<input type='button' onmouseup=\"RootViewer.hud.popUpVerification(" +
					"'Are you sure you want to create directory with name &quot;REPLACE&quot;?','RootViewer.hud.makeConfigDir');\" value='Make New Directory'>";
			}
			else if(type == 1) //new file
			{
				str += "Save a new ROOT Viewer<br>Configuration File for all users <br>based on the current view<br>at path:<br><br>" + path + "<br>";
				str += "<input type='text' id='hudAdminControlField' size='20' value=''><br>";
				
				str += "<div ><input type='checkbox' id='hudSaveFileRunWildCardCheckbox'>" + 
					"<label for='hudSaveFileRunWildCardCheckbox' >" + "Use Wildcard Run #" + "</label></div>";	

				str += "<input type='button' onmouseup=\"RootViewer.hud.popUpVerification(" +
					"'Are you sure you want to save a file with name &quot;REPLACE&quot;?','RootViewer.hud.saveConfigFile');\" value='Save New File'>";
			}
			else if(type == 2) //delete path
			{
				str += "Delete a ROOT Viewer<br>Configuration Directory or File<br>at path:<br><br>" + path + "<br>";
				str += "<input type='text' id='hudAdminControlField' onkeyup='document.getElementById(\"hudAdminControlStatus\").innerHTML=\"\";' size='20' value=''><br>";
				str += "<input type='button' onmouseup=\"RootViewer.hud.popUpVerification(" +
					"'Are you sure you want to delete file or directory with name &quot;REPLACE&quot;?','RootViewer.hud.removeConfigPath');\" value='Delete Path'>";
			
			}
			
			str += "<br><div id='hudAdminControlStatus'></div>";
			str += "<br>";
			str += "<a href='javascript:RootViewer.hud.toggleAdminControls();' title='Return to ROOT Browser'><u>Return to Browser</u></a>";
			hudDirBrowserDiv.innerHTML = str;
			
		}
		else //return to showing current directory
			RootViewer.hud.changeDirectory(getPath(currDirPtr));	//return and refresh directory contents from server
	}

	this.makeConfigDir = function() {
		var dir = document.getElementById('hudAdminControlField').value;
		Debug.log("RootViewer Hud makeConfigDir  " + dir);

		DesktopContent.XMLHttpRequest("request?RequestType=rootAdminControls&cmd=mkdir", "path="+adminControlsPath+"&name="+dir, RootViewer.hud.adminControlsReqHandler);
			
	}

	this.saveConfigFile = function() {
		
		//create file string based on current configuration
		if(RootViewer.numPositionsTiled < 1) 
		{
			document.getElementById('hudAdminControlStatus').innerHTML = "You must have at least 1 Root object in your configuration to save it.";
			return;			
		}
		
		var file = document.getElementById('hudAdminControlField').value;
		var wildcard = document.getElementById('hudSaveFileRunWildCardCheckbox').checked;
		
		var fileStr = "";
		
		fileStr += "<ROOT><DATA>";	
		fileStr += "<numPositionsTiled>" + RootViewer.numPositionsTiled + "</numPositionsTiled>";	
		fileStr += "<runNumWildcard>" + (wildcard?1:0) + "</runNumWildcard>";	
		
		for(var i=0;i<RootViewer.rootElArr.length;++i)
		{
			fileStr += "<rootObjName>" + RootViewer.rootObjNameArr[i] + "</rootObjName>";	
			fileStr += "<rootPos>" + RootViewer.rootPosArr[i] + "</rootPos>";	
			fileStr += "<rootIsTransparent>" + (RootViewer.rootIsTransparentArr[i]?1:0) + "</rootIsTransparent>";	
			fileStr += "<rootIsAutoRefresh>" + (RootViewer.rootIsAutoRefreshArr[i]?1:0) + "</rootIsAutoRefresh>";
		}

		fileStr += "</DATA></ROOT>";
		Debug.log("RootViewer Hud saveConfigFile fileStr  " + fileStr);
		
		DesktopContent.XMLHttpRequest("request?RequestType=rootAdminControls&cmd=save", 
				"path="+adminControlsPath+"&name="+file+"&config="+fileStr, RootViewer.hud.adminControlsReqHandler);
	}

	this.removeConfigPath = function() {

		var target = document.getElementById('hudAdminControlField').value;
		Debug.log("RootViewer Hud removeConfigPath  " + target);		

		DesktopContent.XMLHttpRequest("request?RequestType=rootAdminControls&cmd=delete", "path="+adminControlsPath+"&name="+target, RootViewer.hud.adminControlsReqHandler);
	}

	this.adminControlsReqHandler = function(req) {
		Debug.log("RootViewer Hud adminControlsReqHandler  " + req.responseText);

		var status = DesktopContent.getXMLValue(req,'status');
		
		if(status == "1") //success indicated
			RootViewer.hud.toggleAdminControls();
		else
			document.getElementById('hudAdminControlStatus').innerHTML = status;
	}

	//popUpVerification ~~
	//	asks user if sure
	//	replace REPLACE in prompt with value
	this.popUpVerification = function(prompt, func) {				
		//remove pop up if already exist
		if(hudPopUpDiv) hudPopUpDiv.parentNode.removeChild(hudPopUpDiv);
		
		var path = document.getElementById('hudAdminControlField').value;
		
		var ptrn = /^([a-zA-Z0-9_-]+)$/;
		if(path.length < 3 || !ptrn.test(path))
		{			
			document.getElementById('hudAdminControlStatus').innerHTML = "Entry must be at least 3 characters and alpha-numeric with only underscores and dashes.";
			return;
		}
	
		//replace REPLACE
		prompt = prompt.replace(/REPLACE/g, path); 
         
		var el = this.hudDiv;//document.getElementById("adminArea");	
		hudPopUpDiv = document.createElement("div");
		hudPopUpDiv.setAttribute("class", "hudPopUpDiv");			
		var str = "<div id='hudPopUpText'>" + prompt + "</div>" +
			"<input type='submit' onmouseup='RootViewer.hud.clearPopUpVerification(" + func + ");' value='Yes'> " +
			"&nbsp;&nbsp;&nbsp;" + 
			"<input type='submit' onmouseup='RootViewer.hud.clearPopUpVerification();' value='Cancel'>";
		hudPopUpDiv.innerHTML = str;
		el.appendChild(hudPopUpDiv);
	}
	
	//clearPopUpVerification ~~
	//	call func after clearing, if exists
	this.clearPopUpVerification = function(func) {
		//remove pop up if already exist
		if(hudPopUpDiv) hudPopUpDiv.parentNode.removeChild(hudPopUpDiv);
		hudPopUpDiv = 0;
		if(func) func();
		else	//Action was cancelled by user
			document.getElementById('hudAdminControlStatus').innerHTML = "Action was cancelled by user!";
	}
	
	hudMouseOverDiv = this.hudMouseOverDiv = document.createElement('div');	
	hudMouseOverDiv.setAttribute("id", "RootViewer-hudMouseOver");//RootViewer.hudAutoHide?"RootViewer-hudMouseOver":"RootViewer-hudMouseOver-locked");
	hudMouseOverDiv.style.position = "absolute";	
    hudMouseOverDiv.style.zIndex = 100;
	
	this.hudDiv = document.createElement('div');	
	this.hudDiv.setAttribute("id","RootViewer-hud");// RootViewer.hudAutoHide?"RootViewer-hud":"RootViewer-hud-locked");
	
	

	
	//create content elements
	var str = "";
	str += "With new Root objects...<br>";

	var chkLabels = ["Auto-Refresh"];
	var chkDefaults = [""]; //"checked" for default true	
	str += "<div style='float:right'>"
	for(var i=0;i<chkLabels.length;++i)
		str += "<input type='checkbox' id='hudCheckbox" + i + "' onchange='RootViewer.hud.checkboxUpdate(" + i + 
					");' " + chkDefaults[i] + "><label for='hudCheckbox" + i + "' >" + chkLabels[i] + "</label>";
	str += "</div>";
	
	var radioLabels = ["Tile","Replace", "Superimpose"];
	var radioDefault = RootViewer.nextObjectMode;
	for(var i=0;i<radioLabels.length;++i)
		str += "<input type='radio' id='newRootObjectModeRadio" + i + "' " + (i==radioDefault?"checked":"") +
		" onchange='RootViewer.hud.radioSelect(" + i + ");'" +
		" name='newRootObjectModeRadio' value='0' /><label for='newRootObjectModeRadio" + i + "'>" + radioLabels[i] + "</label><br>";
		
	str += "<hr>";
	
	str += "<div id='RootViewer-hudDirBrowser'></div>";
	//var histos = ["TH1F","TH2F","TProfile","TCanvas"];
	//for(var i=0;i<histos.length;++i)
	//	str += "<a href='javascript:RootViewer.rootReq(\""+histos[i]+"\");'>"+histos[i]+"</a><br>";
	str += "<hr>";
	
	str += "<div id='RootViewer-hudAdminControlsIcon' " + 
		"style='float:left;margin: -2px 0 -20px 20px; cursor: pointer;' onmouseup='RootViewer.hud.toggleAdminControls();' " +
		"title='Admin Controls'><img width='18px' src='/WebPath/images/dashboardImages/icon-Settings.png'></div>";
	
	str += "<div style='float:right; margin:-3px 0 -20px 0;'>";
	str += "Refresh Period: <input type='text' id='hudAutoRefreshPeriod' onchange='RootViewer.hud.handlerRefreshPeriodChange(this.value);' size='6' value='" + 
		RootViewer.autoRefreshPeriod + "'> ms</div>";
		
	str += "<br>";
		
	str += "<a href='javascript:RootViewer.clearAll();' title='Clear ROOT objects from view'>Clear</a>";
	
	str += "<div style='float:right;' ><input type='checkbox' id='hudCheckbox" + chkLabels.length + "' onchange='RootViewer.hud.checkboxUpdate(" + chkLabels.length + 
	");' " + "" + "><label for='hudCheckbox" + chkLabels.length + "' >" + "Auto-Hide" + "</label></div>";	
	
	str += "<div style='float:right;margin-right:10px;' ><input type='checkbox' id='hudCheckbox" + (chkLabels.length+1) + "' onchange='RootViewer.hud.checkboxUpdate(" + (chkLabels.length+1) + 
	");' " + "" + "><label for='hudCheckbox" + (chkLabels.length+1) + "' >" + "Pause Refresh" + "</label></div>";
	
	this.hudDiv.innerHTML = str;
	
	
	
	
	
	
    hudMouseOverDiv.appendChild(this.hudDiv);        

    hudMouseOverDiv.style.width = RootViewer.HUD_WIDTH + "px";
    hudMouseOverDiv.onmouseover = mouseOverDropDown;
    hudMouseOverDiv.onmouseout = mouseOutDropDown;
    RootViewer.omni.appendChild(hudMouseOverDiv);
	
    hudDirBrowserDiv = document.getElementById('RootViewer-hudDirBrowser');	
    
    /*
    hudAdminSettingsDiv = document.createElement('div');	
    hudAdminSettingsDiv.setAttribute("id", "RootViewer-hudAdminSettings");
    hudMouseOverDiv.appendChild(hudAdminSettingsDiv);  
    */
    
    if(RootViewer.hudAutoHide)
    {
		//setup dropdown effect
        hudMouseOverDiv.style.top = 15 - hudMouseOverDiv.offsetHeight + "px";//hudMouseOverDiv.offsetHeight - 15 + "px";
        
		isDropDownDown = false;
		isDropDownAnimating = true;
		animationTargetTop = 15 - hudMouseOverDiv.offsetHeight;
		window.setTimeout(animateDropDown,30);
    }
    else
    	this.handleWindowResize();
}