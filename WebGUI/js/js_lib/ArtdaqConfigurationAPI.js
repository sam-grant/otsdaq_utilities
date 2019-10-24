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
//
//
// Example usage: 	/WebPath/html/ConfigurationGUI_artdaq.html
//
//=====================================================================================

var ArtdaqConfigurationAPI = ArtdaqConfigurationAPI || {}; //define ArtdaqConfigurationAPI namespace

if (typeof ConfigurationAPI == 'undefined') 
	alert('ERROR: ConfigurationAPI is undefined! Must include ConfigurationAPI.js before ArtdaqConfigurationAPI.js');

//"public" function list: 
//	ArtdaqConfigurationAPI.getArtdaqNodes(responseHandler,modifiedTables)
//	ArtdaqConfigurationAPI.saveArtdaqNodes(responseHandler,modifiedTables)

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


//Function definitions:

//=====================================================================================
//getArtdaqNodes ~~
//	get currently active artdaq nodes
//
//	when complete, the responseHandler is called with an object parameter.
//		on failure, the object will be empty.
//		on success, the object of Active artdaq nodes
//		retObj := {}
//			retObj.<nodeType> = {}
//			retObj.<nodeType>.<nodeName> = {hostname,subsystemId}
//			...
//			retObj.subsystems = {}
//			retObj.subsystems.<subsystemId> = {label,sourcesCount,destination}
//
//		<nodeType> = ArtdaqConfigurationAPI.NODE_TYPES := reader, builder, aggregator, dispatcher, monitor
//
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

//====================================================================================
//saveArtdaqNodes ~~
//	save artdaq nodes and subsystems to active groups (with modified tables)
//		nodeObj := {}
//			nodeObj.<nodeType> = {}
//			nodeObj.<nodeType>.<nodeName> = {originalName,hostname,subsystemName}
//
// <nodeType> = ArtdaqConfigurationAPI.NODE_TYPES := reader, builder, aggregator, dispatcher, monitor
//
//		subsystemObj = {}
//			subsystemObj.<subsystemName> = {destination}
//
ArtdaqConfigurationAPI.saveArtdaqNodes = function(nodesObject, subsystemsObject, responseHandler,
		modifiedTables)
{	
	console.log("nodesObject",nodesObject);
	console.log("subsystemsObject",subsystemsObject);
	
	var modifiedTablesListStr = "";
	for(var i=0;modifiedTables && i<modifiedTables.length;++i)
	{
		if(i) modifiedTablesListStr += ",";
		modifiedTablesListStr += modifiedTables[i].tableName + "," +
				modifiedTables[i].tableVersion;
	}
	
	var nodeStr = "";
	var subsystemStr = "";
	
	for(var i in nodesObject)
	{
		nodeStr += encodeURIComponent(i) + ":";
		for(var j in nodesObject[i])
		{
			nodeStr += encodeURIComponent(j) + "=";
			nodeStr += encodeURIComponent(nodesObject[i][j].originalHostname) + ",";
			nodeStr += encodeURIComponent(nodesObject[i][j].hostname) + ",";
			nodeStr += encodeURIComponent(nodesObject[i][j].subsystemName) + "";
			nodeStr += ";"; //end node
		}
		nodeStr += "|"; //end artdaq type		
	}
	for(var i in subsystemsObject)
	{
		subsystemStr += encodeURIComponent(i) + ":";
		subsystemStr += encodeURIComponent(subsystemStr[i].destination);
		subsystemStr += ";"; //end subsystem 	
	}
	
	console.log("nodeStr",nodeStr);
	console.log("subsystemStr",subsystemStr);

	//get active configuration group
	DesktopContent.XMLHttpRequest("Request?RequestType=saveArtdaqNodes",			
			"modifiedTables=" + modifiedTablesListStr + 
			"&nodeStr=" + nodeStr +
			"&subsystemStr=" + subsystemStr, //end post data, 
			function(req) 
			{
		console.log("response",req);
		//responseHandler(localExtractActiveArtdaqNodes(req));
			},
			0,0,true  //reqParam, progressHandler, callHandlerOnErr
	); //end of getActiveTableGroups handler

	return;
	
	
} // end saveArtdaqNodes()



















