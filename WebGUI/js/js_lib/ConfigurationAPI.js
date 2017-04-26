//=====================================================================================
//
//	Created April, 2017
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	ConfigurationAPI.js
//
//  Requirements: 
//   1. paste the following: 
//				
//				<script type="text/JavaScript" src="/WebPath/js/Globals.js"></script>	
//				<script type="text/JavaScript" src="/WebPath/js/Debug.js"></script>	
//				<script type="text/JavaScript" src="/WebPath/js/DesktopWindowContentCode.js"></script>
//				<script type="text/JavaScript" src="/WebPath/js/js_lib/ConfiguraitonAPI.js"></script>
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
// Example usage: 	/WebPath/html/ConfigurationGUI.html
//					/WebPath/html/ConfigurationGUI_subset.html
//
//=====================================================================================

var ConfigurationAPI = ConfigurationAPI || {}; //define ConfigurationAPI namespace

if (typeof Debug == 'undefined') 
	alert('ERROR: Debug is undefined! Must include Debug.js before ConfigurationAPI.js');
if (typeof Globals == 'undefined') 
	alert('ERROR: Globals is undefined! Must include Globals.js before ConfigurationAPI.js');
if (typeof DesktopContent == 'undefined' && 
		typeof Desktop == 'undefined') 
	alert('ERROR: DesktopContent is undefined! Must include DesktopContent.js before ConfigurationAPI.js');


//"public" function list: 
//	ConfigurationAPI.getDateString(date)
// 	ConfigurationAPI.getSubsetRecords(subsetBasePath,filterList,responseHandler)

//"private" function list:


//=====================================================================================
//getSubsetRecords ~~
//	takes as input a base path where the desired records are, 
//	  and a filter list.
//
// <filterList> is a CSV list of tree paths relative to <subsetBasePath> 
//	 and their required value.
//		e.g. "LinkToFETypeConfiguration=NIMPlus,FEInterfacePluginName=NIMPlusPlugin"
//
// <modifiedTables> is an array of Table objects (as returned from 
//		ConfigurationAPI.setFieldValuesForRecords)
//
//	when complete, the responseHandler is called with an array parameter.
//		on failure, the array will be empty.
//		on success, the array will be an array of records (their UIDs) 
//			from the subset that match the filter list
//
ConfigurationAPI.getSubsetRecords = function(subsetBasePath,
		filterList,responseHandler,modifiedTables)
{
	var modifiedTablesListStr = "";
	for(var i=0;modifiedTables && i<modifiedTables.length;++i)
	{
		if(i) modifiedTablesListStr += ",";
		modifiedTablesListStr += modifiedTables[i].tableName + "," +
				modifiedTables[i].tableVersion;
	}
	
	DesktopContent.XMLHttpRequest("Request?RequestType=getTreeView" + 
			"&configGroup=" +
			"&configGroupKey=-1" +
			"&hideStatusFalse=0" + 
			"&depth=1", //end get data 
			"startPath=/" + subsetBasePath +  
			"&filterList=" + filterList + 
			"&modifiedTables=" + modifiedTablesListStr, //end post data
			function(req)
			{
		var records = [];
		var err = DesktopContent.getXMLValue(req,"Error");
		if(err) 
		{
			Debug.log(err,Debug.HIGH_PRIORITY);
			responseHandler(records);
			return;
		}
		
		//console.log(req);
		
		var tree = DesktopContent.getXMLNode(req,"tree");
		var nodes = tree.children;
		for(var i=0;i<nodes.length;++i)
			records.push(nodes[i].getAttribute("value"));
		Debug.log("Records: " + records);
		responseHandler(records);

			}, //handler
			0, //handler param
			0,0,true); //progressHandler, callHandlerOnErr, showLoadingOverlay
}


//=====================================================================================
//getFieldsOfRecords ~~
//	takes as input a base path where the records are, 
//	  and an array of records.
// <recordArr> is an array or record UIDs (as returned from 
//		ConfigurationAPI.getSubsetRecords)
// <fieldList> is a CSV list of tree paths relative to <subsetBasePath> 
//	 to the allowed fields. If empty, then all available fields are allowed.
//		e.g. "LinkToFETypeConfiguration,FEInterfacePluginName"
//
// <modifiedTables> is an array of Table objects (as returned from 
//		ConfigurationAPI.setFieldValuesForRecords)
//
// 	maxDepth is used to force a end to search for common fields
//	
//	when complete, the responseHandler is called with an array parameter.
//		on failure, the array will be empty.
//		on success, the array will be an array of Field objects	
//		Field := {}
//			obj.fieldTableName 
//			obj.fieldUID 
//			obj.fieldColumnName
//			obj.fieldRelativePath 
//			obj.fieldColumnType
//			
//
ConfigurationAPI.getFieldsOfRecords = function(subsetBasePath,recordArr,fieldList,
		maxDepth,responseHandler,modifiedTables)
{
	var modifiedTablesListStr = "";
	for(var i=0;modifiedTables && i<modifiedTables.length;++i)
	{
		if(i) modifiedTablesListStr += ",";
		modifiedTablesListStr += modifiedTables[i].tableName + "," +
				modifiedTables[i].tableVersion;
	}
	
	var recordListStr = "";
	for(var i=0;i<recordArr.length;++i)
	{
		if(i) recordListStr += ",";
		recordListStr += recordArr[i];
	}
	
	DesktopContent.XMLHttpRequest("Request?RequestType=getTreeNodeCommonFields" + 
			"&configGroup=" +
			"&configGroupKey=-1" + 
			"&depth=" + (maxDepth|0), //end get data 
			"startPath=/" + subsetBasePath + 
			"&recordList=" + recordListStr +  
			"&fieldList=" + fieldList +
			"&modifiedTables=" + modifiedTablesListStr, //end post data
			function(req)
			{
		var recFields = [];
		var err = DesktopContent.getXMLValue(req,"Error");
		if(err) 
		{
			Debug.log(err,Debug.HIGH_PRIORITY);
			responseHandler(recFields);
			return;
		}
		
		var fields = DesktopContent.getXMLNode(req,"fields");
		
		var FieldTableNames = fields.getElementsByTagName("FieldTableName");
		var FieldColumnNames = fields.getElementsByTagName("FieldColumnName");
		var FieldRelativePaths = fields.getElementsByTagName("FieldRelativePath");
		var FieldColumnTypes = fields.getElementsByTagName("FieldColumnType");
		
		for(var i=0;i<FieldTableNames.length;++i)
		{
			var obj = {};
			obj.fieldTableName = DesktopContent.getXMLValue(FieldTableNames[i]);
			obj.fieldColumnName = DesktopContent.getXMLValue(FieldColumnNames[i]);
			obj.fieldRelativePath = DesktopContent.getXMLValue(FieldRelativePaths[i]);
			obj.fieldColumnType = DesktopContent.getXMLValue(FieldColumnTypes[i]);
			recFields.push(obj);
		}
		Debug.log("Records: " + recFields);		
		responseHandler(recFields);

			}, //handler
			0, //handler param
			0,0,true); //progressHandler, callHandlerOnErr, showLoadingOverlay
}


//=====================================================================================
//getFieldValuesForRecord ~~
//	takes as input a base path where the record is, 
//	  and the record uid.
// <recordArr> is an array or record UIDs (as returned from 
//		ConfigurationAPI.getSubsetRecords)
// <fieldObjArr> is an array of field objects (as returned from 
//		ConfigurationAPI.getFieldsOfRecords). This
//		is converted internally to a CSV list of tree paths relative to <subsetBasePath> 
//	 	to the fields to be read.
//	
// <modifiedTables> is an array of Table objects (as returned from 
//		ConfigurationAPI.setFieldValuesForRecords)
//
//	when complete, the responseHandler is called with an array parameter.
//		on failure, the array will be empty.
//		on success, the array will be an array of FieldValue objects	
//		FieldValue := {}
//			obj.fieldUID
//			obj.fieldPath   
//			obj.fieldValue
//
ConfigurationAPI.getFieldValuesForRecord = function(subsetBasePath,recordArr,fieldObjArr,
		responseHandler,modifiedTables)
{	
	var modifiedTablesListStr = "";
	for(var i=0;modifiedTables && i<modifiedTables.length;++i)
	{
		if(i) modifiedTablesListStr += ",";
		modifiedTablesListStr += modifiedTables[i].tableName + "," +
				modifiedTables[i].tableVersion;
	}

	var recordListStr = "";
	for(var i=0;i<recordArr.length;++i)
	{
		if(i) recordListStr += ",";
		recordListStr += recordArr[i];
	}
	
	var fieldListStr = "";
	for(var i=0;i<fieldObjArr.length;++i)
	{
		if(i) fieldListStr += ",";
		fieldListStr += fieldObjArr[i].fieldRelativePath + 
				fieldObjArr[i].fieldColumnName;
	}
	
	DesktopContent.XMLHttpRequest("Request?RequestType=getTreeNodeFieldValues" + 
			"&configGroup=" +
			"&configGroupKey=-1", //end get data 
			"startPath=/" + subsetBasePath + 
			"&recordList=" + recordListStr +
			"&fieldList=" + fieldListStr + 
			"&modifiedTables=" + modifiedTablesListStr, //end post data
			function(req)
			{
		var recFieldValues = [];
		var err = DesktopContent.getXMLValue(req,"Error");
		if(err) 
		{
			Debug.log(err,Debug.HIGH_PRIORITY);
			responseHandler(recFieldValues);
			return;
		}
		
		var fieldValues = req.responseXML.getElementsByTagName("fieldValues");

		for(var f=0;f<fieldValues.length;++f)
		{
			var FieldPaths = fieldValues[f].getElementsByTagName("FieldPath");
			var FieldValues = fieldValues[f].getElementsByTagName("FieldValue");
			for(var i=0;i<FieldPaths.length;++i)
			{
				var obj = {};
				obj.fieldUID = DesktopContent.getXMLValue(fieldValues[f]);
				obj.fieldPath = DesktopContent.getXMLValue(FieldPaths[i]);
				obj.fieldValue = DesktopContent.getXMLValue(FieldValues[i]);
				recFieldValues.push(obj);
			}
		}
		
		responseHandler(recFieldValues);

			}, //handler
			0, //handler param
			0,0,true); //progressHandler, callHandlerOnErr, showLoadingOverlay
}

//=====================================================================================
//setFieldValuesForRecords ~~
//	takes as input a base path where the records are, 
//	  and an array of records.
// <recordArr> is an array or record UIDs (as returned from 
//		ConfigurationAPI.getSubsetRecords)
// <fieldObjArr> is an array of field objects (as returned from 
//		ConfigurationAPI.getFieldsOfRecords). This
//		is converted internally to a CSV list of tree paths relative to <subsetBasePath> 
//	 	to the fields to be written.
// <valueArr> is an array of values, with index corresponding to the associated 
//	 	field in the <fieldObjArr>.
//
// <modifiedTables> is an array of Table objects (as returned from 
//		ConfigurationAPI.setFieldValuesForRecords)
//	
//	when complete, the responseHandler is called with an array parameter.
//		on failure, the array will be empty.
//		on success, the array will be an array of Table objects	
//		Table := {}
//			obj.tableName   
//			obj.tableVersion
//
//
ConfigurationAPI.setFieldValuesForRecords = function(subsetBasePath,recordArr,fieldObjArr,
		valueArr,responseHandler,modifiedTables)
{	
	var modifiedTablesListStr = "";
	for(var i=0;modifiedTables && i<modifiedTables.length;++i)
	{
		if(i) modifiedTablesListStr += ",";
		modifiedTablesListStr += modifiedTables[i].tableName + "," +
				modifiedTables[i].tableVersion;
	}
	
	var fieldListStr = "";
	for(var i=0;i<fieldObjArr.length;++i)
	{
		if(i) fieldListStr += ",";
		fieldListStr += fieldObjArr[i].fieldRelativePath + 
				fieldObjArr[i].fieldColumnName;
	}
	
	var valueListStr = "";
	for(var i=0;i<valueArr.length;++i)
	{
		if(i) valueListStr += ",";
		valueListStr += valueArr[i];
	}
	
	var recordListStr = "";
	for(var i=0;i<recordArr.length;++i)
	{
		if(i) recordListStr += ",";
		recordListStr += recordArr[i];
	}
	
	DesktopContent.XMLHttpRequest("Request?RequestType=setTreeNodeFieldValues" + 
			"&configGroup=" +
			"&configGroupKey=-1", //end get data 
			"startPath=/" + subsetBasePath +  
			"&recordList=" + recordListStr +
			"&valueList=" + valueListStr +
			"&fieldList=" + fieldListStr + 
			"&modifiedTables=" + modifiedTablesListStr, //end post data
			function(req)
			{
		
		var modifiedTables = [];
		
		var err = DesktopContent.getXMLValue(req,"Error");
		if(err) 
		{
			Debug.log(err,Debug.HIGH_PRIORITY);
			if(responseHandler) responseHandler(modifiedTables);
			return;
		}		
		//modifiedTables
		var tableNames = req.responseXML.getElementsByTagName("NewActiveTableName");
		var tableVersions = req.responseXML.getElementsByTagName("NewActiveTableVersion");
		var tableVersion;
		
		//add only temporary version
		for(var i=0;i<tableNames.length;++i)
		{
			tableVersion = DesktopContent.getXMLValue(tableVersions[i])|0; //force integer
			if(tableVersion >= -1) continue; //skip unless temporary
			var obj = {};
			obj.tableName = DesktopContent.getXMLValue(tableNames[i]);
			obj.tableVersion = DesktopContent.getXMLValue(tableVersions[i]);
			modifiedTables.push(obj);
		}
		
		if(responseHandler) responseHandler(modifiedTables);

			}, //handler
			0, //handler param
			0,0,true); //progressHandler, callHandlerOnErr, showLoadingOverlay
}

//=====================================================================================
//getDateString ~~
//	Example call from linux timestamp:
//		groupCreationTime = ConfigurationAPI.getDateString(new Date((groupCreationTime|0)*1000));
ConfigurationAPI.getDateString;
{
var dayArr_ = ["Sun","Mon","Tue","Wed","Thu","Fri","Sat"];
var monthArr_ = ["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"];
ConfigurationAPI.getDateString = function(date)
{
	var dateStr = "";

	dateStr += dayArr_[date.getDay()];
	dateStr += " ";
	dateStr += monthArr_[date.getMonth()];
	dateStr += " ";
	dateStr += date.getDate();
	dateStr += " ";
	dateStr += date.getHours();
	dateStr += ":";
	dateStr += ((date.getMinutes()<10)?"0":"") + date.getMinutes();
	dateStr += ":";
	dateStr += ((date.getSeconds()<10)?"0":"") + date.getSeconds();
	dateStr += " ";
	dateStr += date.getFullYear();
	dateStr += " ";
	dateStr += date.toLocaleTimeString([],{timeZoneName: "short"}).split(" ")[2];
	return dateStr;
}
}























