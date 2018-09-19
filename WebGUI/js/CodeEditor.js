


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

	//functions:			
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
	//	openFile(forPrimary,path,extension)
	//	handleFileContent(forPrimary,req)
	//	updateDecorations(forPrimary,inPlace)
	
	
	//for display
	var _CHECKBOX_H = 40;
	var _CHECKBOX_MIN_W = 240;
	var _CHECKBOX_MAX_W = 540;
	var _WINDOW_MIN_SZ = 525;
	var _MARGIN = 40;


	var _needEventListeners = true;
	
	var _viewMode = 0; //0: only primary, 1: vertical split, 2: horizontal split
	var _navMode = [0,0]; //1 for showing directory nav
	var _filePath = ["",""]; //file path for primary and secondary
	var _fileExtension = ["",""]; //file extension for primary and secondary

	
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
				"/otsdaq/otsdaq-core/CoreSupervisors/version.h",
								  //"/CMakeLists.txt", //DesktopContent.getParameter(0,"startFilePrimary"),
		   DesktopContent.getParameter(0,"startFileSecondary")
		   ];
		var parameterViewMode = DesktopContent.getParameter(0,"startViewMode");
		if(parameterViewMode !== undefined) //set view mode if parameter
		{
			_viewMode = parameterViewMode|0;
		}
		console.log("parameterStartFile",parameterStartFile);
		console.log("parameterViewMode",parameterViewMode);
		
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
			//primaryPane div
			el = document.createElement("div");	
			el.setAttribute("class","editorPane");	
			{
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
								},"" /*innerHTML*/, 0 /*doCloseTag*/);
						{
							str += htmlOpen("div",
									{
											"style":"margin:11px 0 0 12px;",
									},"D" /*innerHTML*/, 1 /*doCloseTag*/);
						} //end directoryNavToggle
						str += "</div>"; //close directoryNavToggle
		
						//save
						str += htmlOpen("div",
								{
										"id":"saveFile",
										"class":"controlsButton",
										"style":"float:left;",
										"onclick":"CodeEditor.editor.saveFile(" + forPrimary + ");",
								},"" /*innerHTML*/, 0 /*doCloseTag*/);
						{
							str += htmlOpen("div",
									{
											"style":"margin:11px 0 0 12px;",
									},"S" /*innerHTML*/, 1 /*doCloseTag*/);
						} //end directoryNavToggle
						str += "</div>"; //close saveFile
						
					} //end locals controlsPane
					str += "</div>"; //close controlsPane
					return str;
				} //end localCreatePaneControls
				
				var str = "";
				str += createTextEditor(1 /*forPrimary*/);
				str += createDirectoryNav(1 /*forPrimary*/);
				str += localCreatePaneControls(1 /*forPrimary*/);				
				el.innerHTML = str;
			}				
			cel.appendChild(el);
			
			//================
			//secondaryPane div
			el = document.createElement("div");
			el.setAttribute("class","editorPane");	

			var str = "";
			str += createTextEditor(0 /*forPrimary*/);
			str += createDirectoryNav(0 /*forPrimary*/);
			str += localCreatePaneControls(0 /*forPrimary*/);	
			el.innerHTML = str;
			cel.appendChild(el);
			
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
			
		}					
		
		document.body.appendChild(cel);		
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
						"style":"overflow:auto;",
				},"Text" /*innerHTML*/, 1 /*doCloseTag*/);
		
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
		var dns = document.getElementsByClassName("directoryNav");
		
		eps[0].style.position = "absolute";
		eps[1].style.position = "absolute";
			
		var DIR_NAV_MARGIN = 50;
		switch(_viewMode)
		{
		case 0: //only primary
			eps[0].style.left 		= 0 + "px";
			eps[0].style.top 		= 0 + "px";
			eps[0].style.height 	= h + "px";
			eps[0].style.width 		= w + "px";	

			dns[0].style.left 	= DIR_NAV_MARGIN + "px";
			dns[0].style.top 	= DIR_NAV_MARGIN + "px";
			dns[0].style.width 	= (w - 2*DIR_NAV_MARGIN) + "px";
			dns[0].style.height = (h - 2*DIR_NAV_MARGIN) + "px";
			
			eps[1].style.display = "none";
			break;
		case 1: //vertical split
			eps[0].style.left 		= 0 + "px";
			eps[0].style.top 		= 0 + "px";
			eps[0].style.height 	= h + "px";
			eps[0].style.width 		= ((w/2)|0) + "px";	

			dns[0].style.left 	= DIR_NAV_MARGIN + "px";
			dns[0].style.top 	= DIR_NAV_MARGIN + "px";
			dns[0].style.width 	= (((w/2)|0) - 2*DIR_NAV_MARGIN) + "px";
			dns[0].style.height = (h - 2*DIR_NAV_MARGIN) + "px";
			
			eps[1].style.left 		= ((w/2)|0) + "px";			
			eps[1].style.top 		= 0 + "px";
			eps[1].style.height 	= h + "px";
			w -= ((w/2)|0);
			eps[1].style.width 		= w + "px";	

			dns[1].style.left 	= DIR_NAV_MARGIN + "px";
			dns[1].style.top 	= DIR_NAV_MARGIN + "px";
			dns[1].style.width 	= (w - 2*DIR_NAV_MARGIN) + "px";
			dns[1].style.height = (h - 2*DIR_NAV_MARGIN) + "px";
			
			eps[1].style.display = "block";
			break;
		case 2: //horizontal split
			eps[0].style.left 		= 0 + "px";
			eps[0].style.top 		= 0 + "px";
			eps[0].style.height 	= ((h/2)|0) + "px";
			eps[0].style.width 		= w + "px";	

			dns[0].style.left 	= DIR_NAV_MARGIN + "px";
			dns[0].style.top 	= DIR_NAV_MARGIN + "px";
			dns[0].style.width 	= (w - 2*DIR_NAV_MARGIN) + "px";
			dns[0].style.height = (((h/2)|0) - 2*DIR_NAV_MARGIN) + "px";
			
			eps[1].style.left 		= 0 + "px";			
			eps[1].style.top 		= ((h/2)|0) + "px";
			h -= ((h/2)|0);
			eps[1].style.height 	= h + "px";
			eps[1].style.width 		= w + "px";

			dns[1].style.left 	= DIR_NAV_MARGIN + "px";
			dns[1].style.top 	= DIR_NAV_MARGIN + "px";
			dns[1].style.width 	= (w - 2*DIR_NAV_MARGIN) + "px";
			dns[1].style.height = (h - 2*DIR_NAV_MARGIN) + "px";
			
			eps[1].style.display = "block";
			break;
		default:
			Debug.log("Invalid view mode encountered: " + _viewMode);		
		}		
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
		Debug.log("toggleDirectoryNav forPrimary=" + forPrimary);

		if(v !== undefined) //if being set, take value
			_navMode[forPrimary] = v?1:0;
		else				//else toggle
			_navMode[forPrimary] = _navMode[forPrimary]?0:1;
		Debug.log("toggleDirectoryNav _navMode=" + _navMode[forPrimary]);
						
		document.getElementById("directoryNav" + forPrimary).style.display =
				_navMode[forPrimary]?"block":"none";				
	} //end toggleDirectoryNav()

	//=====================================================================================
	//saveFile ~~
	//	save file for pane
	this.saveFile = function(forPrimary)
	{
		forPrimary = forPrimary?1:0;
		Debug.log("saveFile forPrimary=" + forPrimary);

		Debug.log("saveFile _filePath=" + _filePath[forPrimary]);
		Debug.log("saveFile _fileExtension=" + _fileExtension[forPrimary]);
		
		var content = encodeURIComponent(
				document.getElementById("editableBox" + forPrimary).innerText);
		console.log(content);
		
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
					_fileExtension[forPrimary], Debug.INFO_PRIORITY);

				}, 0 /*progressHandler*/, 0 /*callHandlerOnErr*/, 1 /*showLoadingOverlay*/);

	} //end saveFile()

	//=====================================================================================
	//build ~~
	//	launch compile
	this.build = function(cleanBuild)
	{
		cleanBuild = cleanBuild?1:0;
		Debug.log("build cleanBuild=" + cleanBuild);
		
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

			Debug.log("Check console for result!", Debug.INFO_PRIORITY);

				}, 0 /*progressHandler*/, 0 /*callHandlerOnErr*/, 1 /*showLoadingOverlay*/);

		
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
		var dirs = req.responseXML.getElementsByTagName("directory");
		var files = req.responseXML.getElementsByTagName("file");

		Debug.log("handleDirectoryContent path=" + path);
		console.log(dirs);console.log(files);
		
		var str = "";
		var i;
		var name;
		str += htmlOpen("div",
				{
			"style":"margin:20px;" 
				,
				});

		/////////////
		//show path with links
		{
			var pathSplit = path.split('/');
			var buildPath = "";
			var pathSplitName;			

			str += "<a class='dirNavPath' onclick='CodeEditor.editor.openDirectory(" + 
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
			str += "<br><br>";
		}
		
		/////////////
		//show folders
		for(i=0;i<dirs.length;++i)
		{
			name = dirs[i].getAttribute('value');
			
			str += "<a class='dirNavFolder' onclick='CodeEditor.editor.openDirectory(" + 
					forPrimary + ",\"" + 
					path + "/" + name + "\"" + 
					")'>" + 
					name + "</a>";
			str += "<br>";
					
		}/////////////
		//show files
		for(i=0;i<files.length;++i)
		{
			name = files[i].getAttribute('value');
			
			str += "<a class='dirNavFile' onclick='CodeEditor.editor.openFile(" + 
					forPrimary + ",\"" + 
					path + "/" + name + "\", \"" +
					name.substr(name.indexOf('.')+1) + "\"" + //extension
					")'>" + 
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
	this.openFile = function(forPrimary,path,extension)
	{
		forPrimary = forPrimary?1:0;
		Debug.log("openFile forPrimary=" + forPrimary +
				" path=" + path);
		var i = path.indexOf('.');
		if(i > 0) //up to extension
			path = path.substr(0,i);
		
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

			CodeEditor.editor.handleFileContent(forPrimary, req);			
			CodeEditor.editor.toggleDirectoryNav(forPrimary,0 /*set nav mode*/);
			
			
				}, 0 /*progressHandler*/, 0 /*callHandlerOnErr*/, 1 /*showLoadingOverlay*/);
	} //end openFile()
	

	//=====================================================================================
	//handleFileContent ~~
	//	redraw text editor based on file content in req response
	var TABKEY = 9;
	this.handleFileContent = function(forPrimary,req)
	{
		forPrimary = forPrimary?1:0;
		Debug.log("handleFileContent forPrimary=" + forPrimary);
		console.log(req);

		var path = DesktopContent.getXMLValue(req,"path");
		var extension = DesktopContent.getXMLValue(req,"ext");
		var text = DesktopContent.getXMLValue(req,"content");
		
		console.log(text);
		
		_filePath[forPrimary] = path;
		_fileExtension[forPrimary] = extension;
		
		var el = document.getElementById("textEditor" + forPrimary);
		
		var str = "";
		
		str += htmlOpen("div",
				{
						"class":"editableBox",
						"id":"editableBox" + forPrimary,
						"style":"margin: 50px 20px 20px 20px;",	
						"contenteditable":"true",
				});		
		//str += localConvertForClient(text);
		str += "</div>"; //close editableBox
		
		el.innerHTML = str;
		
		var box = document.getElementById("editableBox" + forPrimary);
		box.textContent = text;
		CodeEditor.editor.updateDecorations(forPrimary);	

		var inputTimer = 0;
		box.addEventListener("input",
				function(e)
				{
			Debug.log("input forPrimary=" + forPrimary);
			window.clearTimeout(inputTimer);
			inputTimer = window.setTimeout(
					function()
					{
				CodeEditor.editor.updateDecorations(forPrimary,1 /*inPlace*/);				
					}, 1000); //end setTimeout
				}); //end addEventListener
		
		box.addEventListener("keydown",
						function(e)
						{
					Debug.log("keydown e=" + e.keyCode);
					
					window.clearTimeout(inputTimer);
					inputTimer = window.setTimeout(
							function()
							{
						CodeEditor.editor.updateDecorations(forPrimary,1 /*inPlace*/);				
							}, 1000); //end setTimeout
										
					
					if(e.keyCode == TABKEY)
					{
						//this.value
						//manage tabs
						document.execCommand('insertHTML', false, '&#009');

						e.preventDefault();
						return;
						
						
						range= window.getSelection().getRangeAt(0);
						start = range.startOffset;
						end = range.endOffset;
						startNode = range.startContainer;
						endNode = range.endContainer;
						selectedText = range.toString();
						
						var val = startNode.textContent;
						console.log(val);
						
						startNode.textContent = val.substr(0,start) +
								"T" + val.substr(start);
						
						
					
						
						//var startPos = box.selectionStart;
						//var endPos = box.selectionEnd;
						//console.log("startPos",startPos,endPos,localGetCaretPosition(box));
						console.log("sel.getRangeAt(0)",range);
						var startPos = range.startOffset;
						var endPos = range.endOffset;
						e.preventDefault();
					}
					
						}); //end addEventListener
		
		//////////////////
		//localConvertForClient
		//	replace html/xhtml reserved characters with equivalent for client display.
		//	reserved: ", ', &, <, >, newline, space
		function localConvertForClient(s)
		{
			//steps:
			//	convert tabs
			//
			
			console.log(s);
			
			//is 8 spaces on linux console - do the same
			var t;
			var s;
			var i,c;
			var tabSz = 8;
			while((t = str.indexOf("\t")) != -1)
			{
				s = ""; //determine the number of spaces to represent the tab as
				c = (tabSz-(t%tabSz)); 
				if(c<=1) c+=tabSz; //make sure tab space is > 1
				for(i=0;i<c;++i)
					s += "&nbsp;";
				str = [str.slice(0, t), s, str.slice(t+1)].join('');
			}

//			s = encodeURIComponent(s);			
//			console.log(s);
//			s = s
//				.replace(/%23/g, "&amp;")  			//&					
//				.replace(/%26/g, "&amp;")  			//&
//				.replace(/%3C/g, "&lt;") 				//<
//				.replace(/%3E/g, "&gt;")				//>
//				.replace(/%22/g, "&quot;")				//"
//				.replace(/%27/g, "&#039;")				//'
//				.replace(/%0A%0D/g, "<br>")			//newline
//				.replace(/%20%20/g, "&nbsp;&nbsp;")		//double space
//				.replace(/%20/g, " ");	//space
//
//			console.log(s);
			return s;
		} // end localDecorateText
		
		
		
		function localGetCaretPosition(editableDiv) 
		{
			var caretPos = 0,
					sel, range;
			if (window.getSelection) 
			{
				sel = window.getSelection();
				if (sel.rangeCount) 
				{
					range = sel.getRangeAt(0);
					if (range.commonAncestorContainer.parentNode == editableDiv) 
					{
						caretPos = range.endOffset;
					}
				}
			} 
			else if (document.selection && document.selection.createRange) 
			{
				range = document.selection.createRange();
				if (range.parentElement() == editableDiv) 
				{
					var tempEl = document.createElement("span");
					editableDiv.insertBefore(tempEl, editableDiv.firstChild);
					var tempRange = range.duplicate();
					tempRange.moveToElementText(tempEl);
					tempRange.setEndPoint("EndToEnd", range);
					caretPos = tempRange.text.length;
				}
			}
			return caretPos;
		}
		
	} //end handleFileContent()
	
	

	//=====================================================================================
	//updateDecorations ~~
	//	redraw text editor based on file content in req response
	var _DECORATION_RED = "rgb(202, 52, 52)";
	var _DECORATION_BLUE = "rgb(64, 86, 206)";
	var _DECORATION_GREEN = "rgb(33, 175, 60)";
	var _DECORATION_BLACK = "rgb(5, 5, 5)";
	var _DECORATIONS = {
			"ADD_SUBDIRECTORY" 	: _DECORATION_BLUE,
			"namespace" 		: _DECORATION_RED,
			"const" 			: _DECORATION_RED,
			"#define" 			: _DECORATION_RED,
			"#undef" 			: _DECORATION_RED,
			"#include" 			: _DECORATION_RED,
			"#ifndef" 			: _DECORATION_RED,
			"#else" 			: _DECORATION_RED,
			"#endif" 			: _DECORATION_RED,
			"void"	 			: _DECORATION_RED,
			"std::" 			: _DECORATION_BLACK,
			"string" 			: _DECORATION_GREEN,
			"set" 				: _DECORATION_GREEN,
			"get" 				: _DECORATION_GREEN,
			"map" 				: _DECORATION_GREEN,
	};
	this.updateDecorations = function(forPrimary,inPlace)
	{	

		var el = document.getElementById("editableBox" + forPrimary);
		
		//handle get cursor location
		var cursor = {
				"startNode":undefined,
				"startNodeIndex":undefined,
				"startPos":undefined,
				"endNode":undefined,
				"endNodeIndex":undefined,
				"endPos":undefined,
				//maybe not needed
				"rangeValue":undefined,
				"startNodeValue": undefined,
				"endNodeValue": undefined,
		};
		
		try
		{
			range = window.getSelection().getRangeAt(0);
			
			cursor.startPos = range.startOffset;
			cursor.endPos = range.endOffset;
			cursor.startNode = range.startContainer;
			cursor.endNode = range.endContainer;
			cursor.rangeValue = range.toString();
			cursor.startNodeValue = cursor.startNode.textContent; //.nodeValue; //.wholeText
			cursor.endNodeValue = cursor.endNode.textContent; //.nodeValue; //.wholeText
			
			//find start and end node index
			for(var i=0;i<el.childNodes.length;++i)
			{
				if(el.childNodes[i] == cursor.startNode ||
						el.childNodes[i] == cursor.startNode.parentNode||
						el.childNodes[i] == cursor.startNode.parentNode.parentNode)
				{
					cursor.startNodeIndex = i;
					var str =  el.childNodes[i].parentNode.innerHTML;
					str += "<label id='startRangeNode'></label>" + str;
					el.childNodes[i].parentNode.innerHTML = str;
				}

				if(el.childNodes[i] == cursor.endNode ||
						el.childNodes[i] == cursor.endNode.parentNode||
						el.childNodes[i] == cursor.endNode.parentNode.parentNode)
					cursor.endNodeIndex = i;
			}

			console.log("cursor",cursor);
		}
		catch(err)
		{
			console.log("err",err);
		}

		
		
		
		var orig = el.innerText;//el.textContent;//inPlace?el.innerText:el.textContent;
		var str = "";
		
		console.log("orig", orig);
		//manage tabs, is 8 spaces on linux console - do the same		
		var s;
		var i,j,c;
		var lineStart = 0;
		var tabSz = 4; //to match eclipse!
		var t;
		var decor;
		
		var startOfWord = -1;
		var specialString;
		var startOfString = -1;
		
		
		for(i=0;i<orig.length;++i)
		{
			++t; //increment tab position
			
			//string handling
			if(startOfString != -1 || orig[i] == '"')
			{
				if(startOfString == -1) //start string
					startOfString = str.length;
				else if(orig[i] == '"')	//end string
				{
					specialString = str.substr(startOfString);
					//console.log("string",startOfString,str.length,specialString);
					str = str.substr(0,startOfString) + 
							"<label style='" +
							"font-weight:bold; " +
							"color:" + 
							_DECORATION_BLUE + "'>" + 
							specialString + "\"</label>";
					startOfString = -1;
					continue;
				}					
			}
			//special word handling			
			else if((orig[i] >= 'a' && orig[i] <= 'z') || 
							(orig[i] >= 'A' && orig[i] <= 'Z') ||
							(orig[i] >= '0' && orig[i] <= '9') || 
							(orig[i] == '_' || orig[i] == '-') || 
							orig[i] == '#')
			{
				if(startOfWord == -1)
					startOfWord = i;
				//else still within word
			}
			else if(startOfWord != -1) //found end of word, check for special word
			{
				specialString = orig.substr(startOfWord,i-startOfWord);
				startOfWord = -1;
				decor = _DECORATIONS[specialString];
				//console.log(specialString);
					
				if(decor) //found special word
				{
					//console.log(specialString);
					
					str = str.substr(0,str.length - specialString.length) + 
							"<label style='" +
							"font-weight:bold; " +
							"color:" + 
							decor + "'>" + 
							specialString + "</label>";	
				}				
			}
			
			
			//continue here with current character
			
						
			
			//replace other characters, like white space and html-chars
			if(0 && orig[i] == '\n') 
			{
				s = "<br>";
				t = 0; //reset tab start index				 		
			}
			else if(0 && orig[i] == '\t')
			{
				//found tab
				
				//calculate number of spaces to add
				s = ""; //determine the number of spaces to represent the tab as
				//s = "&emsp;";
				c = tabSz - (t%tabSz) + 1; 
				if(c <= 1) c+=tabSz; //make sure tab space is > 1				
				for(j=0;j<c;++j)
					s += "&nbsp;";
				t += c;				
			}
			else if(0 && orig[i] == '<')
				s = "&lt;";
			else if(0 && orig[i] == '>')
				s = "&gt;";
			else if(0 && orig[i] == ' ')
				s = "&nbsp;";
//			else if(orig[i] == '(' || orig[i] == ')'
//					 || orig[i] == '{' || orig[i] == '}'
//							 || orig[i] == '=') //single character special 
//				s = "<label style='" +
//				"font-weight:bold; " +
//				"color:" + 
//				_DECORATION_BLACK + "'>" + orig[i] + "</label>";
			else
			{
				str += orig[i];
				continue; //dont do special replace
			}
			
			str += s;//[str.slice(0, i), s, str.slice(i + 1)].join('');
			//i += s.length-1; //skip ahead by size of insert			
		} //done modifying string
			
//
//		for(var d in _DECORATIONS)
//		{				
//			str = str.replace(new RegExp(d, 'g'),"<label style='" +
//					"font-weight:bold; " +
//					"color:" + 
//					_DECORATIONS[d] + "'>" + d + "</label>");
//		}
		
		console.log("str",str);
		
		//replace other characters
//		str = str
//						.replace(/\n/g, "<br>")  			//&
//						;
//						.replace(/%26/g, "&amp;")  			//&
//						.replace(/%3C/g, "&lt;") 				//<
//						.replace(/%3E/g, "&gt;")				//>
//						.replace(/%22/g, "&quot;")				//"
//						.replace(/%27/g, "&#039;")				//'
//						.replace(/%0A%0D/g, "<br>")			//newline
//						.replace(/%20%20/g, "&nbsp;&nbsp;")		//double space
//						;
		el.innerHTML = str;
		
		
		
		//handle set cursor placement
		try
		{
			console.log("cursor",cursor);
			
			range = document.createRange();
			
//			cursor.startNodeIndex = 0;
//			cursor.endNodeIndex = 0;
//			cursor.startPos = 1;
//			cursor.endPos = 3;
			
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
			
//			selection = window.getSelection();
//				if (selection.rangeCount > 0) {
//					selection.removeAllRanges();
//					selection.addRange(range);
//				}
//				
			var selection = window.getSelection();
			selection.removeAllRanges();
			selection.addRange(range);

		}
		catch(err)
		{
			console.log("err",err);
		}

		
	} //end updateDecorations()
	
} //end create() CodeEditor instance











