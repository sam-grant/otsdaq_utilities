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
	console.log('ERROR: Debug is undefined! Must include Debug.js before Desktop.js');
	
if (typeof Desktop == 'undefined') 
	console.log('ERROR: Desktop is undefined! Must include Desktop.js before DesktopIcons.js');
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
        
        var _iconsElement;
        
        var _numOfIcons = 0;
        
		//------------------------------------------------------------------
		//create public members variables ----------------------
		//------------------------------------------------------------------
		this.iconsElement;

		this.iconAlts = [
   			//"Open a FEC",
   			"Calibrate",
   			"Physics",
   			//"PhysicsLore",
   			"Chat",
   			"System Status",
   			"Logbook",
   			"Visualize",
   			"Configure"
   			];
		this.iconTitles =  [
		                    //"FEC",
		                    "CAL",	"PHY",	
		                    //"PHY",	
		                    "CHAT",	"SYS",	"LOG", 
		                    "VIS", 	"CFG"];
		this.iconUniques = [
		                    //0,
		                    1,		1,		
		                    //1,	
		                    1,		1,		1,		
		                    0,		1];
		this.iconPermissionsNeeded = 
   						   [
   						    //100,
   						    100,	1,		
   						    //1,
   						    1,		1,		1,		
   						   	1,		10];
		this.iconImages = [		//use 0 for text only ICON
   			//"icon-FecSupervisor.png",
   			0,//"icon-Calibrations.png",
   			"icon-Physics.gif",
   			//"icon-Physics.gif",
   			"icon-Chat.png",
			"icon-SystemStatus.png",
			"icon-Logbook1.png",
			"icon-Visualizer.png",
			0
			];
		this.iconLinks = [
  			//"/WebPath/html/FecDirectory.html",//"http://rulinux01.dhcp.fnal.gov:1983/urn:xdaq-application:lid=253/",
  			"/WebPath/html/Calibrations.html",
  			"/WebPath/html/Physics.html",
  			//"/WebPath/html/PhysicsLore.html",
  			"/urn:xdaq-application:lid=250/",//"/WebPath/html/Chat.html"
  			"/WebPath/html/SystemStatus.html",
  			"/urn:xdaq-application:lid=260/",//"/WebPath/html/Logbook.html"
  			"/WebPath/html/Visualization.html?urn=270",//"/urn:xdaq-application:lid=242/" //"/WebPath/html/Visualization.html"//"/WebPath/js/WebGL/RotatingPlanes/OldRotating3D.html"
  			"/urn:xdaq-application:lid=280/"
  			];
		
		
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

           	for(var i=0;i<this.iconAlts.length;++i) //add icons
           		if(permissions >= this.iconPermissionsNeeded[i])
           			this.addIcon(this.iconAlts[i],this.iconTitles[i],this.iconLinks[i],this.iconUniques[i],this.iconImages[i]);           
      	}
      	
      	// this.addIcon ~~
      	//	adds an icon with alt text, subtext wording underneath and image icon (if picfn defined)
      	this.addIcon = function(alt, subtext, linkurl, uniqueWin, picfn) {
      	      		
      		var iconContainer = document.createElement("div");			
			iconContainer.setAttribute("class", "DesktopIcons-iconContainer");
			
			var link;
			var div;
			
			var jsCmd = "Javascript:Desktop.desktop.addWindow(\""+ alt + "\",\"" + ""
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
			if(picfn){ //if icon image			
				div.style.backgroundImage = "url(../images/iconImages/" + picfn+")";
				
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










