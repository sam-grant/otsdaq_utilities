//=====================================================================================
//
//	Created Mar, 2013
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	DesktopIcons.js
//
//	This is the desktop code for the user interface for ots. ots is the DAQ
// 		and control software for the Fermi Strips Telescope.
//
//	The desktop consists of icons to launch windows
//
//=====================================================================================


if (typeof Debug == 'undefined') 
	Debug.log('ERROR: Debug is undefined! Must include Debug.js before DesktopIcons.js');
if (typeof SimpleContextMenu == 'undefined') 
	Debug.log('ERROR: SimpleContextMenu is undefined! Must include SimpleContextMenu.js before DesktopIcons.js');


	
if (typeof Desktop == 'undefined') 
	Debug.log('ERROR: Desktop is undefined! Must include Desktop.js before DesktopIcons.js');
else {

		
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	//define Desktop.createDashboard to return dashboard class	
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	Desktop.createIcons = function(userPermissions, z) {
        if(false === (this instanceof Desktop.createIcons)) {
			//here to correct if called as "var v = Desktop.createIcons();"
			//	instead of "var v = new Desktop.createIcons();"
	        return new Desktop.createIcons(userPermissions, z);
	    }
        
        
        //------------------------------------------------------------------
		//create private members variables ----------------------
		//------------------------------------------------------------------
		
		//all units in pixels unless otherwise specified
		
		var _defaultIconsMargin = 50; //margin to desktop border for all icons
		var _defaultIconWidth = 64;
		var _defaultIconHeight = 64;   
		var _defaultIconTextWidth = 90; 
		var _defaultIconTextHeight = 32; 
		var _permissions = 0;
        
        var _iconsElement;
        
        var _numOfIcons = 0;
        
        var _deepClickTimer = 0;
        
		//------------------------------------------------------------------
		//create public members variables ----------------------
		//------------------------------------------------------------------
		this.iconsElement;
		
		
		//------------------------------------------------------------------
		//create PRIVATE members functions ----------------------
		//------------------------------------------------------------------
        
        var _redrawIcons = function() {      
        
			_iconsElement.style.left = Desktop.desktop.getDesktopContentX()+_defaultIconsMargin+"px";
			_iconsElement.style.top = Desktop.desktop.getDesktopContentY()+_defaultIconsMargin+"px";
			_iconsElement.style.width = Desktop.desktop.getDesktopContentWidth()-_defaultIconsMargin-_defaultIconsMargin+"px";
			_iconsElement.style.height = Desktop.desktop.getDesktopContentHeight()-_defaultIconsMargin-_defaultIconsMargin+"px";
            
        }
		
		//------------------------------------------------------------------
		//create PUBLIC members functions ----------------------
		//------------------------------------------------------------------
      
      	this.redrawIcons = _redrawIcons;

      	// this.addIcon ~~
      	//	adds an icon with alt text, subtext wording underneath and image icon (if picfn defined)
      	this.resetWithPermissions = function(permissions) {
            Debug.log("Desktop resetWithPermissions " + permissions,Debug.LOW_PRIORITY);
            
            this.iconsElement.innerHTML = ""; //clear current icons
            _numOfIcons = 0;
            _permissions = permissions;
            ////////////


			if(!Desktop.isWizardMode()) 
		    { //This is satisfied for  Digest Access Authorization and No Security on OTS
		    	Desktop.XMLHttpRequest("Request?RequestType=getDesktopIcons", "", iconRequestHandler);
		    	return;
	      	}
		    else //it is the sequence for OtsWizardConfiguration
			{
		    	Debug.log("OtsWizardConfiguration");
		    	Desktop.XMLHttpRequest("requestIcons", "sequence="+Desktop.desktop.security, iconRequestHandler);
	      		_permissions = 1;
		    	return;
			}
	      	 
      	}
      	
      	//_iconRequestHandler
      	//adds the icons from the hardcoded, C++ in OtsConfigurationWizard
      	var iconRequestHandler = function(req) {

      		var iconArray;

			if(!Desktop.isWizardMode()) 
		    { //This is satisfied for  Digest Access Authorization and No Security on OTS
		    	iconArray = Desktop.getXMLValue(req,"iconList"); 
				//Debug.log("icon Array unsplit: " + iconArray);
				iconArray = iconArray.split(","); 
			}
      		else //it is the wizard
      		{ 
      			iconArray = req.responseText.split(","); 
      		}
		    
			Debug.log("icon Array split: " + iconArray);
      		//Debug.log(_permissions);

     	    //an icon is 6 fields.. give comma-separated
     	    	//0 - alt = text for mouse over
     	    	//1 - subtext = text below icon
     	    	//2 - uniqueWin = if true, only one window is allowed, else multiple instances of window
     	    	//3 - permissions = security level needed to see icon
     	    	//4 - picfn = icon image filename
     	    	//5 - linkurl = url of the window to open
      		var numberOfIconFields = 6;
     		for(var i=0;i<(iconArray.length);i+=numberOfIconFields) //add icons
      		{
           		if(_permissions >= iconArray[i+3])
           			Desktop.desktop.icons.addIcon(iconArray[i],iconArray[i+1],iconArray[i+5],iconArray[i+2],iconArray[i+4]);       
      		}
     		
     		_permissions = 0;
      	}
      	
      	// this.addIcon ~~
      	//	adds an icon with alt text, subtext wording underneath and image icon (if picfn defined)
      	this.addIcon = function(alt, subtext, linkurl, uniqueWin, picfn) {
      	      		
      		//Debug.log("this.addIcon");
      		var iconContainer = document.createElement("div");			
			iconContainer.setAttribute("class", "DesktopIcons-iconContainer");
			
			var link;
			var div;
			
			var jsCmd = "Javascript:var newWin = Desktop.desktop.addWindow(\""+ alt + "\",\"" + ""
				 + "\",\"" + linkurl + "\","+uniqueWin+");";
				
			//create icon square
			link = document.createElement("a");
			link.setAttribute("class", "DesktopIcons-iconLink");
      		link.title = alt;
			link.href = jsCmd;
			
      		div = document.createElement("div");
			div.setAttribute("class", "DesktopIcons-iconDiv");
			div.style.width = _defaultIconWidth + "px";
			div.style.height = _defaultIconHeight + "px";
			div.style.marginLeft = (_defaultIconTextWidth-_defaultIconWidth-2)/2 + "px"; //extra 2 for border
			
			//define icon content
			if(picfn != "0"){ //if icon image			
				div.style.backgroundImage = "url(/WebPath/images/iconImages/" + picfn+")";
				
				var div2 = document.createElement("div");
				div2.setAttribute("class", "DesktopIcons-iconDiv");
				div2.style.width = _defaultIconWidth + "px";
				div2.style.height = _defaultIconHeight + "px";
				div2.style.marginLeft = (-1) + "px"; //put effect over top	of background image and line up with border		
				div2.style.marginTop = (-1) + "px"; 
				div.appendChild(div2);
			}
			else {	//if text only icon, gets effect from .css
        		div.innerHTML = "<table width='100%' height='100%'><td width='100%' height='100%' valign='middle' align='center'>"+
        			subtext+"</td></table>";
        	}
        			
			link.appendChild(div);
			iconContainer.appendChild(link); //add square to icon container

			//create icon sub text
      		link = document.createElement("a");
			link.setAttribute("class", "DesktopIcons-iconLink");
      		link.title = alt;
			link.href = jsCmd;
			
      		div = document.createElement("div");
			div.setAttribute("class", "DesktopIcons-iconSubText");
			div.style.width = _defaultIconTextWidth + "px";
			div.style.height = _defaultIconTextHeight + "px";
      		div.innerHTML = alt;
      		
			link.appendChild(div);				
			iconContainer.appendChild(link); //add subtext to icon container

			//add context click menu handlers
			//	mouseup and contextMenu to stop default right-click behavior
			//	and mousedown to start the menu (so "deep clicks" work)
			iconContainer.addEventListener("mouseup", function(event) {
				if(_deepClickTimer)
				{
					window.clearTimeout(_deepClickTimer);
					_deepClickTimer = 0;
				}
			}); //end onmouseup event
			
			var deepClickHandler = function(event) {				
				event.cancelBubble = true; //prevent default behavior 
				event.preventDefault();
				_deepClickTimer = window.setTimeout(function() {

					Debug.log("Create Icon Menu");
					var menuItems = [
									 "Open and Maximize Window",
									 "Open in New Browser Tab",
									 "Open and Tile All Windows"
									 ];
					console.log(linkurl);
					var menuItemHandlers = [
											"Desktop.desktop.addWindow(\""+ alt + "\",\"" + ""
											+ "\",\"" + linkurl + "\","+uniqueWin+",2);", // 2 for maximize
											"Desktop.openNewBrowserTab(\""+ alt + "\",\"" + ""
											+ "\",\"" + linkurl + "\","+uniqueWin+");", // 2 for maximize
											"Desktop.desktop.addWindow(\""+ alt + "\",\"" + ""
											+ "\",\"" + linkurl + "\","+uniqueWin+",1);", // 1 for tile					 
											];
					Debug.log("createEditTableMenu()");
					SimpleContextMenu.createMenu(
							menuItems,
							menuItemHandlers,
							"DesktopIconContextMenu",		//element ID
							event.pageX-1,event.pageY-1, 	//top left position
							Desktop.desktop.dashboard.getDefaultDashboardColor(), 	//primary color
							"white"				//secondary color
					);
					
				},500); //end timeout handler
				
			}; //end deepClickHandler event
			
			//FIXME ... debug touchstart on android to block browser context
			//			iconContainer.addEventListener("touchstart", function(event) {	
			//				event.preventDefault();
			//				
			//				deepClickHandler(event);
			//				return false;
			//			});
			iconContainer.addEventListener("mousedown",	deepClickHandler); 
			
			_iconsElement.appendChild(iconContainer); //add to desktop icon element
			
			++_numOfIcons; //maintain icon count
			
			//reset icon arrangement based on _numOfIcons
			if(_numOfIcons > 1) {
				var cdArr = _iconsElement.getElementsByClassName("iconsClearDiv");		
				
				while(cdArr.length)	cdArr[0].parentNode.removeChild(cdArr[0]); //get rid of all clearDivs							

				var newLine = Math.ceil((-1 + Math.sqrt(_numOfIcons*8+1))/2);
				var newLineOff = newLine;
				
				var currChild = _iconsElement.childNodes[0];
				for(var i=0;i<_numOfIcons;++i) {	
					if(i == newLine) { //add new line before currChild				        
						div = document.createElement("div");
						div.setAttribute("id", "clearDiv");
						div.setAttribute("class", "iconsClearDiv");
						_iconsElement.insertBefore(div,currChild);						
						newLine += --newLineOff; //update new line index						
					}
					currChild = currChild.nextSibling; //get next child
				}
			}
      	}
        
		//------------------------------------------------------------------
		//handle class construction ----------------------
		//------------------------------------------------------------------
        
        this.iconsElement = _iconsElement = document.createElement("div");
		this.iconsElement.setAttribute("class", "DesktopIcons");
		this.iconsElement.setAttribute("id", "DesktopIcons");
		this.iconsElement.style.position = "absolute";
    	this.iconsElement.style.zIndex = z;
       
   		
		
        Debug.log("Desktop Icons created",Debug.LOW_PRIORITY);
    }

}










