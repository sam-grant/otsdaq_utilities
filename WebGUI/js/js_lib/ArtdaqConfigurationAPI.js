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
ArtdaqConfigurationAPI.NODE_TYPE_READER 		= 0;
ArtdaqConfigurationAPI.NODE_TYPE_BUILDER 		= 1;
ArtdaqConfigurationAPI.NODE_TYPE_LOGGER 		= 2;
ArtdaqConfigurationAPI.NODE_TYPE_DISPATCHER 	= 3;
ArtdaqConfigurationAPI.NODE_TYPE_MONITOR 		= 4;
ArtdaqConfigurationAPI.NODE_TYPES = ["reader","builder",
									 "logger","dispatcher","monitor"];

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
		
		//Example xml response:
		//		<artdaqSupervisor value='ARTDAQSupervisor'>
		//			<artdaqSupervisor-contextAddress value='http://correlator2.fnal.gov'/>
		//			<artdaqSupervisor-contextPort value='2016'/>
		//			<subsystem value='nullDestinationSubsystem'/>
		//			<subsystem-id value='0'/>
		//			<subsystem-sourcesCount value='1'/>
		//			<subsystem-destination value='0'/>
		//			<subsystem value='subsystem1'/>
		//			<subsystem-id value='2'/>
		//			<subsystem-sourcesCount value='0'/>
		//			<subsystem-destination value='3'/>
		//			<subsystem value='subsystem2'/>
		//			<subsystem-id value='3'/>
		//			<subsystem-sourcesCount value='1'/>
		//			<subsystem-destination value='0'/>
		//			<reader value='reader0'/>
		//			<reader-hostname value='localhost'/>
		//			<reader-subsystem value='2'/>
		//			<builder value='builder0'/>
		//			<builder-hostname value='localhost'/>
		//			<builder-subsystem value='2'/>
		//			<builder value='builder1'/>
		//			<builder-hostname value='localhost'/>
		//			<builder-subsystem value='3'/>
		//			<logger value='logger0'/>
		//			<logger-hostname value='localhost'/>
		//			<logger-subsystem value='3'/>
		//			<dispatcher value='dispatcher0'/>
		//			<dispatcher-hostname value='localhost'/>
		//			<dispatcher-subsystem value='3'/>
		//		</artdaqSupervisor>
		
		//can call this at almost all API handlers
		try
		{
			var types = ArtdaqConfigurationAPI.NODE_TYPES;
			
			var i,j;
			var retObj = {};	
			

			retObj.nodeCount = 0;
			
			var artdaqSupervisor = DesktopContent.getXMLNode(
					req.responseXML,
					"artdaqSupervisor");
			
			if(artdaqSupervisor)
			{
				//extract all processes from the artdaq supervisor object
				
				for(i=0;i<types.length;++i)
				{
					Debug.log("Extracting " + types[i]);
					var nodes = artdaqSupervisor.getElementsByTagName(
							types[i]);
					var hostnames = artdaqSupervisor.getElementsByTagName(
							types[i] + "-hostname");
					var subsystemIds = artdaqSupervisor.getElementsByTagName(
							types[i] + "-subsystem");
	
					retObj[types[i]] = {};
	
					for(j=0;j<nodes.length;++j)
						retObj[types[i]][nodes[j].getAttribute('value')] = 
							{
								"hostname": 	hostnames[j].getAttribute('value'),
								"subsystemId": 	subsystemIds[j].getAttribute('value') | 0, //integer
							};
					
					Debug.log("Extracted " + 
							nodes.length + " " +
							types[i]);
					
					retObj.nodeCount += nodes.length;
					
				} //end type extraction loop
				
				//extract all subsystems
				retObj.subsystems = {};
				var subsystems = artdaqSupervisor.getElementsByTagName("subsystem");
				var subsystemIds = artdaqSupervisor.getElementsByTagName(
						"subsystem" + "-id");
				var subsystemSourcesCount = artdaqSupervisor.getElementsByTagName(
						"subsystem" + "-sourcesCount");
				var subsystemDestination = artdaqSupervisor.getElementsByTagName(
						"subsystem" + "-destination");

				for(j=0;j<subsystems.length;++j)
				{
					retObj.subsystems[subsystemIds[j].getAttribute('value') | 0 /*integer*/] = 
						{
							"label":			subsystems[j].getAttribute('value'),
							"sourcesCount":		subsystemSourcesCount[j].getAttribute('value'),
							"destination":		subsystemDestination[j].getAttribute('value'),
						};
				}
				
				
			} //end artdaq Supervisor extraction
			else
				Debug.log("No artdaq Supervisor found.");
			
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





















