//=====================================================================================
//
//	Created August, 2019
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	ArtdaqConfigurationAPI.js
//
//  Requirements: 
//   1. paste the following: 
//				
//				<script type="text/JavaScript" src="/WebPath/js/Globals.js"></script>	
//				<script type="text/JavaScript" src="/WebPath/js/Debug.js"></script>	
//				<script type="text/JavaScript" src="/WebPath/js/DesktopWindowContentCode.js"></script>
//				<script type="text/JavaScript" src="/WebPath/js/js_lib/ConfiguraitonAPI.js"></script>
//				<link rel="stylesheet" type="text/css" href="/WebPath/css/ConfigurationAPI.css">
//				<script type="text/JavaScript" src="/WebPath/js/js_lib/ArtdaqConfiguraitonAPI.js"></script>
//
//		...anywhere inside the <head></head> tag of a window content html page
//	 2. for proper functionality certain handlers are used:
//   		cannot overwrite handlers for window: onfocus, onscroll, onblur, onmousemove
//			(if you must overwrite, try to call the DesktopContent handlers from your handlers)
//
//	Recommendations:
//	 1. use Debug to output status and errors, e.g.:
//				Debug.log("this is my status",Debug.LOW_PRIORITY); //LOW_PRIORITY, MED_PRIORITY, INFO_PRIORITY, WARN_PRIORITY, HIGH_PRIORITY
//	 2. call window.focus() to bring your window to the front of the Desktop
//
//	The code of Requirement #1 should be inserted in the header of each page that will be 
//  the content of a window in the ots desktop.
//
//  This code handles bringing the window to the front when the content
//  is clicked or scrolled.
//
// Example usage: 	/WebPath/html/ConfigurationGUI_artdaq.html
//
//=====================================================================================

var ArtdaqConfigurationAPI = ArtdaqConfigurationAPI || {}; //define ArtdaqConfigurationAPI namespace

if (typeof Debug == 'undefined') 
	alert('ERROR: Debug is undefined! Must include Debug.js before ConfigurationAPI.js');
if (typeof Globals == 'undefined') 
	alert('ERROR: Globals is undefined! Must include Globals.js before ConfigurationAPI.js');
if (typeof DesktopContent == 'undefined' && 
		typeof Desktop == 'undefined') 
	alert('ERROR: DesktopContent is undefined! Must include DesktopContent.js before ConfigurationAPI.js');


//"public" function list: 
//	ArtdaqConfigurationAPI.getArtdaqNodes(responseHandler,modifiedTables)

//"public" helpers:

//"public" members:

//"public" constants:
ArtdaqConfigurationAPI.NODE_TYPES = ["reader","builder",
									 "logger","dispacher","monitor"];

//"private" function list:

//"private" constants:

//=====================================================================================
//getArtdaqNodes ~~
//	get currently active artdaq nodes
//
//	when complete, the responseHandler is called with an object parameter.
//		on failure, the object will be empty.
//		on success, the object of Active artdaq nodes
//		artdaqNodes := {}
//			artdaqNodes.<nodeType> = {}
//			artdaqNodes.<nodeType>.<nodeName> = {} //for now node empty, but could put context url/etc., or just look it up as needed with server
//			...
//
//		<nodeType> = reader, builder, aggregator, dispatcher
//
ArtdaqConfigurationAPI.getArtdaqNodes = function(responseHandler,
		modifiedTables)
{	
	var modifiedTablesListStr = "";
	for(var i=0;modifiedTables && i<modifiedTables.length;++i)
	{
		if(i) modifiedTablesListStr += ",";
		modifiedTablesListStr += modifiedTables[i].tableName + "," +
				modifiedTables[i].tableVersion;
	}
	
	//get active configuration group
	DesktopContent.XMLHttpRequest("Request?RequestType=getArtdaqNodes",
			"modifiedTables=" + modifiedTablesListStr, //end post data, 
			function(req) 
			{
		responseHandler(localExtractActiveArtdaqNodes(req));
			},
			0,0,true  //reqParam, progressHandler, callHandlerOnErr
	); //end of getActiveTableGroups handler
	
	return;
	
	//=================
	function localExtractActiveArtdaqNodes(req)
	{
		Debug.log("localExtractActiveArtdaqNodes");
		
		//can call this at almost all API handlers
		try
		{
			var types = ArtdaqConfigurationAPI.NODE_TYPES;
			
			var i,j;
			var retObj = {};	
			

			retObj.nodeCount = 0;
						
			for(i=0;i<types.length;++i)
			{
				Debug.log("Extracting " + types[i]);
				var nodes = req.responseXML.getElementsByTagName(
						types[i]);
				var addresses = req.responseXML.getElementsByTagName(
						types[i] + "-contextAddress");
				var ports = req.responseXML.getElementsByTagName(
						types[i] + "-contextPort");

				retObj[types[i]] = {};

				for(j=0;j<nodes.length;++j)
					retObj[types[i]][nodes[j].getAttribute('value')] = 
						{
								"address": 	addresses[j].getAttribute('value'),
								"port": 	ports[j].getAttribute('value'),
						};
				
				Debug.log("Extracted " + 
						nodes.length + " " +
						types[i]);
				
				retObj.nodeCount += nodes.length;
				
			} //end type extraction loop
		
			Debug.log("Total nodes extracted " +
					retObj.nodeCount);
		}
		catch(e)
		{
			Debug.log("Error extracting active artdaq nodes: " + e);
			return undefined;
		}

		return retObj;
	} // end localExtractActiveArtdaqNodes()
	
} // end getArtdaqNodes()





















