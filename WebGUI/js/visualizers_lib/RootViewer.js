//=====================================================================================
//
//	Created Aug, 2013
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	RootViewer.js
//
//	requires an omni div, that will be made full window size (window.innerWidth and 
//		window.innerHeight) at position 0,0. 
//
//	Note: there are some variables declared outside of the namespace RootViewer
//		so take care to avoid naming collisions with:
//			- gFile
//			- source_dir
//			- obj_list
//			- RootViewer.objIndex
//			- last_index
//			- function_list
//			- func_list
//			- frame_id
//			- random_id
//
//	public function list:
//		RootViewer.launch()
//
//	from web: 	http://root.cern.ch/js/
//				http://root.cern.ch/drupal/content/trevolutionjs
//				currently using v3.5: https://root.cern.ch/js/3.5/demo/demo.htm
//					-- to change versions, hopefully just replace folder here:
//					-- WebGUI/js/visualizers_lib/RootViewer_lib/JsRoot/... tar.gz file unzipped inside JsRoot
//=====================================================================================


var RootViewer = RootViewer || {}; //define namespace

////////////////////////////////////////////////////////////
//public function definitions

//RootViewer ~~
//called to start Root viewer
RootViewer.launch = function() {

	Debug.log("RootViewer.launch");

	document.getElementById("omni").innerHTML = "<div id='omniHistogramViewer'></div>";
	RootViewer.omni = document.getElementById("omniHistogramViewer");
	
	//initialize with loading message in center
    var w = RootViewer.w = window.innerWidth;
    var h = RootViewer.h = window.innerHeight;

    RootViewer.omni.style.position = "absolute";
    RootViewer.omni.style.left = 0  + "px";
    RootViewer.omni.style.top = 0  + "px";
    RootViewer.omni.style.width = w + "px";
    RootViewer.omni.style.height = h + "px";
    RootViewer.omni.style.backgroundColor = "rgb(30,30,30)";
	
	RootViewer.omni.innerHTML = "<center><div id='loaderStatus' style='margin-top:" + (h/2-8) + "px'>Loading Root Viewer...</div></center>";

	
	//load directories of histograms
	//and display selected histogram
	//allow window splitting and comparing and contrasting with transparent histos
	////////uncomment for initial ROOT example end//////////////////////
	
	loadScript(source_dir+'RootViewerHud.js',function(){
	loadScript(source_dir+'JsRoot/scripts/JSRootCore.js',function(){
			JSROOT.AssertPrerequisites('2d;io;3d;',RootViewer.init);
		}); });
	

	///	Drawing Strategy
	//		- if rootCanvas not created, clear omni and create, full window
	//		- since root js library knows to draw in div with id='report' 
	//			each root object goes in its own div, which at time of creation is given
	//			id='report'. Maintain array of references to divs holding root object
	//			that does not depend on id. And use for removal and movement and sizing?
	//		- first histo always goes full screen within rootCanvas
	//		- next histo based on RADIO: Tile, Replace, Superimpose 
	//			- if TILE, tile it within rootCanvas
	//			- if REPLACE, replace previous object
	//			- if SUPERIMPOSE, place over previous object with alpha 0
	//
	//		- implementation
	//			- keep array of all root elements drawn
	//			- keep array of their corresponding locations
	//			- keep number of locations used in tile


}

//end public function definitions
////////////////////////////////////////////////////////////


var source_dir = "/WebPath/js/visualizers_lib/RootViewer_lib/";
//var gFile;
//var obj_list;
//var last_index;
//var function_list;
//var func_list;
//var frame_id;
//var random_id;

RootViewer.STREAMER_INFO_FILE_PATH = "/WebPath/js/visualizers_lib/RootViewer_lib/streamerInfo.root";

RootViewer.LOAD_STREAMER_INFO_CHECK_PERIOD = 100; //ms
RootViewer.DISPLAY_MIN_WIDTH = 450; //dont allow w or h to go less than this
RootViewer.DISPLAY_MIN_HEIGHT = 300; //dont allow w or h to go less than this
RootViewer.HUD_WIDTH = 300;
RootViewer.HUD_MARGIN_RIGHT = 0;
RootViewer.HUD_DROP_DOWN_SPEED = 40;
RootViewer.ROOT_CONTAINER_OFFY = 20;
RootViewer.ROOT_HEADER_HEIGHT = 20;

RootViewer.TILE_MODE = 0;
RootViewer.REPLACE_MODE = 1;
RootViewer.SUPERIMPOSE_MODE = 2;

RootViewer.ADMIN_PERMISSIONS_THRESHOLD = 100;
RootViewer.userPermissions = 0; //0 is min, 255 is max

RootViewer.omni;
RootViewer.rootContainer;

RootViewer.objIndex;

RootViewer.rootElArr;
RootViewer.rootPosArr;
RootViewer.rootObjArr;
RootViewer.rootObjIndexArr;
RootViewer.rootHeaderElArr;
RootViewer.rootObjNameArr;
RootViewer.rootIsTransparentArr;
RootViewer.rootIsAutoRefreshArr;

RootViewer.numPositionsTiled;
RootViewer.rootTargetIndex; //targeted object for replace or superimpose
RootViewer.w;
RootViewer.h;
RootViewer.sFile;

RootViewer.hudAutoHide = false;
RootViewer.nextObjectMode = 1;
RootViewer.clearEachRequest = true;
RootViewer.autoRefreshDefault = false;
RootViewer.autoRefreshPeriod = 1000;
RootViewer.pauseRefresh = false;

RootViewer.autoRefreshTimer = 0;


RootViewer.iterLoading = false;
RootViewer.iterNumberRemaining;
RootViewer.iterNumPositionsTiled;
RootViewer.iterRunWildcard;
RootViewer.iterRootObjNameArr;
RootViewer.iterRootPosArr;
RootViewer.iterRootIsTransparentArr;
RootViewer.iterRootIsAutoRefreshArr;
RootViewer.iterSaveNextObjectMode;
RootViewer.iterSaveAutoRefreshDefault;

//"private" function list
//RootViewer.init 
//RootViewer.autoRefreshTick
//DesktopContent.prepareNextLocation
//RootViewer.removeAllAtPosition
//RootViewer.manageRootHeaders
//RootViewer.toggleAllAtPositionAutoRefresh
//RootViewer.handleRootPositionSelect
//RootViewer.clearAll
//RootViewer.handleWindowResize
//RootViewer.resizeRootObjects
//RootViewer.refreshTransparency
//RootViewer.checkStreamerInfoLoaded
//RootViewer.getDirectoryContents
//RootViewer.getDirContentsHandler 
//RootViewer.rootReq
//RootViewer.rootConfigReq
//RootViewer.getRootConfigHandler
//RootViewer.iterativeConfigLoader
//RootViewer.getRootDataHandler 
//RootViewer.interpretObjectBuffer

RootViewer.init = function() {
	Debug.log("RootViewer.init");
	//JSROOT.redraw('object_draw', histo, "colz");
	
	
	RootViewer.rootElArr = [];
	RootViewer.rootPosArr = [];
	RootViewer.rootObjArr = [];
	RootViewer.rootObjIndexArr = [];
	RootViewer.rootObjNameArr = [];
	RootViewer.rootHeaderElArr = [];
	RootViewer.rootIsTransparentArr = [];
	RootViewer.rootIsAutoRefreshArr = [];
	
	RootViewer.numPositionsTiled = 0;
	RootViewer.rootTargetIndex = -1;
	
	obj_list = new Array();
	RootViewer.objIndex = 0;
	last_index = 0;
	function_list = new Array();
	func_list = new Array();
	frame_id = 0;
	random_id = 0;

	//create and add report container
	RootViewer.rootContainer = document.createElement('div');  
	RootViewer.rootContainer.setAttribute("id","reportContainer");
	RootViewer.rootContainer.onmouseup = function(){
		Debug.log("Deselect all root containers");
		RootViewer.rootTargetIndex = -1;
		RootViewer.resizeRootObjects();
	};
	RootViewer.omni.appendChild(RootViewer.rootContainer);
	
	RootViewer.hud = new RootViewer.createHud();
	window.onresize = RootViewer.handleWindowResize;
	RootViewer.handleWindowResize();
	
	RootViewer.autoRefreshTimer = window.setInterval(RootViewer.autoRefreshTick,
			RootViewer.autoRefreshPeriod);
	
	document.getElementById("loaderStatus").innerHTML = "Root Viewer Loaded.<br>Use drop-down to make selections.";
	RootViewer.getDirectoryContents("/");
}

RootViewer.autoRefreshMatchArr = []; //use array to match request returns to index

// RootViewer.autoRefreshTick ~~
//	handle autorefreshes 
//	Strategy:
//		For each root object that is in refresh mode, push index,path to an array
//		and send req. When req returns match path to array and remove entry.
//		When array is empty auto refresh complete.
RootViewer.autoRefreshTick = function() {

	//Debug.log("RootViewer autoRefreshTick pause=" + RootViewer.pauseRefresh);
	if(RootViewer.pauseRefresh) return;
	
	if(RootViewer.autoRefreshMatchArr.length) //not done yet with previous refresh!
	{
		Debug.log("RootViewer autoRefreshTick not done yet! Refresh period too short.");
		return;
	}
	
	RootViewer.autoRefreshMatchArr = []; //insert [<index>, <path>] tuples
	for(var j=0;j<RootViewer.rootPosArr.length;++j)
		if(RootViewer.rootIsAutoRefreshArr[j])
		{
			//Debug.log("RootViewer autoRefreshTick " + j + " " + RootViewer.rootObjNameArr[j]);
			RootViewer.autoRefreshMatchArr.push([j, RootViewer.rootObjNameArr[j]]);
			RootViewer.rootReq(RootViewer.rootObjNameArr[j]);
		}	
}

// DesktopContent.prepareNextLocation() ~~
//		Prepares next div location for root js library drawing
//		based on RADIO: Tile, Replace, Superimpose. The div id
//		will be "histogram"+RootViewer.objIndex.. this is the div
//		the root js library will draw to.
DesktopContent.prepareNextLocation = function(objName) {
	Debug.log("RootViewer prepareNextLocation for RootViewer.objIndex " + "mode " + RootViewer.nextObjectMode +
			": " + RootViewer.objIndex + ": " + objName);
	
	//fix target just in case foul play occured
	if(RootViewer.rootTargetIndex > RootViewer.numPositionsTiled) RootViewer.rootTargetIndex = -1;
	
	//create and add new div 
	var ri = RootViewer.rootElArr.length;
	RootViewer.rootElArr.push(document.createElement('div')); //make container  
	RootViewer.rootElArr[ri].setAttribute("class","rootObjectContainer"); //set new report target
	
	var tmpdiv = document.createElement('div'); //make target div
	tmpdiv.setAttribute("id","histogram"+RootViewer.objIndex); //set new target for root object
	tmpdiv.setAttribute("class","rootObjectContainerTarget");
	RootViewer.rootElArr[ri].appendChild(tmpdiv);

	//add report to report container
	RootViewer.rootContainer.appendChild(RootViewer.rootElArr[ri]);
	
	var drawTransparently = false;
	if(!RootViewer.numPositionsTiled || RootViewer.nextObjectMode == RootViewer.TILE_MODE)
	{
		//next tile position (or first tile)
		RootViewer.rootPosArr.push(RootViewer.numPositionsTiled++);
	}
	else if(RootViewer.nextObjectMode == RootViewer.REPLACE_MODE)
	{
		//replace tile(s) at position RootViewer.rootTargetIndex, if -1 then replace last tile(s)		
		var repi = RootViewer.rootTargetIndex == -1? RootViewer.numPositionsTiled-1:RootViewer.rootTargetIndex;		
		RootViewer.removeAllAtPosition(repi);	//remove all tiles that match repi		
		
		RootViewer.rootPosArr.push(repi); //assign new report to position
	}
	else if(RootViewer.nextObjectMode == RootViewer.SUPERIMPOSE_MODE)
	{
		//add tile at position RootViewer.rootTargetIndex, if -1 then at last tile(s)
		var supi = RootViewer.rootTargetIndex == -1? RootViewer.numPositionsTiled-1:RootViewer.rootTargetIndex;
		
		RootViewer.rootPosArr.push(supi);	//assign new report to position
		drawTransparently = true;
	}
	
	RootViewer.rootIsTransparentArr.push(drawTransparently); //keep for transparent drawing
	RootViewer.rootIsAutoRefreshArr.push(RootViewer.autoRefreshDefault);
	RootViewer.rootObjNameArr.push(objName);	//assign new report to position
	
	RootViewer.manageRootHeaders(); 	//manage headers for all positions	
	RootViewer.resizeRootObjects(true); 	//resize all root objects as a result of new element
}

// RootViewer.removeAllAtPosition ~~
//		Remove all histogram div elements and associated root object data structures
//		for the given position i.
//		If isClosingPosition then redraw after and update tiled positions and renumber all above 
//		position i.
RootViewer.removeAllAtPosition = function(posi, isClosingPosition) {
	Debug.log("RootViewer removeAllAtPosition " + posi);

	for(var i=0;i<RootViewer.rootPosArr.length;++i)		
		if(RootViewer.rootPosArr[i] == posi) //remove element 
		{
			RootViewer.rootElArr[i].parentNode.removeChild(RootViewer.rootElArr[i]);
			RootViewer.rootElArr.splice(i,1);
			RootViewer.rootPosArr.splice(i,1);
			delete RootViewer.rootObjArr[i]; RootViewer.rootObjArr[i] = null;
			RootViewer.rootObjArr.splice(i,1);
			RootViewer.rootObjIndexArr.splice(i,1);
			RootViewer.rootObjNameArr.splice(i,1);
			RootViewer.rootIsTransparentArr.splice(i,1);
			RootViewer.rootIsAutoRefreshArr.splice(i,1);			
			
			--i; //rewind
		}
		else if(isClosingPosition && RootViewer.rootPosArr[i] > posi) //renumber position
			--RootViewer.rootPosArr[i];

	if(isClosingPosition)
	{
		--RootViewer.numPositionsTiled;
		RootViewer.manageRootHeaders();		
		if(RootViewer.rootTargetIndex > posi) --RootViewer.rootTargetIndex;
		else if(RootViewer.rootTargetIndex >= RootViewer.numPositionsTiled) RootViewer.rootTargetIndex = -1;
		RootViewer.resizeRootObjects(true); 	//resize all root objects as a result of new element
	}
}

// RootViewer.manageRootHeaders ~~
//	handle adding/removing/drawing of root object headers
RootViewer.manageRootHeaders = function() {
	Debug.log("RootViewer manageRootHeaders");
	
	var tmpdiv;
	while(RootViewer.numPositionsTiled > RootViewer.rootHeaderElArr.length) //add header elements
	{
		tmpdiv = document.createElement('div'); //make target div
		tmpdiv.setAttribute("id","rootContainerHeader-"+RootViewer.rootHeaderElArr.length);
		tmpdiv.setAttribute("class","rootContainerHeader");
		tmpdiv.onmouseup = RootViewer.handleRootPositionSelect;
		RootViewer.rootContainer.appendChild(tmpdiv);
		RootViewer.rootHeaderElArr.push(tmpdiv);
	}

	while(RootViewer.numPositionsTiled < RootViewer.rootHeaderElArr.length) //remove header elements
	{
		tmpdiv = RootViewer.rootHeaderElArr[RootViewer.rootHeaderElArr.length-1];
		tmpdiv.parentNode.removeChild(tmpdiv);
		RootViewer.rootHeaderElArr.splice(RootViewer.rootHeaderElArr.length-1,1);
	}
	
	//give name to headers by position
	var found;
	var name, fullPath;
	var str;
	var isAtLeastOneRefreshing;
	for(var i=0;i<RootViewer.rootHeaderElArr.length;++i)
	{
		found = 0;
		isAtLeastOneRefreshing = false;
		for(var j=0;j<RootViewer.rootPosArr.length;++j)
			if(RootViewer.rootPosArr[j] == i) { ++found; fullPath = RootViewer.rootObjNameArr[j];
				name = (fullPath.length > 20)?("..." + fullPath.substr(fullPath.length-18)):fullPath;
				
				if(RootViewer.rootIsAutoRefreshArr[j]) isAtLeastOneRefreshing = true; //this root object is set to autorefresh
			}
		
		str = "";
		
		//add title
		str += "<div title='" + fullPath + "' class='rootContainerHeader-name'>" + (found == 1?name:"Multiple Files...") + "</div>";
		
		//add close button
		str += "<a title='Close' href='Javascript:RootViewer.removeAllAtPosition("+i+",true);' onmouseup='event.cancelBubble=true;' " +
			"class='rootContainerHeader-closeBtn'>X</a>";
		
		//add auto refresh icon
		//if at least one root object is refreshing show icon as on
		str += "<a title='Close' href='Javascript:RootViewer.toggleAllAtPositionAutoRefresh(" + i + 
			");' onmouseup='event.cancelBubble=true;' " +
			"class='rootContainerHeader-refreshBtn'><img id='rootContainerHeaderRefreshImg" + i +
			"'src='/WebPath/images/iconImages/icon-rootAutoRefresh" + (isAtLeastOneRefreshing?"On":"Off") + ".png'></a>";
		
		RootViewer.rootHeaderElArr[i].innerHTML = str;
	}
}

// RootViewer.toggleAllAtPositionAutoRefresh ~~
//	toggle auto refresh for position i
//	Superimposed position is a special case
//		if any of superimposed are true, then all should go false
//		else all go true
RootViewer.toggleAllAtPositionAutoRefresh = function(i)
{	
	Debug.log("RootViewer toggleAllAtPositionAutoRefresh " + i);
	var found = 0;
	var v = true, lastv;
	var doover = false;
	do
	{
		for(var j=0;j<RootViewer.rootPosArr.length;++j)
			if(RootViewer.rootPosArr[j] == i) 
			{
				if(!doover && RootViewer.rootIsAutoRefreshArr[j]) v = false;
				RootViewer.rootIsAutoRefreshArr[j] = v;

				Debug.log("RootViewer toggleAllAtPositionAutoRefresh rootObj " + j + " to " + v);
				
				var tmp = document.getElementById("rootContainerHeaderRefreshImg" + i );
				tmp.src = "/WebPath/images/iconImages/icon-rootAutoRefresh" + (v?"On":"Off") + ".png";
				if(lastv != v ){++found; lastv = v;}
			}
		if(!doover && found>1) doover = true;
		else doover = false;
	} while(doover) //may need to do it over again, because values of superimposed root objects could be wrong
}

// RootViewer.handleRootPositionSelect ~~
RootViewer.handleRootPositionSelect = function(event) {
	event.cancelBubble = true;
	var i = parseInt(this.id.substr(this.id.indexOf("-")+1))
	Debug.log("RootViewer handleRootPositionSelect " + i);
	RootViewer.rootTargetIndex = i;
	RootViewer.resizeRootObjects();
}

// RootViewer.clearAll ~~
//		remove all root objects
RootViewer.clearAll = function() {
	Debug.log("RootViewer clearAll");
	
	RootViewer.rootTargetIndex = -1;
	//RootViewer.numPositionsTiled will be 0 at end 
	for(RootViewer.numPositionsTiled; RootViewer.numPositionsTiled>0; --RootViewer.numPositionsTiled)
		RootViewer.removeAllAtPosition(RootViewer.numPositionsTiled-1);
	RootViewer.manageRootHeaders();
	RootViewer.resizeRootObjects();
}

// RootViewer.handleWindowResize ~~
RootViewer.handleWindowResize = function() {
	
	var w = RootViewer.w = window.innerWidth < RootViewer.DISPLAY_MIN_WIDTH? RootViewer.DISPLAY_MIN_WIDTH:window.innerWidth;
	var h = RootViewer.h = window.innerHeight < RootViewer.DISPLAY_MIN_HEIGHT? RootViewer.DISPLAY_MIN_HEIGHT:window.innerHeight;

	if(!RootViewer.hudAutoHide) //force w smaller
		RootViewer.w = w -= RootViewer.HUD_WIDTH + RootViewer.HUD_MARGIN_RIGHT + (5*2); //5 is padding of mouseover region in css
	
	Debug.log("RootViewer handleWindowResize " + w + "-" + h);
	
	RootViewer.omni.style.width = w + "px";
	RootViewer.omni.style.height = h + "px";
	
	RootViewer.hud.handleWindowResize();	
	RootViewer.resizeRootObjects();
}

// RootViewer.resizeRootObjects ~~
//		Resize all root objects based on positions and tile arrangement
//		if isForNewObject = true, then redraw all reports except last(new) report
//		do not need to redraw for normal window resize, because obj's handler handles
RootViewer.resizeRootObjects = function(needToRedraw) {
	
	RootViewer.rootContainer.style.width = RootViewer.w + "px";
	RootViewer.rootContainer.style.height = RootViewer.h + "px";	

	if(RootViewer.numPositionsTiled < 1) 
	{ 	//if no rootObjects, invisible container
		RootViewer.rootContainer.style.backgroundColor = "rgba(0,0,0,0)";
		return;
	}
	RootViewer.rootContainer.style.backgroundColor = "white";
	

	var w = RootViewer.w;
	var h = RootViewer.h - RootViewer.ROOT_CONTAINER_OFFY;
			
	var aspect = 3/4; //3/4, 9/16 , 1
	var r = Math.round(Math.sqrt(h*RootViewer.numPositionsTiled/aspect/w)); //Math.ceil(Math.sqrt(RootViewer.numPositionsTiled));
	if(r<1) r = 1;
	var c = Math.ceil(RootViewer.numPositionsTiled/r);
	
	Debug.log("RootViewer resizeRootObjects " + r  + "-" + c + " for " + RootViewer.numPositionsTiled);
	
	//calc individual position size
	w = Math.floor(w/c);
	h = Math.floor(h/r);	
	
	Debug.log("RootViewer resizeRootObjects size " + w  + "-" + h );
	
	//re-calc root object width based on aspect
	var rootAspect = 3/4;
	var rootw = h/w < rootAspect?h/rootAspect:w;
	
	//position all reports properly
	for(var i=0;i<RootViewer.rootElArr.length;++i)
	{
		RootViewer.rootElArr[i].style.width = rootw + "px";
		RootViewer.rootElArr[i].style.height = (h - RootViewer.ROOT_HEADER_HEIGHT) + "px";
		RootViewer.rootElArr[i].style.left = w*(RootViewer.rootPosArr[i]%c) + (w-rootw)/2 + "px";
		RootViewer.rootElArr[i].style.top = RootViewer.ROOT_CONTAINER_OFFY + RootViewer.ROOT_HEADER_HEIGHT + 
			h*Math.floor(RootViewer.rootPosArr[i]/c) + "px";
		
		//		Debug.log("RootViewer resizeRootObjects x y " + i + " at pos " + RootViewer.rootPosArr[i] +
		//				":" + RootViewer.rootElArr[i].style.left  + "-" + RootViewer.rootElArr[i].style.top );
		
		if(needToRedraw && RootViewer.rootObjArr[i]) //redraw for new size, when new added rootobj not defined at this point
		{
			//Debug.log("RootViewer resizeRootObjects redraw " + i  + "-" + RootViewer.rootObjIndexArr[i] );
			//JSROOTPainter.drawObject(RootViewer.rootObjArr[i], RootViewer.rootObjIndexArr[i]);
			JSROOT.redraw('histogram'+
					RootViewer.rootObjIndexArr[i],
					RootViewer.rootObjArr[i], ""); //last arg, root draw option
			RootViewer.refreshTransparency(i);
		}			
	}
	
	//position headers
	for(var i=0;i<RootViewer.rootHeaderElArr.length;++i)
	{
		RootViewer.rootHeaderElArr[i].style.width = w-2 + "px";
		RootViewer.rootHeaderElArr[i].style.height = RootViewer.ROOT_HEADER_HEIGHT  + "px";
		RootViewer.rootHeaderElArr[i].style.left = w*(i%c)  + "px";
		RootViewer.rootHeaderElArr[i].style.top = RootViewer.ROOT_CONTAINER_OFFY + h*Math.floor(i/c) + "px";

		RootViewer.rootHeaderElArr[i].style.borderColor = 
			RootViewer.rootTargetIndex==i?'rgb(68,156,44)':'black';
		RootViewer.rootHeaderElArr[i].style.backgroundColor =
			RootViewer.rootTargetIndex==i?'rgb(178,222,166)':'rgba(0,0,0,0)';
	}
}

// RootViewer.refreshTransparency ~~
//		refresh the transparency state of histogram i and svg components
RootViewer.refreshTransparency = function(i) {
	//if need be, make transparent
	if(RootViewer.rootIsTransparentArr[i])
	{
		//Debug.log("superimpose draw " + i);		
		//histogram div bgColor
		RootViewer.rootElArr[i].children[0].style.backgroundColor = "rgba(0,0,0,0)";
		//svg bgColor
		var svg = RootViewer.rootElArr[i].children[0].getElementsByTagName('svg')[0];
		svg.style.backgroundColor = "rgba(0,0,0,0)";
		//rect fill		
		if(svg.getElementsByTagName('rect')[0])
			svg.getElementsByTagName('rect')[0].style.fill = "rgba(0,0,0,0)";
		
		//structure:
			//<div id='histogram#'><svg><g>
			//<rect x="0" y="0" width="461.48" height="281.70663849600004" fill="rgba(0,0,0,0)" style="stroke: #000000; stroke-width: 1px;"></rect>
			//<svg>..root drawing..</svg>
			//</g>
			//</svg>
	}
}

// RootViewer.checkStreamerInfoLoaded ~~
//	periodically check to usee if the streamer info, giving information on root types has completely loaded
//	this is a critical step before attempting to draw any root objects
RootViewer.checkStreamerInfoLoaded = function() {
	if(RootViewer.sFile && 
			RootViewer.sFile.fStreamerInfo &&
			gFile.fStreamerInfo.fClassMap &&
			gFile.fStreamerInfo.fClassMap.length > 0) //done
	{
		document.getElementById("loaderStatus").innerHTML = "Root Viewer Loaded.<br>Use drop-down to make selections.";
		RootViewer.getDirectoryContents("/");
	}
	else //not done, wait longer
		window.setTimeout(RootViewer.checkStreamerInfoLoaded,RootViewer.LOAD_STREAMER_INFO_CHECK_PERIOD);		
}

// RootViewer.getDirectoryContents ~~
//	request directory contents from server for path
RootViewer.getDirectoryContents = function(path) {
	
	Debug.log("RootViewer getDirectoryContents " + path);
	
	if(path.indexOf(".root/") >=0)
		DesktopContent.XMLHttpRequest("request?RequestType=getRoot", "RootPath="+path, RootViewer.getDirContentsHandler);
	else
		DesktopContent.XMLHttpRequest("request?RequestType=getDirectoryContents", "Path="+path, RootViewer.getDirContentsHandler);
}

// RootViewer.getDirContentsHandler ~~
RootViewer.getDirContentsHandler = function(req) {
	//Debug.log("RootViewer getDirContentsHandler " + req.responseText );
	
	var permissions = DesktopContent.getXMLValue(req,'permissions');
	if(!permissions) 
		Debug.log("RootViewer getDirContentsHandler permissions missing");
	else if(RootViewer.userPermissions != permissions)
	{
		Debug.log("RootViewer getDirContentsHandler user permissions = " + permissions);
		RootViewer.userPermissions = permissions;
		RootViewer.hud.handleWindowResize();
	}
	
	RootViewer.hud.handleDirContents(req);
}

// RootViewer.rootReq ~~
//	if refreshIndex, then request is meant to replace root object at index
RootViewer.rootReq = function(rootPath) {
	
	Debug.log("RootViewer.rootReq " + rootPath );
	DesktopContent.XMLHttpRequest("request?RequestType=getRoot", "RootPath="+rootPath, 
			//RootViewer.tmpRootDataHandler);
			RootViewer.getRootDataHandler);
}

//RootViewer.rootConfigReq ~~
RootViewer.rootConfigReq = function(rootConfigPath) {	
	//Debug.log("RootViewer.rootReq");
	DesktopContent.XMLHttpRequest("request?RequestType=getRootConfig", "RootConfigPath="+rootConfigPath, 
			RootViewer.getRootConfigHandler);
}

//RootViewer.getRootConfigHandler ~~
//	receives saved configuration and rebuilds the view based on the configuration
RootViewer.getRootConfigHandler = function(req) {
	Debug.log("RootViewer getRootConfigHandler " + req.responseText );

	var status = DesktopContent.getXMLValue(req,"status");
	if(status != "1")
	{ alert("Loading Root Pre-Made Configuration Failed: " + status); return }
	
	RootViewer.iterNumPositionsTiled = DesktopContent.getXMLValue(req,"numPositionsTiled");
	RootViewer.iterRunWildcard = DesktopContent.getXMLValue(req,"runNumWildcard");  //TODO replace obj names with current run number!
	
	//copy NodeList to just normal arrays
	
	var tmp = req.responseXML.getElementsByTagName("rootObjName");
	RootViewer.iterRootObjNameArr = [];
	for(var i=0;i<tmp.length;++i) RootViewer.iterRootObjNameArr[i] = tmp[i].getAttribute("value");
	
	tmp = req.responseXML.getElementsByTagName("rootPos");
	RootViewer.iterRootPosArr = [];
	for(var i=0;i<tmp.length;++i) RootViewer.iterRootPosArr[i] = tmp[i].getAttribute("value") | 0; //parse as int
	
	tmp = req.responseXML.getElementsByTagName("rootIsTransparent");
	RootViewer.iterRootIsTransparentArr = [];
	for(var i=0;i<tmp.length;++i) RootViewer.iterRootIsTransparentArr[i] = tmp[i].getAttribute("value") | 0; //parse as int
	
	tmp = req.responseXML.getElementsByTagName("rootIsAutoRefresh");
	RootViewer.iterRootIsAutoRefreshArr = [];
	for(var i=0;i<tmp.length;++i) RootViewer.iterRootIsAutoRefreshArr[i] = tmp[i].getAttribute("value") | 0; //parse as int
		
	RootViewer.clearAll();		

	RootViewer.iterLoading = true;
	RootViewer.iterNumberRemaining = RootViewer.iterRootObjNameArr.length;
	RootViewer.iterSaveNextObjectMode = RootViewer.nextObjectMode;
	RootViewer.iterSaveAutoRefreshDefault = RootViewer.autoRefreshDefault;
	
	RootViewer.iterativeConfigLoader();
}

//RootViewer.iterativeConfigLoader ~~
//	goes through every iterRootObj and loads sequentially to display
RootViewer.iterativeConfigLoader = function() {
	//Debug.log("RootViewer iterativeConfigLoader " + RootViewer.iterNumberRemaining);
	if(!RootViewer.iterNumberRemaining)  //done
	{	
		RootViewer.autoRefreshDefault = RootViewer.iterSaveAutoRefreshDefault;
		RootViewer.nextObjectMode = RootViewer.iterSaveNextObjectMode;
		RootViewer.iterLoading = false; 
		return;
	}
	
	--RootViewer.iterNumberRemaining; 
	
	//next is always the lowest position left
	var min = -1;
	for(var i=0;i<RootViewer.iterRootPosArr.length;++i)
		if(min == -1 || RootViewer.iterRootPosArr[i] < RootViewer.iterRootPosArr[min]) min = i;
	
	RootViewer.nextObjectMode = RootViewer.iterRootIsTransparentArr[min]?RootViewer.SUPERIMPOSE_MODE:RootViewer.TILE_MODE;
	RootViewer.autoRefreshDefault = RootViewer.iterRootIsAutoRefreshArr[min];
	
	RootViewer.rootReq(RootViewer.iterRootObjNameArr[min]);	
	
	//remove from iter array
	RootViewer.iterRootObjNameArr.splice(min,1);
	RootViewer.iterRootPosArr.splice(min,1);
	RootViewer.iterRootIsTransparentArr.splice(min,1);
	RootViewer.iterRootIsAutoRefreshArr.splice(min,1);
	
}


// RootViewer.getRootDataHandler ~~
//	receives streamed root object from server and prepares it for js structures
RootViewer.getRootDataHandler = function(req) {

	//Debug.log("RootViewer getRootDataHandler " + req.responseText );
	
	var rootType = DesktopContent.getXMLValue(req,"rootType");
	var rootStr = DesktopContent.getXMLValue(req,"rootData");
	var rootName = DesktopContent.getXMLValue(req,"path");//
	//"my" + rootType + RootViewer.objIndex;// DesktopContent.getXMLValue(req,"path");// + RootViewer.objIndex;
	//if(rootName.length > 20) rootName = "..." + rootName.substr(rootName.length-18);
	
	var rootJSON = DesktopContent.getXMLValue(req,"rootJSON");
		
	//Debug.log("RootViewer tmpRootDataHandler JSON \n\n" + rootJSON );
		
	var ojbect = JSROOT.parse(rootJSON);
		
	if(!ojbect || !rootType || !rootName)
	{ alert("Error reading Root object from server - Name: " + rootName); return; }
	

	var refreshIndex = -1; //default to -1 if no auto refresh needed
	if(RootViewer.autoRefreshMatchArr.length) //check if request matches auto refresh entry
	{
		for(var i=0;i<RootViewer.autoRefreshMatchArr.length;++i)
		{
			if(rootName == RootViewer.autoRefreshMatchArr[i][1])
			{
				refreshIndex = RootViewer.autoRefreshMatchArr[i][0];	
								
				//remove from auto refresh array
				RootViewer.autoRefreshMatchArr[i] = 0;
				RootViewer.autoRefreshMatchArr.splice(i,1);

				//if name in js structures has changed,
				//	assume it is users fault and throw out this refreshed object
				if(refreshIndex >= RootViewer.rootObjNameArr.length ||
					RootViewer.rootObjNameArr[refreshIndex] != rootName)
				{
					Debug.log("RootViewer getRootDataHandler weird unmatch!?#$@%");
					return; //throw out object, since incomplete match
				}				
				break;
			}
		}				
		//if not found, assume it is a new object
	}
	
	if(refreshIndex < 0) DesktopContent.prepareNextLocation(rootName);
	RootViewer.interpretObjectJSON(ojbect,rootType,rootName,refreshIndex);
	if(RootViewer.iterLoading) RootViewer.iterativeConfigLoader();
}


// RootViewer.interpretObjectJSON ~~
//	interpret and draw
RootViewer.interpretObjectJSON = function(object,rootType,objName,refreshIndex) {

	if(refreshIndex == undefined) refreshIndex = -1;
	
	//draw based on refresh index
	JSROOT.redraw('histogram'+
			(refreshIndex<0?RootViewer.objIndex:
			RootViewer.rootObjIndexArr[refreshIndex]),
			object, ""); //last arg, root draw option
	
	if(refreshIndex < 0)
	{
		RootViewer.rootObjArr.push(object);
		RootViewer.rootObjIndexArr.push(RootViewer.objIndex);				
		RootViewer.objIndex++;
	}
	else //use refresh index
	{
		delete RootViewer.rootObjArr[refreshIndex]; RootViewer.rootObjArr[refreshIndex] = null;
		RootViewer.rootObjArr[refreshIndex] = object;
	}
	
	RootViewer.refreshTransparency(refreshIndex<0?(RootViewer.rootObjArr.length-1):refreshIndex);

}


function loadScript(url, callback) {
   // dynamic script loader using callback
   // (as loading scripts may be asynchronous)
   var script = document.createElement("script")
   script.type = "text/javascript";
   if (script.readyState) { // Internet Explorer specific
      script.onreadystatechange = function() {
         if (script.readyState == "loaded" ||
             script.readyState == "complete") {
            script.onreadystatechange = null;
            callback();
         }
      };
   } else { // Other browsers
      script.onload = function(){
         callback();
      };
   }
   var rnd = Math.floor(Math.random()*80000);
   script.src = url;//+ "?r=" + rnd;
   document.getElementsByTagName("head")[0].appendChild(script);
};
