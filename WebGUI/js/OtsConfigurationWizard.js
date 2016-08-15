//=====================================================================================
//
//	Created Jun, 2014
//	by Daniel Parilla ((parilla at fnal.gov))
//
//	OtsConfigurationWizard.js
//
//
//
//
//
//
//=====================================================================================
var OtsConfigurationWizard = OtsConfigurationWizard || {}; 
OtsConfigurationWizard.init = function()
{
	
	console.log("init()");
	OtsConfigurationWizard.desktop = OtsConfigurationWizard.createDesktop();
	if(OtsConfigurationWizard.desktop)
		Debug.log("OtsConfigurationWizard.desktop Initalized Successfully",Debug.LOW_PRIORITY);	
}

OtsConfigurationWizard.createDesktop = function(){
	
	if (typeof Debug == 'undefined') return 0; //fail if debug not defined just to force consistent behavior
	
	if(false === (this instanceof OtsConfigurationWizard.createDesktop)) {
		//here to correct if called as "var v = Desktop.createDesktop();"
		//	instead of "var v = new Desktop.createDesktop();"
        return new OtsConfigurationWizard.createDesktop();
	
}
}

OtsConfigurationWizard.buttonClick = function()
{
	var text = document.getElementById("input1").value;
	console.log(text);
	
}