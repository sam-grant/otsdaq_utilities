


	//	Description of Code Editor Functionality/Behavior:
	//	
	//		Folder Icon in top-left (and again if split pane for right side, or bottom)
	//		(when file open) Save Icon in top-left (and again if split pane for right side, or bottom)
	//
	//		Split Pane Icon in top-right (toggle horiz/vert split)
	//		Incremental Build Icon in top-right (mrb b)
	//		Clean Build Icon in top-right (mrb z)
	//
	//		Use console to view build results
	//
	//		Recent files at start of folder display
	//		
	//		Selecting a file in folder, opens the Code Editor Box for that file.
	//			- Save Icon then appears next to Folder Icon
		


var CodeEditor = CodeEditor || {}; //define CodeEditor namespace

if (typeof DesktopContent == 'undefined')
	throw('ERROR: DesktopContent is undefined! Must include DesktopWindowContentCode.js before CodeEditor.js');


CodeEditor.MENU_PRIMARY_COLOR = "rgb(220, 187, 165)";
CodeEditor.MENU_SECONDARY_COLOR = "rgb(130, 51, 51)";
	
	
CodeEditor.editor; //this is THE CodeEditor variable


//htmlOpen(tag,attObj,innerHTML,closeTag)
//htmlClearDiv()

//=====================================================================================
//htmlOpen ~~		
//	tab name and attribute/value map object
function htmlOpen(tag,attObj,innerHTML,doCloseTag)
{
	var str = "";
	var attKeys = Object.keys(attObj); 
	str += "<" + tag + " ";
	for(var i=0;i<attKeys.length;++i)
		str += " " + attKeys[i] + "='" +
		attObj[attKeys[i]] + "' ";
	str += ">";
	if(innerHTML) str += innerHTML;
	if(doCloseTag)
		str += "</" + tag + ">";
	return str;
} // end htmlOpen()

//=====================================================================================
//htmlClearDiv ~~		
function htmlClearDiv()
{
	return "<div id='clearDiv'></div>";
} //end htmlClearDiv()


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//call create to create instance of a SmartLaunch
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
CodeEditor.create = function() {

	DesktopContent.tooltip("Code Editor",
			"Welcome to the Code Editor user interface. "+
			"Edit your code, save it, and compile!\n\n" +
			"To open a file, use the folder icon in the top-left to navigate to a code file to edit.\n\n" +
			"To toggle view, use the split-pane icon in the top-right to toggle another code editor in the same window.\n\n" +
			"To save, use the save icon in the top-left to save your changes.\n\n" +
			"To compile, use the Incremmental Build or Clean Build icons in the top-right. "
	);

	//outline:			
	//
	//	"private":
	//	================
	//	init()
	//	redrawWindow()
	//	createElements()
	//		localCreatePaneControls()
	//	createTextEditor(forPrimary)
	//	createDirectoryNav(forPrimary)
	
	//  "public":
	//	================
	//	toggleDirectoryNav(forPrimary,v)
	//	saveFile(forPrimary)
	//	toggleView(v)
	//	build(cleanBuild)
	//	openDirectory(forPrimary,path)
	//	handleDirectoryContent(forPrimary,req)
	//	openFile(forPrimary,path,extension,doConfirm)
	//	handleFileContent(forPrimary,req)
	//	updateDecorations(forPrimary,insertNewLine)
	//		localInsertLabel(startPos)
	//	updateOutline(forPrimary)
	//	handleOutlineSelect(forPrimary)
	//	updateLastSave(forPrimary)	
	//	keyDownHandler(e,forPrimary,shortcutsOnly)
	//	handleFileNameMouseMove(forPrimary,doNotStartTimer)
	//	startEditFileName(forPrimary)	
	//	editCellOK(forPrimary)
	//	editCellCancel(forPrimary)
	
	
	//for display
	var _WINDOW_MIN_SZ = 525;

	var _needEventListeners = true;
	
	var _viewMode = 0; //0: only primary, 1: vertical split, 2: horizontal split
	var _navMode = [0,0]; //1 for showing directory nav
	var _filePath = ["",""]; //file path for primary and secondary
	var _fileExtension = ["",""]; //file extension for primary and secondary
	var _fileLastSave = [0,0]; //file last save time for primary and secondary
	var _fileWasModified = [false,false]; //file wasModified for primary and secondary
	var _numberOfLines = [0,0];

	var _inputTimerHandle = 0;
	var _fileNameMouseMoveTimerHandle = 0;
	var _fileNameEditing = [false,false]; //for primary and secondary
	
	var _activePaneIsPrimary = 1; //default to primary, and switch based on last click
	
	//////////////////////////////////////////////////
	//////////////////////////////////////////////////
	// end variable declaration
	CodeEditor.editor = this;
	Debug.log("CodeEditor.editor constructed");
	
	init();	
	Debug.log("CodeEditor.editor initialized");


	//=====================================================================================
	//init ~~
	function init() 
	{						
		Debug.log("Code Editor init ");

		//extract GET parameters
		var parameterStartFile = [
								  //"/otsdaq/otsdaq-core/CoreSupervisors/version.h",
								  //"/otsdaq_components/otsdaq-components/FEInterfaces/FEOtsUDPTemplateInterface.h",
								  //"/otsdaq_components/otsdaq-components/FEInterfaces/FEOtsUDPTemplateInterface_interface.cc",
								  //"/CMakeLists.txt", 
								  //"/CMakeLists.txt",
								  DesktopContent.getParameter(0,"startFilePrimary"),
								  DesktopContent.getParameter(0,"startFileSecondary")
		   ];
		var parameterViewMode = DesktopContent.getParameter(0,"startViewMode");
		if(parameterViewMode !== undefined) //set view mode if parameter
		{
			_viewMode = parameterViewMode|0;
		}
		console.log("parameterStartFile",parameterStartFile);
		console.log("parameterViewMode",parameterViewMode);
		
		//_viewMode = 2; for debugging
		
		//proceed
		
		createElements();
		redrawWindow();
		
		if(_needEventListeners)
		{
			window.addEventListener("resize",redrawWindow);
			_needEventListeners = false;
		}
			
	

		DesktopContent.XMLHttpRequest("Request?RequestType=codeEditor" + 
				"&option=getDirectoryContent" +
				"&path=/"
				, "" /* data */,
				function(req)
				{
			 
			var err = DesktopContent.getXMLValue(req,"Error"); //example application level error
			if(err) 
			{
				Debug.log(err,Debug.HIGH_PRIORITY);	//log error and create pop-up error box
				return;
			}
			
			var fileSplit; 
			
			console.log("getDirectoryContent",req);
			
			CodeEditor.editor.handleDirectoryContent(1 /*forPrimary*/, req);
			CodeEditor.editor.handleDirectoryContent(0 /*forPrimary*/, req);
			
			//decide how to start display(file or directory)
			fileSplit = [];
			if(parameterStartFile[0] && parameterStartFile[0] != "")
				fileSplit = parameterStartFile[0].split('.');
			
			if(fileSplit.length == 2) //show shortcut file
				CodeEditor.editor.openFile(1 /*forPrimary*/, 
						fileSplit[0]	/*path*/,
						fileSplit[1] /*extension*/);
			else //show base directory nav
				CodeEditor.editor.toggleDirectoryNav(1 /*forPrimary*/, 1 /*showNav*/);
			
			//for secondary pane
			fileSplit = [];
			if(parameterStartFile[1] && parameterStartFile[1] != "")
				fileSplit = parameterStartFile[1].split('.');

			if(fileSplit.length == 2) //show shortcut file
				CodeEditor.editor.openFile(0 /*forPrimary*/, 
						fileSplit[0]	/*path*/,
						fileSplit[1] /*extension*/);
			else //show base directory nav
				CodeEditor.editor.toggleDirectoryNav(0 /*forPrimary*/, 1 /*showNav*/);
			

			_activePaneIsPrimary = 1; //default active pane to primary
		    
				}, 0 /*progressHandler*/, 0 /*callHandlerOnErr*/, 1 /*showLoadingOverlay*/);
					

	} //end init()


	
	
	//=====================================================================================
	//createElements ~~
	//	called initially to create checkbox and button elements
	function createElements()
	{
		Debug.log("createElements");


		//		<!-- body content populated by javascript -->
		//		<div id='content'>
		//			<div id='primaryPane'></div>	<!-- for primary view -->
		//			<div id='secondaryPane'></div>	<!-- for horiz/vert split view -->	
		//			<div id='controlsPane'></div>  	<!-- for view toggle and compile -->
		//		</div>
		
		var cel,el,al,sl,str;
		
		cel = document.getElementById("content");
		if(!cel)
		{
			cel = document.createElement("div");
			cel.setAttribute("id","content");
		}

		//clear all elements
		cel.innerHTML = "";
		
		{ //content div		

			//================
			//primaryPane and secondaryPane div
			var forPrimary;
			for(forPrimary=1;forPrimary >= 0;--forPrimary)
			{
				el = document.createElement("div");	
				el.setAttribute("class","editorPane");	
				el.setAttribute("id","editorPane" + forPrimary);
				{
					var str = "";
					str += createTextEditor(forPrimary);
					str += createDirectoryNav(forPrimary);
					str += localCreatePaneControls(forPrimary);				
					el.innerHTML = str;
				} //end fill editor pane			
				cel.appendChild(el);
			}			

			//================
			function localCreatePaneControls(forPrimary)
			{
				//add folder, and save buttons
				//add directory nav and editor divs

				str = "";

				//local pane controls
				str += htmlOpen("div",
						{
								"class":"controlsPane",
						},"" /*innerHTML*/, 0 /*doCloseTag*/);
				{
					//folder
					str += htmlOpen("div",
							{
									"id":"directoryNavToggle",
									"class":"controlsButton",
									"style":"float:left",
									"onclick":"CodeEditor.editor.toggleDirectoryNav(" + forPrimary + ");",
									"title": "Open a file... (Ctrl + D)",
							},"" /*innerHTML*/, 0 /*doCloseTag*/);
					{
						str += htmlOpen("div",
								{
										"id":"directoryNavToggleTop",

								},"" /*innerHTML*/, 1 /*doCloseTag*/);
						str += htmlOpen("div",
								{
										"id":"directoryNavToggleBottom",

								},"" /*innerHTML*/, 1 /*doCloseTag*/);
					} //end directoryNavToggle
					str += "</div>"; //close directoryNavToggle

					//save
					str += htmlOpen("div",
							{
									"id":"saveFile",
									"class":"controlsButton",
									"style":"float:left;",
									"onclick":"CodeEditor.editor.saveFile(" + forPrimary + ");",
									"title": "Save the file (Ctrl + S)",
							},"" /*innerHTML*/, 0 /*doCloseTag*/);
					{
						str += htmlOpen("div",
								{
										"id":"saveFileMain",

								},"" /*innerHTML*/, 1 /*doCloseTag*/);
						str += htmlOpen("div",
								{
										"id":"saveFileMainTop",

								},"" /*innerHTML*/, 1 /*doCloseTag*/);
						str += htmlOpen("div",
								{
										"id":"saveFileMainBottom",

								},"" /*innerHTML*/, 1 /*doCloseTag*/);
					} //end directoryNavToggle
					str += "</div>"; //close saveFile

				} //end locals controlsPane
				str += "</div>"; //close controlsPane
				return str;
			} //end localCreatePaneControls



			//================
			//controlsPane div
			el = document.createElement("div");
			el.setAttribute("class","controlsPane");	
			{
				//add view toggle, incremental compile, and clean compile buttons

				str = "";

				//view toggle
				str += htmlOpen("div",
						{
								"id":"viewToggle",
								"class":"controlsButton",
								"style":"float:right",
								"onclick":"CodeEditor.editor.toggleView();",
								"title":"Toggle Verical/Horizontal Split-view",
						},"" /*innerHTML*/, 0 /*doCloseTag*/);
				{

					str += htmlOpen("div",
							{
									"id":"viewToggleRight",

							},"" /*innerHTML*/, 1 /*doCloseTag*/);
					str += htmlOpen("div",
							{
									"id":"viewToggleLeftTop",

							},"" /*innerHTML*/, 1 /*doCloseTag*/);
					str += htmlOpen("div",
							{
									"id":"viewToggleLeftBottom",

							},"" /*innerHTML*/, 1 /*doCloseTag*/);
				}
				str += "</div>"; //close viewToggle

				//incremental compile
				str += htmlOpen("div",
						{
								"id":"incrementalBuild",
								"class":"controlsButton",
								"style":"float:right",
								"onclick":"CodeEditor.editor.build(0 /*cleanBuild*/);",
								"title":"Incremental Build... (Ctrl + B)",
						},"" /*innerHTML*/, 0 /*doCloseTag*/);
				{

					str += htmlOpen("div",
							{
									"style":"margin:11px 0 0 13px;",
							},"b" /*innerHTML*/, 1 /*doCloseTag*/);
				}
				str += "</div>"; //close incrementalBuild

				//clean compile
				str += htmlOpen("div",
						{
								"id":"cleanBuild",
								"class":"controlsButton",
								"style":"float:right",
								"onclick":"CodeEditor.editor.build(1 /*cleanBuild*/);",
								"title":"Clean Build... (Ctrl + C)",
						},"" /*innerHTML*/, 0 /*doCloseTag*/);
				{

					str += htmlOpen("div",
							{
									"style":"margin:10px 0 0 13px;",
							},"z" /*innerHTML*/, 1 /*doCloseTag*/);
				}
				str += "</div>"; //close cleanBuild

				el.innerHTML = str;	
			}
			cel.appendChild(el);

		} //end content div					
		
		document.body.appendChild(cel);
		
		
		/////////////
		//add event listeners
		var box;
		for(var i=0;i<2;++i)
		{
			box = document.getElementById("editableBox" + i);
			box.addEventListener("input",
					function(e)
					{
				var forPrimary = this.id[this.id.length-1]|0;
				forPrimary = forPrimary?1:0;
				
				Debug.log("input forPrimary=" + forPrimary);

				_fileWasModified[forPrimary] = true;
				CodeEditor.editor.updateLastSave(forPrimary);

				window.clearTimeout(_inputTimerHandle);
				_inputTimerHandle = window.setTimeout(
						function()
						{
					CodeEditor.editor.updateDecorations(forPrimary);				
						}, 1000); //end setTimeout

					}); //end addEventListener

			box.addEventListener("keydown",
							function(e)
							{
				var forPrimary = this.id[this.id.length-1]|0;				
				Debug.log("keydown handler for editableBox" + forPrimary);
				CodeEditor.editor.keyDownHandler(e,forPrimary);
				e.stopPropagation();
							}); //end addEventListener
			

			//add click handler to track active pane
			box = document.getElementById("editorPane" + i);
			box.addEventListener("click",
							function(e)
							{
				var forPrimary = this.id[this.id.length-1]|0;
				forPrimary = forPrimary?1:0;
				
				Debug.log("click handler for pane" + forPrimary);
				_activePaneIsPrimary = forPrimary;
							}); //end addEventListener
			
			
			
		} //end handler creation
		box = document.body;
		box.addEventListener("keydown",
				function(e)
				{
			var forPrimary = _activePaneIsPrimary; //take last active pane
			Debug.log("keydown handler for body" + forPrimary);
			CodeEditor.editor.keyDownHandler(e,forPrimary,true /*shortcutsOnly*/);
			//e.stopPropagation();
				}); //end addEventListener
		
	} //end createElements()

	//=====================================================================================
	//createTextEditor ~~
	function createTextEditor(forPrimary)
	{
		forPrimary = forPrimary?1:0;
		
		Debug.log("createTextEditor forPrimary=" + forPrimary);
		
		var str = "";
		
		str += htmlOpen("div",
				{
						"class":"textEditor",
						"id":"textEditor" + forPrimary,
						"style":"overflow:hidden;",
				},0 /*innerHTML*/, false /*doCloseTag*/);
		
		//add header
		//add body with overflow:auto
			//add leftMargin
			//add editorBox
		

		str += htmlOpen("div",
				{
						"class":"textEditorHeader",
						"id":"textEditorHeader" + forPrimary,
				},0 /*"<div>File</div><div>Save Date</div>"*/ /*innerHTML*/, 
				true /*doCloseTag*/);

		str += htmlOpen("div",
				{
						"class":"textEditorBody",
						"id":"textEditorBody" + forPrimary,
				},0 /*innerHTML*/, false /*doCloseTag*/);
		
		str += "<table class='editableBoxTable' style='margin-bottom:200px'>" + //add white space to bottom for expected scroll behavior
				"<tr><td valign='top'>";
		str += htmlOpen("div",
				{
						"class":"editableBoxLeftMargin",
						"id":"editableBoxLeftMargin" + forPrimary,
				},"0\n1\n2" /*html*/,true /*closeTag*/);
		str += "</td><td valign='top'>";
		str += htmlOpen("div",
				{
						"class":"editableBox",
						"id":"editableBox" + forPrimary,	
						"contenteditable":"true",
						"autocomplete":"off",
						"autocorrect":"off",
						"autocapitalize":"off", 
						"spellcheck":"false",
				},0 /*html*/,true /*closeTag*/);
		str += "</td></tr></table>"; //close table	

		str += "</div>"; //close textEditorBody tag
		
		str += "</div>"; //close textEditor tag
		
		return str;		
	} //end createTextEditor()
	
	//=====================================================================================
	//createDirectoryNav ~~
	function createDirectoryNav(forPrimary)
	{
		forPrimary = forPrimary?1:0;
		
		Debug.log("createDirectoryNav forPrimary=" + forPrimary);	

		var str = "";

		str += htmlOpen("div",
				{
						"class":"directoryNav",
						"id":"directoryNav" + forPrimary,
				},"Directory" /*innerHTML*/, 1 /*doCloseTag*/);
		
		return str;
		
	} //end createDirectoryNav()
	
	//=====================================================================================
	//redrawWindow ~~
	//	called when page is resized
	function redrawWindow()
	{
		//adjust link divs to proper size
		//	use ratio of new-size/original-size to determine proper size

		var w = window.innerWidth | 0;
		var h = window.innerHeight | 0;	  

		if(w < _WINDOW_MIN_SZ)
			w = _WINDOW_MIN_SZ;
		if(h < _WINDOW_MIN_SZ)
			h = _WINDOW_MIN_SZ;

		Debug.log("redrawWindow to " + w + " - " + h);	

		var eps = document.getElementsByClassName("editorPane");
		var epHdrs = document.getElementsByClassName("textEditorHeader");
		var epBdys = document.getElementsByClassName("textEditorBody");
		var dns = document.getElementsByClassName("directoryNav");
		var rect = [{},{}];
		
		
		eps[0].style.position = "absolute";
		eps[1].style.position = "absolute";
			
		var DIR_NAV_MARGIN = 50;
		var EDITOR_MARGIN = 20;
		var EDITOR_HDR_H = 56;
		switch(_viewMode)
		{
		case 0: //only primary
			
			rect = [{"left":0,"top":0,"w":w,"h":h},
					undefined];			
			break;
		case 1: //vertical split

			rect = [{"left":0,"top":0,"w":((w/2)|0),"h":h},
					{"left":((w/2)|0),"top":0,"w":(w-((w/2)|0)),"h":h}];		
						
			break;
		case 2: //horizontal split
			
			rect = [{"left":0,"top":0,"h":((h/2)|0),"w":w},
					{"top":((h/2)|0),"left":0,"h":(h-((h/2)|0)),"w":w}];	
			
			break;
		default:
			Debug.log("Invalid view mode encountered: " + _viewMode);		
		} //end switch
		
		//place all editor components
		for(var i=0;i<2;++i)
		{
			if(!rect[i])
			{
				eps[i].style.display = "none";				
				continue;
			}

			dns[i].style.left 		= (DIR_NAV_MARGIN) + "px";
			dns[i].style.top 		= (DIR_NAV_MARGIN) + "px";
			dns[i].style.width 		= (rect[i].w - 2*DIR_NAV_MARGIN) + "px";
			dns[i].style.height 	= (rect[i].h - 2*DIR_NAV_MARGIN) + "px";

			eps[i].style.left 		= rect[i].left + "px";
			eps[i].style.top 		= rect[i].top + "px";
			eps[i].style.height 	= rect[i].h + "px";
			eps[i].style.width 		= rect[i].w + "px";
			
			epHdrs[i].style.left 	= EDITOR_MARGIN + "px";
			epHdrs[i].style.top 	= (DIR_NAV_MARGIN - 2*EDITOR_MARGIN) + "px";
			epHdrs[i].style.height 	= (EDITOR_HDR_H + 2*EDITOR_MARGIN) + "px";
			epHdrs[i].style.width 	= (rect[i].w - 2*EDITOR_MARGIN) + "px";
			
			//offset body by left and top, but extend to border and allow scroll
			epBdys[i].style.left 	= 0 + "px";
			epBdys[i].style.top 	= (DIR_NAV_MARGIN + EDITOR_HDR_H) + "px";
			epBdys[i].style.height	= (rect[i].h - DIR_NAV_MARGIN - EDITOR_HDR_H) + "px";
			epBdys[i].style.width	= (rect[i].w - 0) + "px";
			
			eps[i].style.display = "block";
		} //end place editor components
		
	} //end redrawWindow()
	

	//=====================================================================================
	//toggleView ~~
	//	does primary only, vertical, or horizontal
	this.toggleView = function(v)
	{
		if(v !== undefined)
			_viewMode = v;
		else		
			_viewMode = (_viewMode+1)%3;
		Debug.log("toggleView _viewMode=" + _viewMode);
		redrawWindow();
	} //end toggleView()

	//=====================================================================================
	//toggleDirectoryNav ~~
	//	toggles directory nav
	this.toggleDirectoryNav = function(forPrimary, v)
	{
		forPrimary = forPrimary?1:0;	
		_activePaneIsPrimary = forPrimary;
		
		Debug.log("toggleDirectoryNav forPrimary=" + forPrimary);

		if(v !== undefined) //if being set, take value
			_navMode[forPrimary] = v?1:0;
		else				//else toggle
			_navMode[forPrimary] = _navMode[forPrimary]?0:1;
		Debug.log("toggleDirectoryNav _navMode=" + _navMode[forPrimary]);
						
		var el = document.getElementById("directoryNav" + forPrimary);
		var wasHidden = el.style.display == "none";
		el.style.display =
				_navMode[forPrimary]?"block":"none";
		
		if(_navMode[forPrimary] && wasHidden)
		{
			var paths = document.getElementById("directoryNav" + 
					forPrimary).getElementsByClassName("dirNavPath");
			var buildPath = "/";
			for(var i=1;i<paths.length;++i)				
				buildPath += (i>1?"/":"") + paths[i].textContent;
			Debug.log("refresh " + buildPath);
			
			CodeEditor.editor.openDirectory(forPrimary,buildPath);
		}
	} //end toggleDirectoryNav()

	//=====================================================================================
	//saveFile ~~
	//	save file for pane
	this.saveFile = function(forPrimary, quiet)
	{
		forPrimary = forPrimary?1:0;
		
		Debug.log("saveFile forPrimary=" + forPrimary);

		Debug.log("saveFile _filePath=" + _filePath[forPrimary]);
		Debug.log("saveFile _fileExtension=" + _fileExtension[forPrimary]);
		
		if(_filePath[forPrimary] == "")
		{
			Debug.log("Error, can not save to empty file name!",
					Debug.HIGH_PRIORITY);
			return;
		}
		
		if(!quiet)
		{
			DesktopContent.popUpVerification(
					"Are you sure you want to save...<br>" + 
					_filePath[forPrimary] + "." + _fileExtension[forPrimary] + "?",
					localDoIt,
					undefined,undefined,undefined,
					undefined,undefined,//val, bgColor, textColor, borderColor, getUserInput, 
					"90%" /*dialogWidth*/
			);
			return;
		}
		else 
			localDoIt();
		
		function localDoIt()
		{		
			var content = encodeURIComponent(
					document.getElementById("editableBox" + forPrimary).innerText);
			//console.log(content,content.length);
			
			//remove crazy characters (or understand where they come from, they seem to be backwards (i.e. 2C and 0A are real characters))
			content = content.replace(/%C2/g,"").replace(/%A0/g,"");
			//console.log(content.length);
			
			
			DesktopContent.XMLHttpRequest("Request?RequestType=codeEditor" + 
					"&option=saveFileContent" +
					"&path=" + _filePath[forPrimary] +
					"&ext=" + _fileExtension[forPrimary]				
					, "content=" + content /* data */,
					function(req)
					{
	
				var err = DesktopContent.getXMLValue(req,"Error"); //example application level error
				if(err) 
				{
					Debug.log(err,Debug.HIGH_PRIORITY);	//log error and create pop-up error box
					return;
				}
	
				Debug.log("Successfully saved " +
						_filePath[forPrimary] + "." + 
						_fileExtension[forPrimary],quiet?Debug.LOW_PRIORITY:Debug.INFO_PRIORITY);
				
				_fileWasModified[forPrimary] = false;
				_fileLastSave[forPrimary] = new Date(); //record last Save time
				//update last save field
				CodeEditor.editor.updateLastSave(forPrimary);
				
				//if other pane is same path and extension, update it too
				if(_filePath[0] == _filePath[1] &&
						_fileExtension[0] == _fileExtension[1])
				{
					Debug.log("Update dual view for save");
					_fileLastSave[(!forPrimary)?1:0] = _fileLastSave[forPrimary];
					_fileWasModified[(!forPrimary)?1:0] = _fileWasModified[forPrimary];
					CodeEditor.editor.updateLastSave(!forPrimary);
				}
	
					}, 0 /*progressHandler*/, 0 /*callHandlerOnErr*/, 1 /*showLoadingOverlay*/);
		} //end localDoIt()
	} //end saveFile()

	//=====================================================================================
	//build ~~
	//	launch compile
	this.build = function(cleanBuild)
	{
		cleanBuild = cleanBuild?1:0;
		
		Debug.log("build cleanBuild=" + cleanBuild);
		
		if(cleanBuild)
		{
			DesktopContent.popUpVerification(
					"Are you sure you want to do a clean build?!",
					localDoIt
					);
					return;
		}
		else 
			localDoIt();
		
		function localDoIt()
		{
			DesktopContent.XMLHttpRequest("Request?RequestType=codeEditor" + 
					"&option=build" +
					"&clean=" + (cleanBuild?1:0)
					, "" /* data */,
					function(req)
					{
	
				var err = DesktopContent.getXMLValue(req,"Error"); //example application level error
				if(err) 
				{
					Debug.log(err,Debug.HIGH_PRIORITY);	//log error and create pop-up error box
					return;
				}
	
				Debug.log("Build was launched! Check " +
						"<a onclick='DesktopContent.openNewBrowserTab(" +
						"\"Console\");' " +
						"title='Click to open the Console web app in a new browser tab.'>" +
						"console</a> for result!", Debug.INFO_PRIORITY);
	
					}, 0 /*progressHandler*/, 0 /*callHandlerOnErr*/, 1 /*showLoadingOverlay*/);
		} //end localDoIt()
		
	} //end build()
	

	//=====================================================================================
	//handleDirectoryContent ~~
	//	redraw directory content based on req response
	this.handleDirectoryContent = function(forPrimary,req)
	{
		forPrimary = forPrimary?1:0;
		
		Debug.log("handleDirectoryContent forPrimary=" + forPrimary);
		console.log(req);

		var path = DesktopContent.getXMLValue(req,"path");
		if(path == "/") path = ""; //default to empty to avoid //
		
		var specials = req.responseXML.getElementsByTagName("special");
		var dirs = req.responseXML.getElementsByTagName("directory");
		var files = req.responseXML.getElementsByTagName("file");
		var specialFiles = req.responseXML.getElementsByTagName("specialFile");

		Debug.log("handleDirectoryContent path=" + path);
		console.log(dirs);console.log(files);
		
		var str = "";
		var i;
		var name;
		str += htmlOpen("div",
				{
			"style":"margin:20px;" +
			"white-space: nowrap;",
				});

		/////////////
		//show path with links
		{
			var pathSplit = path.split('/');
			var buildPath = "";
			var pathSplitName;			

			str += "Path: <a class='dirNavPath' onclick='CodeEditor.editor.openDirectory(" + 
					forPrimary + ",\"" + 
					"/" + "\"" + 
					")'>" + 
					"srcs</a>";
			
			for(i=0;i<pathSplit.length;++i)
			{
				pathSplitName = pathSplit[i].trim();
				if(pathSplitName == "") continue; //skip blanks
				Debug.log("pathSplitName " + pathSplitName);
				
				buildPath += "/" + pathSplitName;
				
				str += "/";
				str += "<a class='dirNavPath' onclick='CodeEditor.editor.openDirectory(" + 
						forPrimary + ",\"" + 
						buildPath + "\"" + 
						")'>" + 
						pathSplitName + "</a>";
				
			}
			str += "/<br><br>";
		}

		/////////////
		//show specials
		for(i=0;i<specials.length;++i)
		{
			name = specials[i].getAttribute('value');

			str += "<a class='dirNavSpecial' onclick='CodeEditor.editor.openDirectory(" + 
					forPrimary + ",\"" + 
					path + "/" + name + "\"" + 
					")'>" + 
					name + "</a>";
			str += "<br>";

		}
		/////////////
		//show special files
		if(specialFiles.length)
		{
			str += "<table>";
			str += "<tr><th>" + path.substr(1,path.length-2) + " Files</th><th style='padding-left:20px'>Repository</th></tr>";
		}
		for(i=0;i<specialFiles.length;++i)
		{
			name = specialFiles[i].getAttribute('value');

			str += "<tr><td>";
			str += "<a class='dirNavFile' onclick='CodeEditor.editor.openFile(" + 
					forPrimary + ",\"" + 
					name + "\",\"" +
					name.substr(name.indexOf('.')+1) + "\"" + //extension
					")' title='srcs" + name + "' >";
			name = name.split('/');			
			str += name[name.length-1] + "</a>";
			str += "</td><td style='padding-left:20px'>" + name[1] + "</td></tr>";

		}
		if(specialFiles.length)
		{
			str += "</table>";
		}
		/////////////
		//show folders
		for(i=0;i<dirs.length;++i)
		{
			name = dirs[i].getAttribute('value');
			
			str += "<a class='dirNavFolder' onclick='CodeEditor.editor.openDirectory(" + 
					forPrimary + ",\"" + 
					path + "/" + name + "\"" + 
					")' title='srcs" + path + "/" + name + "' >" +
					name + "</a>";
			str += "<br>";
					
		}
		/////////////
		//show files
		for(i=0;i<files.length;++i)
		{
			name = files[i].getAttribute('value');
			
			str += "<a class='dirNavFile' onclick='CodeEditor.editor.openFile(" + 
					forPrimary + ",\"" + 
					path + "/" + name + "\", \"" +
					name.substr(name.indexOf('.')+1) + "\"" + //extension
					")' title='srcs" + path + "/" + name + "' >" +
					name + "</a>";
			str += "<br>";
					
		}
		str += "</div>";
		document.getElementById("directoryNav" + forPrimary).innerHTML = str;		
	} //end handleDirectoryContent()

	//=====================================================================================
	//openDirectory ~~
	//	open directory to directory nav
	this.openDirectory = function(forPrimary,path)
	{
		forPrimary = forPrimary?1:0;
		
		Debug.log("openDirectory forPrimary=" + forPrimary +
				" path=" + path);


		DesktopContent.XMLHttpRequest("Request?RequestType=codeEditor" + 
				"&option=getDirectoryContent" +
				"&path=" + path
				, "" /* data */,
				function(req)
				{

			var err = DesktopContent.getXMLValue(req,"Error"); //example application level error
			if(err) 
			{
				Debug.log(err,Debug.HIGH_PRIORITY);	//log error and create pop-up error box
				return;
			}

			CodeEditor.editor.handleDirectoryContent(forPrimary, req);			
			CodeEditor.editor.toggleDirectoryNav(forPrimary,1 /*set nav mode*/);

				}, 0 /*progressHandler*/, 0 /*callHandlerOnErr*/, 1 /*showLoadingOverlay*/);
	} //end openDirectory()

	//=====================================================================================
	//openFile ~~
	//	open the file to text editor
	this.openFile = function(forPrimary,path,extension,doConfirm)
	{
		forPrimary = forPrimary?1:0;
		
		Debug.log("openFile forPrimary=" + forPrimary +
				" path=" + path);
		var i = path.indexOf('.');
		if(i > 0) //up to extension
			path = path.substr(0,i);
		
		if(doConfirm)
		{
			DesktopContent.popUpVerification(
					"Are you sure you want to discard changes and reload file?!",
					localDoIt
			);
			return;
		}
		else localDoIt();

		function localDoIt()
		{
			DesktopContent.XMLHttpRequest("Request?RequestType=codeEditor" + 
					"&option=getFileContent" +
					"&path=" + path + 
					"&ext=" + extension
					, "" /* data */,
					function(req)
					{

				var err = DesktopContent.getXMLValue(req,"Error"); //example application level error
				if(err) 
				{
					Debug.log(err,Debug.HIGH_PRIORITY);	//log error and create pop-up error box
					return;
				}

				CodeEditor.editor.toggleDirectoryNav(forPrimary,0 /*set nav mode*/);
				CodeEditor.editor.handleFileContent(forPrimary, req);			


					}, 0 /*progressHandler*/, 0 /*callHandlerOnErr*/, 1 /*showLoadingOverlay*/);
		} //end localDoIt()
	} //end openFile()
	

	//=====================================================================================
	//handleFileContent ~~
	//	redraw text editor based on file content in req response
	this.handleFileContent = function(forPrimary,req)
	{
		forPrimary = forPrimary?1:0;
		
		Debug.log("handleFileContent forPrimary=" + forPrimary);
		console.log(req);

		var path = DesktopContent.getXMLValue(req,"path");
		var extension = DesktopContent.getXMLValue(req,"ext");
		var text = DesktopContent.getXMLValue(req,"content");
		
		//console.log(text);
		
		_filePath[forPrimary] = path;
		_fileExtension[forPrimary] = extension;
		_fileLastSave[forPrimary] = 0; //default time to 0
		_fileWasModified[forPrimary] = false;

		//set path and extension and last save to header
		var el = document.getElementById("textEditorHeader" + forPrimary);
		
		var str = "";

		//add file name div		
		str += htmlOpen("div",
				{
						"onmousemove" : 
							"CodeEditor.editor.handleFileNameMouseMove(" + forPrimary + ");",
				},0 /*innerHTML*/, false /*doCloseTag*/);
		str += "<center>";
		
		//add rename button		
		str += htmlOpen("div", //this is place holder, that keeps height spacing
				{
						"class":"fileRenameButton",
						"onmousemove": 
							"event.stopPropagation(); " +
							"CodeEditor.editor.handleFileNameMouseMove(" + forPrimary + 
							",1 /*doNotStartTimer*/);",
						"title": "Change the filename...",
						"onclick":
							"CodeEditor.editor.startEditFileName(" + forPrimary + ");",
				},0 /*innerHTML*/, false /*doCloseTag*/);
		str += htmlOpen("div", //this is el that gets hide/show toggle
				{
						"class":"fileRenameButton",
						"id":"fileRenameButton" + forPrimary,
				},0 /*innerHTML*/, true /*doCloseTag*/);
		str += "</div>"; //end fileRenameButton
		
		str += htmlClearDiv();
		
		//add path div
		str += htmlOpen("div",
				{
						"class":"fileNameDiv",
						"id":"fileNameDiv" + forPrimary,
				},0 /*innerHTML*/, false /*doCloseTag*/);
		str += "<a onclick='CodeEditor.editor.openFile(" + forPrimary + 
				",\"" + path + "\",\"" + extension + "\",true /*doConfirm*/);' " +
				"title='Click to reload \n" + path + "." + extension + "' " +
				">" +
				path + "." + extension + "</a>";
		str += "</div>"; //end fileNameDiv
		
		str += "</center>";
		str += "</div>"; //end file name div
		
		
		str += "<div class='textEditorLastSave' id='textEditorLastSave" + 
				forPrimary + "'>Unmodified</div>";
		str += "<div class='textEditorOutline' id='textEditorOutline" + 
				forPrimary + "'>Outline:</div>";
				
		el.innerHTML = str;
		
		var box = document.getElementById("editableBox" + forPrimary);
		box.textContent = text;
		CodeEditor.editor.updateDecorations(forPrimary);		
		
	} //end handleFileContent()
	
	

	//=====================================================================================
	//updateDecorations ~~
	//	redraw text editor based on file content in req response
	var _DECORATION_RED = "rgb(202, 52, 52)";
	var _DECORATION_BLUE = "rgb(64, 86, 206)";
	var _DECORATION_GREEN = "rgb(33, 175, 60)";
	var _DECORATION_BLACK = "rgb(5, 5, 5)";
	var _DECORATION_GRAY = "rgb(162, 179, 158)";
	var _DECORATIONS = {
			"txt": {
				"ADD_SUBDIRECTORY" 		: _DECORATION_RED,
				"include_directories"	: _DECORATION_RED,
				"simple_plugin"			: _DECORATION_RED,
				"set" 					: _DECORATION_RED,
				"install_headers" 		: _DECORATION_RED,
				"install_source"		: _DECORATION_RED,
				"enable_testing"		: _DECORATION_RED,
				"CMAKE_MINIMUM_REQUIRED": _DECORATION_RED,
				"include"				: _DECORATION_RED,
				"create_doxygen_documentation": _DECORATION_RED,
			},
			"c++": {
				"#define" 				: _DECORATION_RED,
				"#undef" 				: _DECORATION_RED,
				"#include" 				: _DECORATION_RED,
				"#ifndef" 				: _DECORATION_RED,
				"#else" 				: _DECORATION_RED,
				"#endif" 				: _DECORATION_RED,
				"using"					: _DECORATION_RED,
				"namespace" 			: _DECORATION_RED,
				"class" 				: _DECORATION_RED,
				"public" 				: _DECORATION_RED,
				"private" 				: _DECORATION_RED,
				"protected"				: _DECORATION_RED,
				"static" 				: _DECORATION_RED,
				"virtual" 				: _DECORATION_RED,
				"override" 				: _DECORATION_RED,
				"const" 				: _DECORATION_RED,
				"void"	 				: _DECORATION_RED,
				"bool"	 				: _DECORATION_RED,
				"unsigned" 				: _DECORATION_RED,
				"int"	 				: _DECORATION_RED,
				"uint64_t" 				: _DECORATION_RED,
				"uint32_t" 				: _DECORATION_RED,
				"uint16_t" 				: _DECORATION_RED,
				"uint8_t" 				: _DECORATION_RED,
				"long"	 				: _DECORATION_RED,
				"float"	 				: _DECORATION_RED,
				"double" 				: _DECORATION_RED,
				"return" 				: _DECORATION_RED,
				"char" 					: _DECORATION_RED,
				"if" 					: _DECORATION_RED,
				"else" 					: _DECORATION_RED,
				"for" 					: _DECORATION_RED,
				"while" 				: _DECORATION_RED,
				"do"	 				: _DECORATION_RED,
				"switch" 				: _DECORATION_RED,
				"case" 					: _DECORATION_RED,
				"default" 				: _DECORATION_RED,
				"try" 					: _DECORATION_RED,
				"catch"					: _DECORATION_RED,
				"this"					: _DECORATION_RED,
				"true"					: _DECORATION_RED,
				"false"					: _DECORATION_RED,
				
				"std::" 				: _DECORATION_BLACK,
				
				"string" 				: _DECORATION_GREEN,
				"set" 					: _DECORATION_GREEN,
				"vector"				: _DECORATION_GREEN,
				"pair"					: _DECORATION_GREEN,
				"get" 					: _DECORATION_GREEN,
				"map" 					: _DECORATION_GREEN,
				"endl" 					: _DECORATION_GREEN,
				"runtime_error"			: _DECORATION_GREEN,
				"memcpy"				: _DECORATION_GREEN,
				"cout"					: _DECORATION_GREEN,						
			},
			"js": {
				"this" 					: _DECORATION_RED,
				"var" 					: _DECORATION_RED,
				"return" 				: _DECORATION_RED,
				"function"				: _DECORATION_RED,
				"if" 					: _DECORATION_RED,
				"else" 					: _DECORATION_RED,
				"for" 					: _DECORATION_RED,
				"while" 				: _DECORATION_RED,
				"do"	 				: _DECORATION_RED,
				"switch" 				: _DECORATION_RED,
				"case" 					: _DECORATION_RED,
				"default" 				: _DECORATION_RED,
				"try" 					: _DECORATION_RED,
				"catch"					: _DECORATION_RED,
				"new" 					: _DECORATION_RED,
				"instanceof" 			: _DECORATION_RED,
				"true"					: _DECORATION_RED,
				"false"					: _DECORATION_RED,

				"Debug"					: _DECORATION_GREEN,
				"DesktopContent"		: _DECORATION_GREEN,
				"HIGH_PRIORITY"			: _DECORATION_GREEN,
				"WARN_PRIORITY"			: _DECORATION_GREEN,
				"INFO_PRIORITY"			: _DECORATION_GREEN,
				"LOW_PRIORITY"			: _DECORATION_GREEN,
				
				"window"				: _DECORATION_GREEN,
				"document"				: _DECORATION_GREEN,
				"textContent"			: _DECORATION_GREEN,
				"innerHTML"				: _DECORATION_GREEN,				
			},
			"sh" : {
				"if" 					: _DECORATION_RED,
				"then" 					: _DECORATION_RED,
				"else" 					: _DECORATION_RED,
				"fi" 					: _DECORATION_RED,
				"for" 					: _DECORATION_RED,
				"in" 					: _DECORATION_RED,
				"while" 				: _DECORATION_RED,
				"do"	 				: _DECORATION_RED,
				"done"	 				: _DECORATION_RED,
				"switch" 				: _DECORATION_RED,
				"case" 					: _DECORATION_RED,
				"default" 				: _DECORATION_RED,
				"export" 				: _DECORATION_RED,
				
				"echo" 					: _DECORATION_GREEN,	
				"cd"					: _DECORATION_GREEN,
				"cp"					: _DECORATION_GREEN,
				"rm"					: _DECORATION_GREEN,
				"cat"					: _DECORATION_GREEN,
				"wget"					: _DECORATION_GREEN,
				"chmod"					: _DECORATION_GREEN,
				"sleep"					: _DECORATION_GREEN,
			}
	};
	this.updateDecorations = function(forPrimary, insertNewLine)
	{	
		forPrimary = forPrimary?1:0;
		
		if(insertNewLine)
			Debug.log("insertNewLine");
		
		//update last save field
		CodeEditor.editor.updateLastSave(forPrimary);
		

		var el = document.getElementById("editableBox" + forPrimary);
		var i, j;
		var val;
		
		//handle get cursor location
		var cursor = {
				"startNodeIndex":undefined,
				"startPos":undefined,				
				"endNodeIndex":undefined,
				"endPos":undefined,
		};
		
		try
		{
			range = window.getSelection().getRangeAt(0);
			
			cursor.startPos = range.startOffset;
			cursor.endPos = range.endOffset;
			
			//find start and end node index
			for(i=0;i<el.childNodes.length;++i)
			{
				if(el.childNodes[i] == range.startContainer ||
						el.childNodes[i] == range.startContainer.parentNode||
						el.childNodes[i] == range.startContainer.parentNode.parentNode)
					cursor.startNodeIndex = i;

				if(el.childNodes[i] == range.endContainer ||
						el.childNodes[i] == range.endContainer.parentNode||
						el.childNodes[i] == range.endContainer.parentNode.parentNode)
					cursor.endNodeIndex = i;
			}

			console.log("cursor",cursor);			
			if(insertNewLine)
			{
				Debug.log("Inserting new line...");
				
				
				//delete all nodes between endNode and startNode
				if(cursor.endNodeIndex > cursor.startNodeIndex)
				{
					//handle end node first, which is a subset effect
					val = el.childNodes[cursor.endNodeIndex].textContent;
					val = val.substr(cursor.endPos);
					el.childNodes[cursor.endNodeIndex].textContent = val;
					--cursor.endNodeIndex;
					while(cursor.endNodeIndex > cursor.startNodeIndex)
					{
						//delete node
						el.removeChild(el.childNodes[cursor.endNodeIndex]);
						--cursor.endNodeIndex;
					}
					//place end pos to delete the remainder of current node
					cursor.endPos = el.childNodes[cursor.startNodeIndex].textContent.length;
				}
				
				val = el.childNodes[cursor.startNodeIndex].textContent;
				val = val.substr(0,cursor.startPos) + '\n' + 
						val.substr(cursor.endPos);
				el.childNodes[cursor.startNodeIndex].textContent = val;
				++cursor.startPos;
				cursor.endNodeIndex = cursor.startNodeIndex;
				cursor.endPos = cursor.startPos;

				console.log("cursor",cursor);			
			}
		}
		catch(err)
		{
			console.log("err",err);
		}


		var n;
		var decor, fontWeight;
		var specialString;
		var commentString = "#";
		if(_fileExtension[forPrimary][0] == 'c' || 
				_fileExtension[forPrimary][0] == 'C' ||
				_fileExtension[forPrimary][0] == 'h' ||
				_fileExtension[forPrimary][0] == 'j')
			commentString = "//"; //comment string
		
		var fileDecorType = "txt";
		if(	_fileExtension[forPrimary] == 'html' ||
				_fileExtension[forPrimary] == 'js')
			fileDecorType = "js"; //js style
		if(_fileExtension[forPrimary][0] == 'c' || 
				_fileExtension[forPrimary][0] == 'C' ||
				_fileExtension[forPrimary][0] == 'h' ||
				_fileExtension[forPrimary][0] == 'j')
			fileDecorType = "c++"; //c++ style
		else if(_fileExtension[forPrimary] == 'sh' || 
				_fileExtension[forPrimary] == 'py')
			fileDecorType = "sh"; //script style
		
		var newNode;
		var node;
		
		var startOfWord = -1;
		var startOfString = -1;
		var startOfComment = -1;
		
		var done = false; //for debuggin
		
		var eatNode;
		var eatVal;
		var closedString;
				
		var prevChar;
		
		/////////////////////////////////
		function localInsertLabel(startPos)
		{
			//split text node into 3 nodes.. text | label | text

			newNode = document.createTextNode(val.substr(0,startPos)); //pre-special text
			el.insertBefore(newNode,node);

			newNode = document.createElement("label");
			newNode.style.fontWeight = fontWeight; //bold or normal
			newNode.style.color = decor;
			newNode.textContent = specialString; //special text
			el.insertBefore(newNode,node);

			node.textContent = val.substr(i); //post-special text



			//handle cursor position update
			if(cursor.startNodeIndex !== undefined)
			{
				if(n < cursor.startNodeIndex)
				{
					//cursor is in a later node
					cursor.startNodeIndex += 2; //two nodes were inserted
					cursor.endNodeIndex += 2; //two nodes were inserted
				}
				else 
				{
					//handle start and stop independently

					//handle start
					if(n == cursor.startNodeIndex)
					{
						//determine if cursor is in one of of new nodes
						if(cursor.startPos < startPos)
						{
							//then in first new element, and 
							// start Node and Pos are still correct
						}
						else if(cursor.startPos < i)
						{
							//then in second new element, adjust Node and Pos
							++cursor.startNodeIndex;
							cursor.startPos -= startPos; 
						}									
						else 
						{
							//then in original last element, adjust Node and Pos
							cursor.startNodeIndex += 2;
							if(val[cursor.startPos-1] == '\r') --cursor.startPos; //get before any \r
							cursor.startPos -= i;
						}
					} //end start handling

					//handle end									
					if(n == cursor.endNodeIndex)
					{
						//determine if cursor is in one of of new nodes
						if(cursor.endPos < startPos)
						{
							//then in first new element, and 
							// end Node and Pos are still correct
						}
						else if(cursor.endPos < i)
						{
							//then in second new element, adjust Node and Pos
							++cursor.endNodeIndex;
							cursor.endPos -= startPos; 
						}									
						else 
						{
							//then in original last element, adjust Node and Pos
							cursor.endNodeIndex += 2;
							if(val[cursor.endPos-1] == '\r') --cursor.endPos; //get before any \r
							cursor.endPos -= i;
							
						}
					}
					else if(n < cursor.endNodeIndex)
					{
						cursor.endNodeIndex += 2; //two new elements were added in front
					}//end end handling
					
				}
			} //end cursor position update

			n += 1; //to return to same modified text node
			
		} //end localInsertLabel
		
		for(n=0;!done && n<el.childNodes.length;++n)
		{		
			node = el.childNodes[n];
			val = node.textContent; //.nodeValue; //.wholeText

			if(node.nodeName == "LABEL")
			{
				//console.log("Label handling...",val);
				
				//if value is no longer a special word, quote, nor comment, then remove label
				if((_DECORATIONS[fileDecorType][val] === undefined && 
						(val[0] != commentString[0] || //break up comment if there is a new line
								val.indexOf('\n') >= 0)	&& 
								val[0] != '"') || 
						//or if cursor is close
						(n+1 >= cursor.startNodeIndex && n-1 <= cursor.endNodeIndex))
				{
					//Debug.log("val lost " + val);

					//add text node and delete node
					newNode = document.createTextNode(val);
					el.insertBefore(newNode,node);
					el.removeChild(node);
					
					//should have no effect on cursor
										
					--n; //revisit this same node for text rules, now that it is not a special label					
					continue;
				}

			}	//end LABEL type handling	
			else if(node.nodeName == "DIV" ||
					node.nodeName == "BR") //adding new lines causes chrome to make DIVs and BRs
			{
				//get rid of divs as soon as possible
				//convert div to text node and then have it reevaluated
				eatVal = node.innerHTML; //if html <br> then add another new line
				//console.log("div/br",eatVal,val);
				
				i = 1;
				if(node.nodeName == "DIV") 	//for DIV there may be more or less new lines to add					
				{							//	depending on previous new line and <br> tag					 
					if(n > 0) //check for \n in previous node
					{						
						specialString = el.childNodes[n-1].textContent;
						if(specialString[specialString.length-1] == '\n')
							--i; //remove a new line, because DIV does not cause a new line if already done
					}
					//check for new line at start of this node
					if(eatVal.indexOf("<br>") == 0 ||
							eatVal[0] == '\n')
						++i;
				}
				
				if(i == 2)
					val = "\n\n" + val;
				else if(i == 1)
					val = "\n" + val;
				//else sometimes i == 0
				
				//add text node and delete node
				newNode = document.createTextNode(val); //add new line to act like div
				el.insertBefore(newNode,node);
				el.removeChild(node);

				//if cursor was here, then advance to account for added newline
				if(n == cursor.startNodeIndex)
					cursor.startPos += i;
				if(n == cursor.endNodeIndex)
					cursor.endPos += i;

				--n; //revisit this same node for text rules	
				continue;
				
			}	//end DIV type handling
			else if(node.nodeName == "#text")
			{
				//merge text nodes
				if(n + 1 < el.childNodes.length &&
						el.childNodes[n+1].nodeName == "#text")
				{
					//Debug.log("Merging nodes at " + n);
					
					
					//merging may have an effect on cursor!
					//handle cursor position update
					if(cursor.startNodeIndex !== undefined)
					{
						if(n+1 < cursor.startNodeIndex)
						{
							//cursor is in a later node
							cursor.startNodeIndex -= 1; //one node was removed
							cursor.endNodeIndex -= 1; //one node was removed
						}
						else 
						{
							//handle start and stop independently

							//handle start
							if(n+1 == cursor.startNodeIndex)
							{
								//then cursor is in second part of merger
								--cursor.startNodeIndex;
								cursor.startPos += val.length;								
							} //end start handling

							//handle end									
							if(n+1 == cursor.endNodeIndex)
							{
								//then cursor is in second part of merger
								--cursor.endNodeIndex;
								cursor.endPos += val.length;	
							}
							else if(n+1 < cursor.endNodeIndex)
							{
								//then cursor is in a later node
								--cursor.endNodeIndex; //one node was removed
							}//end end handling							
						}
					} //end cursor position update

					//place in next node and delete this one
					newNode = el.childNodes[n+1];					
					val += newNode.textContent;
					newNode.textContent = val;
					el.removeChild(node);
					
					--n; //revisit this same node for text rules, now that it is merged
					continue;
				} //end merge text nodes
				
				startOfWord = -1;

				for(i=0;i<val.length;++i)
				{
					
					//for each character:
					//	check if in quoted string
					//	then if in comment
					//	then if special word
					if(startOfComment == -1 && ( //string handling
							startOfString != -1 ||
							(prevChar != '\\' && val[i] == '"')))
					{
						if(startOfString == -1 && val[i] == '"') //start string
							startOfString = i;
						else if(val[i] == '"')	//end string
						{	
							++i; //include " in label
							specialString = val.substr(startOfString,i-startOfString);
							//console.log("string",startOfString,val.length,specialString);
							
							decor = _DECORATION_BLUE;
							fontWeight = "normal";
							localInsertLabel(startOfString);
							startOfString = -1;							
							//done = true; //for debugging
							break;
						}					
					}		
					else if(startOfString == -1 && ( //comment handling
							startOfComment != -1 || 
							(i+commentString.length-1 < val.length && 
									val.substr(i,commentString.length) == 
											commentString)))
					{
						if(startOfComment == -1 && val[i] == commentString[0]) //start comment
							startOfComment = i;
						else if(val[i] == '\n')	//end comment
						{	
							//++i; //do not include \n in label
							specialString = val.substr(startOfComment,i-startOfComment);
							//console.log("comment",startOfComment,val.length,specialString);

							decor = _DECORATION_GRAY;
							fontWeight = "normal";
							localInsertLabel(startOfComment);
							startOfComment = -1;							
							//done = true; //for debugging
							break;
						}					
					}		
					else if(	//special word handling
							(val[i] >= 'a' && val[i] <= 'z') || 
							(val[i] >= 'A' && val[i] <= 'Z') ||
							(val[i] >= '0' && val[i] <= '9') || 
							(val[i] == '_' || val[i] == '-') || 
							val[i] == '#')
					{
						if(startOfWord == -1)
							startOfWord = i;
						//else still within word
					}
					else if(startOfWord != -1) //found end of word, check for special word
					{
						specialString = val.substr(startOfWord,i-startOfWord);						
						decor = _DECORATIONS[fileDecorType][specialString];
						//console.log(specialString);

						if(decor) //found special word
						{
							//console.log(specialString);
							fontWeight = "bold";
							localInsertLabel(startOfWord);
							startOfWord = -1;
							//done = true; //for debugging							
							break;							
						}	
						else
							startOfWord = -1;
					}
					
					prevChar = val[i]; //save previous character (e.g. to check for quote escape)
					
				} //end node string value loop
				

				
				
				
				//////////////
				//if still within string or comment, handle crossing nodes
				if(startOfString != -1 || startOfComment != -1)
				{
					console.log("In string/comment crossing Nodes!");
					//acquire nodes into string until a quote is encountered

					closedString = false;
					for(++n;!closedString && n<el.childNodes.length;++n)
					{		
						eatNode = el.childNodes[n];
						eatVal = eatNode.textContent; //.nodeValue; //.wholeText

						//eat text and delete node
						val += eatVal;
						el.removeChild(eatNode);
						--n; //after removal, move back index for next node
						
						//look for quote close or comment close 
						for(i;i<val.length;++i)
						{
							//string handling
							if(startOfString != -1 && 
									(prevChar != '\\' && val[i] == '"')) //end string
							{
								Debug.log("Closing node crossed string.");
								
								++i; //include " in label
								specialString = val.substr(startOfString,i-startOfString);
								//console.log("string",startOfString,val.length,specialString);

								decor = _DECORATION_BLUE;
								fontWeight = "normal";
								localInsertLabel(startOfString);
								startOfString = -1;							
								closedString = true;
								break;
							}
							
							//comment handling
							if(startOfComment != -1 && val[i] == '\n') //end comment
							{
								Debug.log("Closing node crossed comment.");

								//++i; //do not include \n in label
								
								specialString = val.substr(startOfComment,i-startOfComment);
								//console.log("string",startOfComment,val.length,specialString);

								decor = _DECORATION_GRAY;
								fontWeight = "normal";
								localInsertLabel(startOfComment);
								startOfComment = -1;							
								closedString = true;
								break;

							}
							
							prevChar = val[i]; //save previous character (e.g. to check for quote escape)
						} //end node string value loop
					
					} //end string node crossing node loop
					
					if(!closedString && startOfString != -1)
					{
						Debug.log("String is never closed!");
						specialString = val.substr(startOfString,i-startOfString);
						//console.log("string",startOfString,val.length,specialString);

						decor = _DECORATION_BLUE;
						--n; //move back index (because it was incremented past bounds in end search)
						localInsertLabel(startOfString);
						startOfString = -1;			
					}
					if(!closedString && startOfComment != -1)
					{
						Debug.log("Comment is never closed!");
						specialString = val.substr(startOfComment,i-startOfComment);
						//console.log("string",startOfString,val.length,specialString);

						decor = _DECORATION_GRAY;
						--n; //move back index (because it was incremented past bounds in end search)
						localInsertLabel(startOfComment);
						startOfComment = -1;			
					}
					
					
				} //end crossing nodes with string
				
			} //end #text type node handling
			else
			{
				console.log("unknown node.nodeName",node.nodeName);
				throw("node error!");
			}
		} //end node loop
		
		
		

		//handle set cursor placement
		if(cursor.startNodeIndex !== undefined)
		{
			try
			{
				console.log("cursor",cursor);
				
				range = document.createRange();
				
				var firstEl = el.childNodes[cursor.startNodeIndex];
				if(firstEl.firstChild)
					firstEl = firstEl.firstChild;
	
				var secondEl = el.childNodes[cursor.endNodeIndex];
				if(secondEl.firstChild)
					secondEl = secondEl.firstChild;
				
				range.setStart(firstEl,
						cursor.startPos);
				range.setEnd(secondEl,
						cursor.endPos);
				
				var selection = window.getSelection();
				selection.removeAllRanges();
				selection.addRange(range);
	
			}
			catch(err)
			{
				console.log("err",err);
			}
		} //end set cursor placement	
		
		CodeEditor.editor.updateOutline(forPrimary);
		
		//if other pane is same path and extension, update it too
		if(_filePath[0] == _filePath[1] &&
				_fileExtension[0] == _fileExtension[1])
		{
			Debug.log("Update dual view");
			_fileLastSave[(!forPrimary)?1:0] = _fileLastSave[forPrimary];
			_fileWasModified[(!forPrimary)?1:0] = _fileWasModified[forPrimary];
			CodeEditor.editor.updateLastSave(!forPrimary);
			
			//copy all elements over

			var elAlt = document.getElementById("editableBox" + ((!forPrimary)?1:0));
			elAlt.innerHTML = ""; //clear all children
			for(i=0;i<el.childNodes.length;++i)
			{
				node = el.childNodes[i];
				val = node.textContent;
				if(node.nodeName == "LABEL")
				{
					newNode = document.createElement("label");
					newNode.style.fontWeight = node.style.fontWeight; //bold or normal
					newNode.style.color = node.style.color;
					newNode.textContent = val; //special text
				}
				else if(node.nodeName == "#text")
				{
					newNode = document.createTextNode(val); 					
				}
				else
					Debug.log("Skipping unknown node " + node.nodeName);
				elAlt.appendChild(newNode);
			}
		}
		
	} //end updateDecorations()

	//=====================================================================================
	//updateOutline ~~
	this.updateOutline = function(forPrimary)
	{
		forPrimary = forPrimary?1:0;	
		
		Debug.log("updateOutline " + forPrimary);
		
		var text = document.getElementById("editableBox" + forPrimary).textContent;
		
		var starti;
		var endi;
		var strLength;
		var str;
		var endPi, startCi;
		var newLinei;
		var localNewLineCount;
		var newLineCount = 0;
		var outline = []; //line number and name
		outline.push([1,"Top"]); //always include top
		var i,j;
		var fail;
		
		var isCsource = _fileExtension[forPrimary] == "cc";
		
		for(i=0;i<text.length;++i)
		{
			if(text[i] == '\n') {++newLineCount; continue;}
			
			if(!isCsource) continue; // only look for functions in C++ source code
			
			if(i+1 >= text.length || 
					text[i] != ':' ||
					text[i+1] != ':') continue;
			
			starti = i; //text.indexOf("::",starti)
			endi = text.indexOf('(',starti+3);
			startCi = text.indexOf('{',endi+2);
			endPi = text.lastIndexOf(')',startCi-1);
			
			if(endi < 0 || endPi < 0 || startCi < 0)
			{
				++i; //skip ahead of :'s
				continue; //need all markers
			}
			fail = false;
			
			//consider string without parameters
			str = text.substr(starti,endi-starti) + 
					text.substr(endPi+1,startCi-(endPi+1));			
			//strLength = str.length;
			//str = str.replace(/\s+/g,''); //remove all whitespace
			//console.log(str);

			//console.log("consider ", str);
			
			newLinei = 0;
			localNewLineCount = 0; //count new lines in substr
			for(j=0;!fail && j<str.length;++j)
			{
				if(str[j] == '\n')
					++localNewLineCount;
				else if(str[j] == ';')
					fail = true;
			}
			if(localNewLineCount > 3 || //not too many new lines without params
					fail) //and no ;'s
			{
				++i; //skip ahead of :'s
				continue; //skip non candidates
			} 
			
			//else check parameters
			for(j=endi;!fail && j<endPi;++j)
			{
				if(text[j] == '\n')
					++localNewLineCount;
				else if(text[j] == ';')
					fail = true;
			}
			if(fail)
			{
				++i; //skip ahead of :'s
				continue; //skip non candidates
			} 						
			
			
			//else found a function, record line number and name
			outline.push([newLineCount+1,
						  text.substr(starti+2,endi-starti-2).replace(/\s+/g,'') + 
						  "()"]);						
			//console.log("function", outline[outline.length-1],localNewLineCount);
			
			newLineCount += localNewLineCount;
			i = startCi; //jump past function
			
		} // end text content char loop
		
		Debug.log("Number of lines " + newLineCount);
		console.log("Done with outline", outline);
		
		//handle create line numbers
		str = "";
		for(i=0;i<newLineCount;++i)
		{
			str += "<a name='" + forPrimary + "L" + (i+1) + "'></a>"; //add anchor tag
			str += (i+1);
			str += "<br>";
		}
		document.getElementById("editableBoxLeftMargin" + forPrimary).innerHTML = str;
		
		_numberOfLines[forPrimary] = newLineCount;
		//window.location.href = "#L220";
		
		if(!isCsource)
		{
			//generate simple outline for non C++ source
			i = (newLineCount/2)|0;			
			if(i > 40)
			{
				outline.push([i,"Middle"]);
			}				
		} //end simple outline generate
		outline.push([newLineCount,"Bottom"]); //always include bottom

		//handle create outline
		str = "";
		str += "Outline: ";
		str += htmlOpen("select",
				{
						"class":"textEditorOutlineSelect",
						"id":"textEditorOutlineSelect" + forPrimary,
						"onchange":
							"CodeEditor.editor.handleOutlineSelect(" + forPrimary + ");",
				},0 /*innerHTML*/, false /*doCloseTag*/);
		str += "<option value='0'>Jump to a Line Number (Ctrl + L)</option>"; //blank option
		for(i=0;i<outline.length;++i)
		{
			str += "<option value='" + (outline[i][0]-2) + "'>";
			text = "#" + outline[i][0];
			str += text;
			for(j=text.length;j<12;++j)
				str += "&nbsp;"; //create fixed spacing for name
			str += outline[i][1];
			str += "</option>";							  
		}
		str += "</select>"; //end textEditorOutlineSelect
		document.getElementById("textEditorOutline" + forPrimary).innerHTML = str;
		
	} //end updateOutline()

	//=====================================================================================
	//handleOutlineSelect ~~
	this.handleOutlineSelect = function(forPrimary)
	{
		forPrimary = forPrimary?1:0;
		
		Debug.log("handleOutlineSelect() " + forPrimary);
		
		var val = document.getElementById("textEditorOutlineSelect" + forPrimary).value | 0;
		if(val < 1) val = 1;
		console.log("val",val);		
		
		window.location.href = "#" + forPrimary + "L" + val;
	} //end handleOutlineSelect()
	
	//=====================================================================================
	//keyDownHandler ~~
	var TABKEY = 9;	
	this.keyDownHandler = function(e,forPrimary,shortcutsOnly)
	{
		forPrimary = forPrimary?1:0;
		
		//if just pressing shiftKey ignore
		if(e.keyCode == 16 /*shift*/)
			return;
		

		//to avoid DIVs, ENTER should trigger updateDecorations immediately
		if(!shortcutsOnly &&
				e.keyCode == 13) // ENTER -- should trigger updateDecorations immediately
		{
			_fileWasModified[forPrimary] = true;

			//document.execCommand('insertHTML', false, '&#010;&#013;');
			//document.execCommand('insertText', false, '\n');
			CodeEditor.editor.updateDecorations(forPrimary, true /*insertNewLine*/);
			e.preventDefault();
			
			
			return;
		}
		
		//Debug.log("keydown e=" + e.keyCode + " shift=" + e.shiftKey + " ctrl=" + e.ctrlKey);
		
				
		if(e.ctrlKey) //handle shortcuts
		{			
			if(e.keyCode == 83) 		// S
			{
				CodeEditor.editor.saveFile(forPrimary,true /*quiet*/);
				e.preventDefault();
				return;
			}
			else if(e.keyCode == 68) 	// D
			{
				CodeEditor.editor.toggleDirectoryNav(forPrimary);
				e.preventDefault();
				return;
			}
			else if(e.keyCode == 66) 	// B
			{
				CodeEditor.editor.build();
				e.preventDefault();
				return;
			}
			else if(e.keyCode == 67) 	// C for clean build
			{
				CodeEditor.editor.build(true /*clean*/);
				e.preventDefault();
				return;
			}
		} //end shortcut cases		
		if(shortcutsOnly)		
			return; //if only doing short-cuts, dont handle text
		
		

		var rectangularTAB = false;
		var blockCOMMENT = false;

		if(!e.shiftKey && e.ctrlKey && 
				e.keyCode == 191) 	// ctrl+/ for block comment
			blockCOMMENT = true;
		else if(e.ctrlKey)
		{			

			if(e.keyCode == 84) // T for rectangular TAB
			{
				rectangularTAB = true;
				e.preventDefault();
				//continue to tab handling below
			}	
			else if(e.keyCode == 76 ||	// L or
					e.keyCode == 71) 	// G for go to line number
			{
				DesktopContent.popUpVerification(
						/*prompt*/ "Goto line number: ", 
						/*func*/
						function(line) 
						{
					line = line | 0;
					if(line < 1) line = 1;
					if(line > _numberOfLines[forPrimary])
						line = _numberOfLines[forPrimary];
					Debug.log("Goto line number " + line);
					window.location.href = "#" + forPrimary + "L" + line;
						}, /*val*/ undefined, 
						/*bgColor*/ undefined,
						/*textColor*/ undefined,
						/*borderColor*/ undefined,
						/*getUserInput*/ true,
						/*dialogWidth*/ undefined,
						/*cancelFunc*/ undefined,
						/*yesButtonText*/ "Go");
				
				e.preventDefault();
				return;
			}
			else
				return;
		} //end ctrl key editor handling


				

		if(e.keyCode == TABKEY || rectangularTAB ||
				blockCOMMENT)
		{					
			_fileWasModified[forPrimary] = true;
			CodeEditor.editor.updateLastSave(forPrimary);
			e.preventDefault();

			//manage tabs
			//	if selection, then tab selected lines
			// 	else insert tab character

			//handle get cursor location
			var el = document.getElementById("editableBox" + forPrimary);
			var i;
			var cursor = {
					"startNodeIndex":undefined,
					"startPos":undefined,
					"endNodeIndex":undefined,
					"endPos":undefined,
			};

			try
			{
				range = window.getSelection().getRangeAt(0);

				cursor.startPos = range.startOffset;
				cursor.endPos = range.endOffset;

				//find start and end node index
				for(i=0;i<el.childNodes.length;++i)
				{
					if(el.childNodes[i] == range.startContainer ||
							el.childNodes[i] == range.startContainer.parentNode||
							el.childNodes[i] == range.startContainer.parentNode.parentNode)
						cursor.startNodeIndex = i;

					if(el.childNodes[i] == range.endContainer ||
							el.childNodes[i] == range.endContainer.parentNode||
							el.childNodes[i] == range.endContainer.parentNode.parentNode)
						cursor.endNodeIndex = i;
				}

				console.log("cursor",cursor);
			}
			catch(err)
			{
				console.log("err",err);
			}



			if(cursor.startNodeIndex !== undefined &&
					(cursor.startNodeIndex != cursor.endNodeIndex ||
							cursor.startPos != cursor.endPos))
			{
				//handle tabbing selected lines
				Debug.log("special key selected lines " + cursor.startNodeIndex + " - " +
						cursor.endNodeIndex);					



				/////////////////////////
				//start rectangular tab handling
				if(rectangularTAB)
				{
					Debug.log("Rectangular TAB");

					//steps:
					//	determine x coordinate by 
					//		going backwards from start until a new line is found
					//		and then going forward and counting spots back to cursor
					//	then for each line in selection
					//		add a tab at that x coordinate (offset from newline)

					
					var node,val;
					var found = false;	
					var x = 0;
					var tabSz = 4; //to match eclipse!	
					
					//reverse-find new line
					for(n=cursor.startNodeIndex;n>=0; --n)
					{
						node = el.childNodes[n];
						val = node.textContent; //.nodeValue; //.wholeText

						for(i=(n==cursor.startNodeIndex?cursor.startPos:
								val.length-1); i>=0; --i)
						{
							if(val[i] == '\n')
							{
								//found start of line
								found = true;
								break;
							}
						}
						if(found) break;
					}
					//assume at new line point (or start of file)
					console.log("at leading newline - n",n,"i",i);
					
					//now return to cursor and count spots
					found = false;
					for(n; n<el.childNodes.length; ++n)
					{
						node = el.childNodes[n];
						val = node.textContent; //.nodeValue; //.wholeText

						for(i;i<val.length;++i)
						{
							//exit loop when back to cursor start position
							if(n == cursor.startNodeIndex &&
									i == cursor.startPos)
							{
								found = true;
								
								//insert tab at first position
								var prelength = val.length;

								if(e.shiftKey) //delete leading tab
								{
									if(i-1 >= 0 && val[i-1] == '\t')
										node.textContent = val.substr(0,i-1) + val.substr(i);
								}
								else //add leading tab
								{
									node.textContent = val.substr(0,i) + "\t" + val.substr(i);

								}

								//adjust selection to follow rectangular tabbing
								//	so future rectangular tabbing works as expected
								cursor.startPos += node.textContent.length - prelength;
								break;
							}
								
							//console.log(xcnt," vs ",x,val[i]);
							if(val[i] == '\t')
								x +=  tabSz - (x+tabSz)%tabSz; //jump to next multiple of tabSz
							else 
								++x; //add single character spot
						}
						
						if(found) break;
						
						i = 0; //reset i for next loop
					}
					
//					var firstCharEver = true;
//					for(n=cursor.startNodeIndex; !found && n>=0; --n)
//					{
//						node = el.childNodes[n];
//						val = node.textContent; //.nodeValue; //.wholeText
//
//						for(i=(n==cursor.startNodeIndex?cursor.startPos:
//								val.length-1); i>=0; --i)
//						{	
//							if(firstCharEver) //do the tab for first since we are there
//							{
//								firstCharEver = false;
//
//								var prelength = val.length;
//
//								if(e.shiftKey) //delete leading tab
//								{
//									if(i-1 >= 0 && val[i-1] == '\t')
//										node.textContent = val.substr(0,i-1) + val.substr(i);
//								}
//								else //add leading tab
//								{
//									node.textContent = val.substr(0,i) + "\t" + val.substr(i);
//
//								}
//
//								//adjust selection to follow rectangular tabbing
//								//	so future rectangular tabbing works as expected
//								cursor.startPos += node.textContent.length - prelength;
//								continue;
//							}
//
//							if(val[i] == '\n')
//							{
//								//found start of line
//								found = true;
//								break;
//							}
//							else if(val[i] == '\t')
//								x += tabSz - (x+tabSz)%tabSz; //jump to next multiple of tabSz
//							else 
//								++x; //add single character spot
//						} //end node text character loop
//					} //end node loop

					console.log("x",x);
					

					//fast-forward to endPos through each line and handle tab at x coord								

					var xcnt = -1;
					for(n=cursor.startNodeIndex; n<el.childNodes.length; ++n)
					{
						node = el.childNodes[n];
						val = node.textContent; //.nodeValue; //.wholeText

						for(i=(n==cursor.startNodeIndex?cursor.startPos:
								0);i<val.length;++i)
						{
							//console.log(xcnt," vs ",x,val[i]);
							if(val[i] == '\n')
							{
								//reset x coord count
								xcnt = 0;
							}
							else if(xcnt == x)
							{											
								console.log("x match at ",xcnt,val.substr(0,i),"TTT",val.substr(i));
								xcnt = -1;

								if(e.shiftKey) //delete leading tab
								{
									if(i-1 < val.length && val[i-1] == '\t')
									{
										val = val.substr(0,i-1) + val.substr(i);
										node.textContent = val;
									}
								}
								else //add leading tab
								{
									val = val.substr(0,i) + "\t" + val.substr(i);
									node.textContent = val;
								}

							}
							else if(xcnt != -1) //if counting, increase 
							{
								if(val[i] == '\t')									
									xcnt += tabSz - (xcnt+tabSz)%tabSz; //jump to next multiple of tabSz
								else 
									++xcnt; //add single character spot
							}									
						} //end node text character loop

						if(n == cursor.endNodeIndex)
							break; //reached end of selection
					} //end node loop



					//need to set cursor
					try
					{
						var range = document.createRange();
//						range.setStart(el.childNodes[cursor.startNodeIndex],cursor.startPos);
//						range.setEnd(el.childNodes[cursor.endNodeIndex],cursor.endPos);
						var firstEl = el.childNodes[cursor.startNodeIndex];
						if(firstEl.firstChild)
							firstEl = firstEl.firstChild;

						var secondEl = el.childNodes[cursor.endNodeIndex];
						if(secondEl.firstChild)
							secondEl = secondEl.firstChild;

						range.setStart(firstEl,
								cursor.startPos);
						range.setEnd(secondEl,
								cursor.endPos);

						var selection = window.getSelection();
						selection.removeAllRanges();
						selection.addRange(range);
					}
					catch(err)
					{
						console.log(err);
						return;
					}

					return;
				} //end rectangular TAB handling





				/////////////////////////
				// normal block TAB handling
				//steps:
				//	go backwards from start until a new line is found
				//	then add tab after each new line until end of selection reached

				//reverse-find new line
				var node,val;
				var found = false;	
				var specialStr = '\t';
				if(blockCOMMENT)
				{
					if(_fileExtension[forPrimary][0] == 'c' || 
							_fileExtension[forPrimary][0] == 'C' ||
							_fileExtension[forPrimary][0] == 'h' ||
							_fileExtension[forPrimary][0] == 'j')
						specialStr = "//"; //comment string
					else
						specialStr = "#"; //comment string
				}

				for(n=cursor.startNodeIndex; !found && n>=0; --n)
				{
					node = el.childNodes[n];
					val = node.textContent; //.nodeValue; //.wholeText

					for(i=(n==cursor.startNodeIndex?cursor.startPos:
							val.length-1); i>=0; --i)
					{
						if(val[i] == '\n')
						{
							if(e.shiftKey) //delete leading special string
							{
								if(i + specialStr.length < val.length &&
										val.indexOf(specialStr,i+1) == i+1)
									node.textContent = val.substr(0,i+1) + 
									val.substr(i+1+specialStr.length);											
							}
							else //add leading special string
								node.textContent = val.substr(0,i+1) + 
								specialStr + val.substr(i+1);
							found = true;
							break;
						}
					} //end node text character loop
				} //end node loop

				//fast-forward to endPos and insert tab after each new line encountered
				found = false;
				var prevCharIsNewLine = false;
				for(n=cursor.startNodeIndex; !found && n<el.childNodes.length &&
				n <= cursor.endNodeIndex; ++n)
				{								
					node = el.childNodes[n];
					val = node.textContent; //.nodeValue; //.wholeText

					for(i=(n==cursor.startNodeIndex?cursor.startPos:
							0);i<val.length;++i)
					{
						if(n == cursor.endNodeIndex && i >= cursor.endPos)
						{
							//reached end of selection
							found = true;
							break;
						}

						if(val[i] == '\n' || 
								(i == 0 && prevCharIsNewLine))
						{
							if(i == 0 && prevCharIsNewLine) --i; //so that tab goes in the right place

							if(e.shiftKey) //delete leading tab
							{											
								if(i + specialStr.length < val.length &&
										val.indexOf(specialStr,i+1) == i+1)
								{
									val = val.substr(0,i+1) + 
											val.substr(i+1+specialStr.length);
									node.textContent = val;

									//if running out of string to keep line selected.. jump to next node
									//	with selection
									if(n == cursor.endNodeIndex && 
											cursor.endPos >= val.length)
									{
										++cursor.endNodeIndex;
										cursor.endPos = 0;
									}
								}
							}
							else //add leading tab
							{
								val = val.substr(0,i+1) + specialStr + val.substr(i+1);
								node.textContent = val;
							}

							if(i == -1 && prevCharIsNewLine) ++i; //so that loop continues properly
						}									
					} //end node text character loop

					prevCharIsNewLine = (val.length && //handle last char newline case
							val[val.length-1] == '\n');
				} //end node loop

				//need to set cursor
				try
				{
					var range = document.createRange();
//					range.setStart(el.childNodes[cursor.startNodeIndex],cursor.startPos);
//					range.setEnd(el.childNodes[cursor.endNodeIndex],cursor.endPos);

					var firstEl = el.childNodes[cursor.startNodeIndex];
					if(firstEl.firstChild)
						firstEl = firstEl.firstChild;

					var secondEl = el.childNodes[cursor.endNodeIndex];
					if(secondEl.firstChild)
						secondEl = secondEl.firstChild;

					range.setStart(firstEl,
							cursor.startPos);
					range.setEnd(secondEl,
							cursor.endPos);

					var selection = window.getSelection();
					selection.removeAllRanges();
					selection.addRange(range);
				}
				catch(err)
				{
					console.log(err);
					return false;
				}
			}
			else if(!blockCOMMENT) //not tabbing a selection, just add or delete single tab in place
			{	
				if(e.shiftKey)
				{								
					if(cursor.startNodeIndex !== undefined)
					{
						try
						{
							var node,val,i;
							i = cursor.startPos;
							node = el.childNodes[cursor.startNodeIndex];
							val = node.textContent; //.nodeValue; //.wholeText
							console.log(node,val,val[i-1]);

							if(val[i-1] == '\t')
							{
								node.textContent = val.substr(0,i-1) + val.substr(i);

								//need to set cursor
								var range = document.createRange();
								range.setStart(node,i-1);
								range.setEnd(node,i-1);

								var selection = window.getSelection();
								selection.removeAllRanges();
								selection.addRange(range);
							}
						}
						catch(err)
						{
							console.log(err);
							return;
						}
					}
					else
						Debug.log("No cursor for reverse tab.");
				}
				else
					document.execCommand('insertHTML', false, '&#009');
			}
			
			//set timeout for decoration update
			window.clearTimeout(_inputTimerHandle);
			_inputTimerHandle = window.setTimeout(
					function()
					{
				CodeEditor.editor.updateDecorations(forPrimary);				
					}, 1000); //end setTimeout

			return;

		} //end handle tab key

	} //end keyDownHandler()

	//=====================================================================================
	//updateLastSave ~~
	//	update display based on lastSave and wasModified member variables
	this.updateLastSave = function(forPrimary)
	{
		forPrimary = forPrimary?1:0;
		
		var el = document.getElementById("textEditorLastSave" + forPrimary);
		if(!el) return; //if not displayed, quick exit
		
		Debug.log("updateLastSave()");
		var str = "";
		if(_fileWasModified[forPrimary])
			str += "<label style='color:red'>Unsaved changes!</label> ";
		else
			str += "Unmodified. ";
		if(_fileLastSave[forPrimary])
		{
			var now = new Date();
			var d = new Date(_fileLastSave[forPrimary]);				
			var tstr = d.toLocaleTimeString();
			tstr = tstr.substring(0,tstr.lastIndexOf(' ')) + //convert AM/PM to am/pm with no space
					(tstr[tstr.length-2]=='A'?"am":"pm");
			
			var diff = ((now.getTime() - d.getTime())/1000)|0; //in seconds				
			var diffStr = "";
			
			if(diff < 5)
				diffStr = "(just now) ";
			else if(diff < 10)
				diffStr = "(5 seconds ago) ";
			else if(diff < 20)
				diffStr = "(15 seconds ago) ";
			else if(diff < 40)
				diffStr = "(30 seconds ago) ";
			else if(diff < 50)
				diffStr = "(45 seconds ago) ";
			else if(diff < 120)
				diffStr = "(one minute ago) ";
			else if(diff < 15*60) 
				diffStr = "(" + ((diff/60)|0) + " minutes ago) ";
			else if(diff < 20*60) //about 15 minutes
				diffStr = "(15 minutes ago) ";
			else if(diff < 40*60) //about 30 minutes
				diffStr = "(30 minutes ago) ";
			else if(diff < 50*60) //about 45 minutes
				diffStr = "(45 minutes ago) ";
			else if(diff < 90*60) //about an hour
				diffStr = "(an hour ago) ";
			else	//hours
				diffStr = "(" + (Math.round(diff/60/60)) + " hours ago) ";
				
			
			str += "Last save was " + diffStr + tstr;
		} 
		el.innerHTML = str;			
	} //end updateLastSave()
	
	
	//=====================================================================================
	//handleFileNameMouseMove ~~
	this.handleFileNameMouseMove = function(forPrimary,doNotStartTimer)
	{
		forPrimary = forPrimary?1:0;
		
		//console.log("handleFileNameMouseMove " + forPrimary + " - " + doNotStartTimer);
		
		if(_fileNameEditing[forPrimary]) return;
		
		var el = document.getElementById("fileRenameButton" + forPrimary);
		el.style.display = "block";
		
		window.clearTimeout(_fileNameMouseMoveTimerHandle);
		
		if(doNotStartTimer) return;
		
		_fileNameMouseMoveTimerHandle = window.setTimeout(
				function()
				{
			el.style.display = "none";
				} //end mouse move timeout handler
				,1000);

	} //end handleFileNameMouseMove()
	
	//=====================================================================================
	//startEditFileName ~~
	this.startEditFileName = function(forPrimary)
	{
		forPrimary = forPrimary?1:0;
		
		if(_fileNameEditing[forPrimary]) return;
		_fileNameEditing[forPrimary] = true;
	
		//hide edit button
		document.getElementById("fileRenameButton" + forPrimary).style.display = "none";
		
		
		console.log("startEditFileName " + forPrimary);

		var el = document.getElementById("fileNameDiv" + forPrimary);
		var initVal = el.textContent.trim();

		var _OK_CANCEL_DIALOG_STR = "";

		_OK_CANCEL_DIALOG_STR += "<div title='' style='padding:5px;background-color:#eeeeee;border:1px solid #555555;position:relative;z-index:2000;" + //node the expander nodes in tree-view are z-index:1000  
				"width:105px;height:20px;margin: 4px -122px -32px -120px; font-size: 16px; white-space:nowrap; text-align:center;'>";
		_OK_CANCEL_DIALOG_STR += "<a class='popUpOkCancel' onclick='" +
				"CodeEditor.editor.editCellOK(" + forPrimary +
				"); event.stopPropagation();' onmouseup='event.stopPropagation();' title='Accept Changes' style='color:green'>" +
				"<b style='color:green;font-size: 16px;'>OK</b></a> | " +
				"<a class='popUpOkCancel' onclick='" +
				"CodeEditor.editor.editCellCancel(" + forPrimary +
				"); event.stopPropagation();' onmouseup='event.stopPropagation();' title='Discard Changes' style='color:red'>" + 
				"<b style='color:red;font-size: 16px;'>Cancel</b></a>";
		_OK_CANCEL_DIALOG_STR += "</div>";
		
		//create input box and ok | cancel
		var str = "";
		str += "<input type='text' style='text-align:center;margin:-4px -2px -4px -1px;width:90%;" + 
				" height:" + (el.offsetHeight>20?el.offsetHeight:20) + "px' value='";
		str += initVal;
		str += "' >";
		str += _OK_CANCEL_DIALOG_STR;	

		el.innerHTML = str;
		
		//select text in new input
		el = el.getElementsByTagName("input")[0];
		el.focus();
		var startPos = initVal.lastIndexOf('/')+1;
		var endPos = initVal.lastIndexOf('.');
		if(endPos < 0) endPos = initVal.length;
		el.setSelectionRange(startPos, endPos);
		
	} //end startEditFileName()
	
	//=====================================================================================
	//editCellOK ~~
	this.editCellOK = function(forPrimary)
	{
		forPrimary = forPrimary?1:0;
		
		var val = document.getElementById("fileNameDiv" + forPrimary).getElementsByTagName("input")[0].value;
		console.log("editCellOK " + forPrimary + " = " + val);
		_fileNameEditing[forPrimary] = false;		

		var extPos = val.lastIndexOf('.');
		
		//set path and extension
		
		_filePath[forPrimary] = val.substr(0,extPos);
		_fileExtension[forPrimary] = extPos > 0?val.substr(extPos+1):"";
				
		var el = document.getElementById("fileNameDiv" + forPrimary);
		
		var str = "";
		str += "<a onclick='CodeEditor.editor.openFile(" + forPrimary + 
				",\"" + _filePath[forPrimary] + "\",\"" + _fileExtension[forPrimary] + "\",true /*doConfirm*/);'>" +
				_filePath[forPrimary] + "." + _fileExtension[forPrimary] + "</a>";
		
		el.innerHTML = str;

		//indicate file was not saved
		_fileWasModified[forPrimary] = true;
		_fileLastSave[forPrimary] = 0; //reset
		CodeEditor.editor.updateLastSave(forPrimary);

	} //end editCellOK()
	
	//=====================================================================================
	//editCellCancel ~~
	this.editCellCancel = function(forPrimary)
	{
		forPrimary = forPrimary?1:0;
		
		console.log("editCellCancel " + forPrimary);
		_fileNameEditing[forPrimary] = false;

		//revert to same path and extension
		var el = document.getElementById("fileNameDiv" + forPrimary);
		
		var str = "";
		str += "<a onclick='CodeEditor.editor.openFile(" + forPrimary + 
				",\"" + _filePath[forPrimary] + "\",\"" + _fileExtension[forPrimary] + "\",true /*doConfirm*/);'>" +
				_filePath[forPrimary] + "." + _fileExtension[forPrimary] + "</a>";

		el.innerHTML = str;

	} //end editCellCancel()
	
} //end create() CodeEditor instance











