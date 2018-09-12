


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
	//	init()
	//	createElements()
	//	redrawWindow()
		
	
	//for display
	var _CHECKBOX_H = 40;
	var _CHECKBOX_MIN_W = 240;
	var _CHECKBOX_MAX_W = 540;
	var _WINDOW_MIN_SZ = 525;
	var _MARGIN = 40;


	var _needEventListeners = true;
	
	var _viewMode = 0; //0: only primary, 1: vertical split, 2: horizontal split

	
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

		createElements();
		redrawWindow();
		
		if(_needEventListeners)
		{
			window.addEventListener("resize",redrawWindow);
			_needEventListeners = false;
		}
		
		return;

		DesktopContent.XMLHttpRequest("Request?RequestType=codeEditor&option=getFolders", "" /* data */,
				function(req)
				{
			 
			var err = DesktopContent.getXMLValue(req,"Error"); //example application level error
			if(err) 
			{
				Debug.log(err,Debug.HIGH_PRIORITY);	//log error and create pop-up error box
				return;
			}
		    
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
			el.setAttribute("id","primaryPane");	
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
				el.innerHTML = str;
			}				
			cel.appendChild(el);
			
			//================
			//secondaryPane div
			el = document.createElement("div");
			el.setAttribute("id","secondaryPane");
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

		var pp = document.getElementById("primaryPane");
		var sp = document.getElementById("secondaryPane");
		//var cp = document.getElementById("controlsPane");
		
		pp.style.position = "absolute";
		sp.style.position = "absolute";
			
		switch(_viewMode)
		{
		case 0: //only primary
			pp.style.left 		= 0 + "px";
			pp.style.top 		= 0 + "px";
			pp.style.height 	= h + "px";
			pp.style.width 		= w + "px";	
			
			sp.style.display = "none";
			break;
		case 1: //vertical split
			pp.style.left 		= 0 + "px";
			pp.style.top 		= 0 + "px";
			pp.style.height 	= h + "px";
			pp.style.width 		= ((w/2)|0) + "px";	
			
			sp.style.left 		= ((w/2)|0) + "px";			
			sp.style.top 		= 0 + "px";
			sp.style.height 	= h + "px";
			w -= ((w/2)|0);
			sp.style.width 		= w + "px";	
			sp.style.display = "block";
			break;
		case 2: //horizontal split
			pp.style.left 		= 0 + "px";
			pp.style.top 		= 0 + "px";
			pp.style.height 	= ((h/2)|0) + "px";
			pp.style.width 		= w + "px";	
			
			sp.style.left 		= 0 + "px";			
			sp.style.top 		= ((h/2)|0) + "px";
			h -= ((h/2)|0);
			sp.style.height 	= h + "px";
			sp.style.width 		= w + "px";	
			sp.style.display = "block";
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
		Debug.log("toggleView " + _viewMode);
		redrawWindow();
	} //end toggleView()
	


} //end create() CodeEditor instance











