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
//				<link rel="stylesheet" type="text/css" href="/WebPath/css/ConfigurationAPI.css">
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
// 	ConfigurationAPI.getSubsetRecords(subsetBasePath,filterList,responseHandler,modifiedTables)
//	ConfigurationAPI.getFieldsOfRecords(subsetBasePath,recordArr,fieldList,maxDepth,responseHandler,modifiedTables)
//	ConfigurationAPI.getFieldValuesForRecord(subsetBasePath,recordArr,fieldObjArr,responseHandler,modifiedTables)
// 	ConfigurationAPI.getUniqueFieldValuesForRecords(subsetBasePath,recordArr,fieldList,responseHandler,modifiedTables)
//	ConfigurationAPI.setFieldValuesForRecords(subsetBasePath,recordArr,fieldObjArr,valueArr,responseHandler,modifiedTables)
//	ConfigurationAPI.popUpSaveModifiedTablesForm(modifiedTables,responseHandler)
//	ConfigurationAPI.saveModifiedTables(modifiedTables,responseHandler,doNotIgnoreWarnings,doNotSaveAffectedGroups,doNotActivateAffectedGroups,doNotSaveAliases)
//	ConfigurationAPI.bitMapDialog(bitMapParams,initBitMapValue,okHandler,cancelHandler)
//	ConfigurationAPI.createEditableFieldElement(fieldObj,fieldIndex,depthIndex /*optional*/)
//	ConfigurationAPI.getEditableFieldValue(fieldObj,fieldIndex,depthIndex /*optional*/)
//	ConfigurationAPI.setEditableFieldValue(fieldObj,value,fieldIndex,depthIndex /*optional*/)

//"public" helpers:
//	ConfigurationAPI.setCaretPosition(elem, caretPos, endPos)
//	ConfigurationAPI.setPopUpPosition(el,w,h,padding,border,margin,doNotResize)
//	ConfigurationAPI.addClass(elem,class)
//	ConfigurationAPI.removeClass(elem,class)
//	ConfigurationAPI.hasClass(elem,class)


//"public" constants:
ConfigurationAPI._DEFAULT_COMMENT = "No comment.";
ConfigurationAPI._POP_UP_DIALOG_ID = "ConfigurationAPI-popUpDialog";

//"private" function list:
//	ConfigurationAPI.handleGroupCommentToggle(groupName,setHideVal)
//	ConfigurationAPI.handlePopUpHeightToggle(h,gh)
//	ConfigurationAPI.handlePopUpAliasEditToggle(i)
//	ConfigurationAPI.activateGroup(groupName, groupKey, ignoreWarnings)
//	ConfigurationAPI.setGroupAliasInActiveBackbone(groupAlias,groupName,groupKey,newBackboneNameAdd,doneHandler,doReturnParams)
//	ConfigurationAPI.newWizBackboneMemberHandler(req,params)
//	ConfigurationAPI.saveGroupAndActivate(groupName,configMap,doneHandler,doReturnParams)
//	ConfigurationAPI.getOnePixelPngData(rgba)
//
//		for Editable Fields
//	ConfigurationAPI.handleEditableFieldClick(depth,uid,editClick,type)
//	ConfigurationAPI.handleEditableFieldHover(depth,uid,event)
//	ConfigurationAPI.handleEditableFieldBodyMouseMove(event)
//	ConfigurationAPI.handleEditableFieldEditOK()
//	ConfigurationAPI.handleEditableFieldEditCancel()
//	ConfigurationAPI.handleEditableFieldKeyDown(event,keyEl)

//"private" constants:
ConfigurationAPI._VERSION_ALIAS_PREPEND = "ALIAS:";
ConfigurationAPI._SCRATCH_VERSION = 2147483647;
ConfigurationAPI._SCRATCH_ALIAS = "Scratch";

ConfigurationAPI._OK_CANCEL_DIALOG_STR = "";
ConfigurationAPI._OK_CANCEL_DIALOG_STR += "<div title='' style='padding:5px;background-color:#eeeeee;border:1px solid #555555;position:relative;z-index:2000;" + //node the expander nodes in tree-view are z-index:1000  
				"width:95px;height:20px;margin:0 -122px -64px 10px; font-size: 16px; white-space:nowrap; text-align:center;'>";
ConfigurationAPI._OK_CANCEL_DIALOG_STR += "<a class='popUpOkCancel' onclick='javascript:ConfigurationAPI.handleEditableFieldEditOK(); event.stopPropagation();' onmouseup='event.stopPropagation();' title='Accept Changes' style='color:green'>" +
				"<b style='color:green;font-size: 16px;'>OK</b></a> | " +
				"<a class='popUpOkCancel' onclick='javascript:ConfigurationAPI.handleEditableFieldEditCancel(); event.stopPropagation();' onmouseup='event.stopPropagation();' title='Discard Changes' style='color:red'>" + 
				"<b style='color:red;font-size: 16px;'>Cancel</b></a>";
ConfigurationAPI._OK_CANCEL_DIALOG_STR += "</div>";	

//=====================================================================================
//getSubsetRecords ~~
//	takes as input a base path where the desired records are, 
//	  and a filter list.
//
// <filterList> 
//	filterList := relative-to-record-path=value(,value,...);path=value... filtering
//		records with relative path not meeting all filter criteria
//		- can accept multiple values per field (values separated by commas) (i.e. OR)
//		- TODO -- fields/value pairs separated by : for OR (before AND in order of operations)
//		- fields/value pairs separated by ; for AND
//		e.g. "LinkToFETypeConfiguration=NIMPlus,TemplateUDP;FEInterfacePluginName=NIMPlusPlugin"
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
// 	maxDepth is used to force an end to search for common fields
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
//			obj.fieldColumnDataType
//			obj.fieldColumnDataChoicesArr[]
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
		var FieldColumnDataTypes = fields.getElementsByTagName("FieldColumnDataType");
		var FieldColumnDataChoices = fields.getElementsByTagName("FieldColumnDataChoices");
		//			obj.fieldColumnDataType
		//			obj.fieldColumnDataChoicesArr
		
		for(var i=0;i<FieldTableNames.length;++i)
		{
			var obj = {};
			obj.fieldTableName = DesktopContent.getXMLValue(FieldTableNames[i]);
			obj.fieldColumnName = DesktopContent.getXMLValue(FieldColumnNames[i]);
			obj.fieldRelativePath = DesktopContent.getXMLValue(FieldRelativePaths[i]);
			obj.fieldColumnType = DesktopContent.getXMLValue(FieldColumnTypes[i]);
			obj.fieldColumnDataType = DesktopContent.getXMLValue(FieldColumnDataTypes[i]);
			var FieldColumnDataChoicesArr = FieldColumnDataChoices[i].getElementsByTagName("FieldColumnDataChoice");
			obj.fieldColumnDataChoicesArr = [];
			for(var j=0; j<FieldColumnDataChoicesArr.length;++j)
				obj.fieldColumnDataChoicesArr.push(DesktopContent.getXMLValue(FieldColumnDataChoicesArr[i]));
			recFields.push(obj);
		}
		Debug.log("Records length: " + recFields.length);		
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
//getUniqueFieldValuesForRecords ~~
//	takes as input a base path where the records are, 
//	  and an array of records.
// <recordArr> is an array or record UIDs (as returned from 
//		ConfigurationAPI.getSubsetRecords)
// <fieldList> is a CSV list of tree paths relative to <subsetBasePath>/<recordUID>/ 
//	 to the fields for which to get the set of unique values. 
//	If empty, then expect an empty array.
//		e.g. "LinkToFETypeConfiguration,FEInterfacePluginName"
//
// <modifiedTables> is an array of Table objects (as returned from 
//		ConfigurationAPI.setFieldValuesForRecords)
//
//	when complete, the responseHandler is called with an array parameter.
//		on failure, the array will be empty.
//		on success, the array will be an array of UniqueValues objects	
//		UniqueValues := {}
//			obj.fieldName 
//			obj.fieldUniqueValueArray
//			
//
ConfigurationAPI.getUniqueFieldValuesForRecords = function(subsetBasePath,recordArr,fieldList,
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
	
	DesktopContent.XMLHttpRequest("Request?RequestType=getUniqueFieldValuesForRecords" + 
			"&configGroup=" +
			"&configGroupKey=-1", //end get data
			"startPath=/" + subsetBasePath + 
			"&recordList=" + recordListStr +  
			"&fieldList=" + fieldList +
			"&modifiedTables=" + modifiedTablesListStr, //end post data
			function(req)
			{
		var fieldUniqueValues = [];
		var err = DesktopContent.getXMLValue(req,"Error");
		if(err) 
		{
			Debug.log(err,Debug.HIGH_PRIORITY);
			responseHandler(fieldUniqueValues);
			return;
		}
		
		var fields = req.responseXML.getElementsByTagName("field");

		for(var i=0;i<fields.length;++i)
		{
			
			var uniqueValues = fields[i].getElementsByTagName("uniqueValue");
			
			var obj = {};
			obj.fieldName = DesktopContent.getXMLValue(fields[i]);
			obj.fieldUniqueValueArray = [];
			for(var j=0;j<uniqueValues.length;++j)					
				obj.fieldUniqueValueArray.push(DesktopContent.getXMLValue(uniqueValues[j]));
			fieldUniqueValues.push(obj);
		}
		Debug.log("fieldUniqueValues length: " + fieldUniqueValues.length);		
		responseHandler(fieldUniqueValues);

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
//			obj.tableComment
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
		var tableComments = req.responseXML.getElementsByTagName("NewActiveTableComment");
		var tableVersion;
		
		//add only temporary version
		for(var i=0;i<tableNames.length;++i)
		{
			tableVersion = DesktopContent.getXMLValue(tableVersions[i])|0; //force integer
			if(tableVersion >= -1) continue; //skip unless temporary
			var obj = {};
			obj.tableName = DesktopContent.getXMLValue(tableNames[i]);
			obj.tableVersion = DesktopContent.getXMLValue(tableVersions[i]);
			obj.tableComment = DesktopContent.getXMLValue(tableComments[i]);
			modifiedTables.push(obj);
		}
		
		if(responseHandler) responseHandler(modifiedTables);

			}, //handler
			0, //handler param
			0,0,true); //progressHandler, callHandlerOnErr, showLoadingOverlay
}

//=====================================================================================
//popUpSaveModifiedTablesForm ~~
//	presents the user with the form to choose the options for ConfigurationAPI.saveModifiedTables
//	
//	When ConfigurationAPI.saveModifiedTables is called,
//		it will generate popup messages indicating progress.
//
// <modifiedTables> is an array of Table objects (as returned from 
//		ConfigurationAPI.setFieldValuesForRecords)
//
//	when complete, the responseHandler is called with 3 array parameters.
//		on failure, the arrays will be empty.
//		on success, the arrays will be an array of Saved Table objects	
//		SavedTable := {}
//			obj.tableName   
//			obj.tableVersion
//			obj.tableComment
//
//			...and array of Saved Group objects	
//		SavedGroup := {}
//			obj.groupName   
//			obj.groupKey
//			obj.groupComment
//
//			...and array of Saved Alias objects	
//		SavedAlias := {}
//			obj.groupName   
//			obj.groupKey
//			obj.groupAlias
//		
ConfigurationAPI.popUpSaveModifiedTablesForm = function(modifiedTables,responseHandler)
{	
	//mimic ConfigurationGUI::popUpSaveTreeForm()
	
	Debug.log("ConfigurationAPI popUpSaveModifiedTablesForm");	

	var str = "";

	var el = document.getElementById(ConfigurationAPI._POP_UP_DIALOG_ID);
	if(!el)
	{
		el = document.createElement("div");			
		el.setAttribute("id", ConfigurationAPI._POP_UP_DIALOG_ID);
	}
	el.style.display = "none";
	
	var gh = 50;
	var w = 380;
	var h = 330;
	ConfigurationAPI.setPopUpPosition(el,w /*w*/,h-gh*2 /*h*/);

	//set position and size
//	var w = 380;
//	var h = 330;
//	var gh = 50;
//	var ww = DesktopContent.getWindowWidth();
//	var wh = DesktopContent.getWindowHeight();
//	el.style.top = (DesktopContent.getWindowScrollTop() + ((wh-h-2)/2)- gh*2) + "px"; //allow for 2xgh growth for each affected group
//	el.style.left = (DesktopContent.getWindowScrollLeft() + ((ww-w-2)/2)) + "px";
//	el.style.width = w + "px";
//	el.style.height = h + "px";

	//always
	//	- save modified tables (show list of modified tables)
	//		(and which active group they are in)
	//
	//optionally
	//	- checkbox to bump version of modified active groups
	//	- checkbox to assign system alias to bumped active group 


	var modTblCount = 0;
	var modTblStr = "";
	var modifiedTablesListStr = ""; //csv table, temporay version,...
	
	for(var j=0;j<modifiedTables.length;++j)
		if((modifiedTables[j].tableVersion|0) < -1)
		{
			if(modTblCount++)
				modTblStr += ",";
			modTblStr += modifiedTables[j].tableName;

			if(modifiedTablesListStr.length)
				modifiedTablesListStr += ",";
			modifiedTablesListStr += modifiedTables[j].tableName;
			modifiedTablesListStr += ",";
			modifiedTablesListStr += modifiedTables[j].tableVersion;
		}

	var str = "<a id='" + 
			ConfigurationAPI._POP_UP_DIALOG_ID + 
			"-cancel' href='#'>Cancel</a><br><br>";

	str += "<div id='" + ConfigurationAPI._POP_UP_DIALOG_ID + "-div'>";
	str += "Saving will create new persistent versions of each modified table." + 
			"<br><br>" + 
			"Here is the list of modified tables (count=" + modTblCount + 
			"):" +					
			"<br>";


	//display modified tables
	str += "<div style='white-space:nowrap; width:" + w + "px; height:40px; " + 
			"overflow:auto; font-weight: bold;'>";
	str += modTblStr;
	str += "</div>";			

	//get affected groups
	//	and save member map to hidden div for Save action			
	///////////////////////////////////////////////////////////
	DesktopContent.XMLHttpRequest("Request?RequestType=getAffectedActiveGroups" +	
			"&groupName=" + 
			"&groupKey=-1", //end get params
			"&modifiedTables=" + modifiedTablesListStr, //end post params
			function(req) 
			{
		var err = DesktopContent.getXMLValue(req,"Error");
		if(err) 
		{					
			Debug.log(err,Debug.HIGH_PRIORITY);
			el.innerHTML = str;
			return;
		}

		//for each affected group
		//	 put csv: name,key,memberName,memberVersion...
		var groups = req.responseXML.getElementsByTagName("AffectedActiveGroup");				
		var memberNames, memberVersions;
		var xmlGroupName;
		modTblStr = ""; //re-use
		for(var i=0;i<groups.length;++i)
		{
			xmlGroupName = DesktopContent.getXMLValue(groups[i],"GroupName");
			str += "<div style='display:none' class='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
					"-affectedGroups' >";
			str += xmlGroupName;
			str += "," + DesktopContent.getXMLValue(groups[i],"GroupKey");

			memberNames = groups[i].getElementsByTagName("MemberName");
			memberVersions = groups[i].getElementsByTagName("MemberVersion");
			Debug.log("memberNames.length " + memberNames.length);
			for(var j=0;j<memberNames.length;++j)
				str += "," + DesktopContent.getXMLValue(memberNames[j]) + 
				"," + DesktopContent.getXMLValue(memberVersions[j]);
			str += "</div>"; //close div " + ConfigurationAPI._POP_UP_DIALOG_ID + "-affectedGroups


			if(modTblStr.length)
				modTblStr += ",";


			modTblStr += "<a style='color:black' href='#' onclick='javascript:" +								
					"var forFirefox = ConfigurationAPI.handleGroupCommentToggle(\"" + 
					xmlGroupName + "\");" +								
					" ConfigurationAPI.handlePopUpHeightToggle(" + h + "," + gh + ");'>";
			modTblStr += xmlGroupName;
			modTblStr += "</a>";

			//store cached group comment in hidden html
			modTblStr += "<div id='" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupComment-" + 
					xmlGroupName + "' " +
					"class='" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupComment-cache' " + 
					"style='display:none'>" + 
					decodeURIComponent(DesktopContent.getXMLValue(groups[i],"GroupComment")) +
					"</div>";
		}

		str += "Please choose the options you want and click 'Save':" +
				"<br>";

		//add checkbox to save affected groups
		str += "<input type='checkbox' id='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-bumpGroupVersions' checked " +
				"onclick='ConfigurationAPI.handlePopUpHeightToggle(" + h + "," + gh + ");'>";
		//add link so text toggles checkbox
		str += "<a href='#' onclick='javascript:" +
				"var el = document.getElementById(\"" + ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-bumpGroupVersions\");" +
				"var forFirefox = (el.checked = !el.checked);" +
				" ConfigurationAPI.handlePopUpHeightToggle(" + h + "," + gh + "); return false;'>";
		str += "Save Affected Groups as New Keys";
		str += "</a>";
		str +=	"</input><br>";

		//add checkbox to activate saved affected groups
		str += "<input type='checkbox' id='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-activateBumpedGroupVersions' checked " +
				">";
		//add link so text toggles checkbox
		str += "<a href='#' onclick='javascript:" +
				"var el = document.getElementById(\"" + ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-activateBumpedGroupVersions\");" +
				"if(el.disabled) return false; " +
				"var forFirefox = (el.checked = !el.checked);" +
				"return false;'>";
		str += "Also Activate New Groups";
		str += "</a>";
		str +=	"</input><br>";

		str += "Here is the list of affected groups (count=" + groups.length + 
				"):" +					
				"<br>";

		//display affected groups
		str += "<div style='white-space:nowrap; width:" + w + "px; margin-bottom:20px; " + 
				"overflow:auto; font-weight: bold;'>";
		str += modTblStr;
		str += "<div id='clearDiv'></div>";
		str += "<center>";			

		str += "<div id='" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupComment-header'></div>";

		str += "<div id='clearDiv'></div>";

		str += "<textarea id='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-groupComment' rows='4' cols='50' " + 
				"style='width:417px;height:68px;display:none;margin:0;'>";					 
		str += ConfigurationAPI._DEFAULT_COMMENT;
		str += "</textarea>";
		str += "</center>";

		str += "</div>"; //end affected groups div		

		str += "<div id='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-groupAliasArea' ><center>";

		//get existing group aliases	
		///////////////////////////////////////////////////////////
		DesktopContent.XMLHttpRequest("Request?RequestType=getGroupAliases" +	
				"",
				"",
				function(req) 
				{
			var err = DesktopContent.getXMLValue(req,"Error");
			if(err) 
			{					
				Debug.log(err,Debug.HIGH_PRIORITY);
				el.innerHTML = str;
				return;
			}

			var aliases = req.responseXML.getElementsByTagName("GroupAlias");
			var aliasGroupNames = req.responseXML.getElementsByTagName("GroupName");
			var aliasGroupKeys = req.responseXML.getElementsByTagName("GroupKey");

			//for each affected group
			//	-Show checkbox for setting alias and dropdown for alias
			//	and a pencil to change dropdown to text box to free-form alias.
			//	-Also, identify if already aliased and choose that as default option
			//	in dropwdown.					
			var alias, aliasGroupName, aliasGroupKey;
			var groupName, groupKey;					
			var groupOptionIndex = []; //keep distance and index of option for each group, or -1 if none
			for(var i=0;i<groups.length;++i)
			{				
				groupOptionIndex.push([-1,0]); //index and distance

				groupName = DesktopContent.getXMLValue(groups[i],"GroupName");
				groupKey = DesktopContent.getXMLValue(groups[i],"GroupKey");

				//find alias
				modTblStr = ""; //re-use
				for(var j=0;j<aliasGroupNames.length;++j)
				{
					alias = DesktopContent.getXMLValue(aliases[j]);
					aliasGroupName = DesktopContent.getXMLValue(aliasGroupNames[j]);
					aliasGroupKey = DesktopContent.getXMLValue(aliasGroupKeys[j]);	

					//Debug.log("compare " + aliasGroupName + ":" +
					//		aliasGroupKey);

					//also build drop down
					modTblStr += "<option value='" + alias + "' ";

					//consider any alias with same groupName
					if(aliasGroupName == groupName)
					{
						if(groupOptionIndex[i][0] == -1 ||	//take best match
								Math.abs(groupKey - aliasGroupKey) < groupOptionIndex[i][1])
						{
							Debug.log("found alias");
							groupOptionIndex[i][0] = j; //index
							groupOptionIndex[i][1] = Math.abs(groupKey - aliasGroupKey); //distance
						}
					}
					modTblStr += ">";
					modTblStr += alias;	//can display however
					modTblStr += "</option>";
				}

				str += "<input type='checkbox' class='" + ConfigurationAPI._POP_UP_DIALOG_ID + "-setGroupAlias' " +
						(groupOptionIndex[i][0] >= 0?"checked":"") + //check if has an alias already
						">";
				//add link so text toggles checkbox
				str += "<a href='#' onclick='javascript:" +
						"var el = document.getElementsByClassName(\"" + ConfigurationAPI._POP_UP_DIALOG_ID + "-setGroupAlias\");" +
						"var forFirefox = (el[" + i + "].checked = !el[" + i + "].checked);" +
						" return false;'>";
				str += "Set '<b style='font-size:16px'>" + groupName + "</b>' to System Alias:";
				str += "</a><br>";

				str += "<table cellpadding='0' cellspacing='0' border='0'><tr><td>";
				str += "<select " +
						"id='" + ConfigurationAPI._POP_UP_DIALOG_ID + "-editAliasSelect-" + (i) + "' " +
						"style='margin:2px; height:" + (25) + "px'>";						
				str += modTblStr;
				str += "</select>";						

				str += "<input type='text' " +
						"id='" + ConfigurationAPI._POP_UP_DIALOG_ID + "-editAliasTextBox-" + (i) + "' " +
						"style='display:none; margin:2px; width:150px; height:" + 
						(19) + "px'>";						
				str += "";
				str += "</input>";	
				str += "</td><td>";

				str += "<div style='display:block' " + 
						"class='" + ConfigurationAPI._POP_UP_DIALOG_ID + "-editIcon' id='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
						"-editIcon-" +
						(i) + "' " +
						"onclick='ConfigurationAPI.handlePopUpAliasEditToggle(" +							 
						i + 
						");' " +
						"title='Toggle free-form system alias editing' " +
						"></div>";
				
				str += "<div class='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
						"-preloadImage' id='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
						"-preloadImage-editIconHover'></div>";

				str += "</td></tr></table>";

				str +=	"</input>";

				//increase height each time a group check is added
				h += gh;
				el.style.height = h + "px";						
			}

			str += "</center></div>"; //close id='" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupAliasArea'


			// done with system alias handling
			// continue with pop-up prompt

			str += "</div><br>"; //close main popup div
//
//			var onmouseupJS = "";
//			onmouseupJS += "document.getElementById(\"" + ConfigurationAPI._POP_UP_DIALOG_ID + "-submitButton\").disabled = true;";
//			onmouseupJS += "ConfigurationAPI.handleGroupCommentToggle(0,1);"; //force cache of group comment
//			onmouseupJS += "ConfigurationAPI.handlePopUpHeightToggle(" + h + "," + gh + ");";
//			onmouseupJS += "ConfigurationAPI.saveModifiedTables();";					

			str += "<input id='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
					"-submitButton' type='button' " + //onmouseup='" + 
					//onmouseupJS + "' " +
					"value='Save' title='" +
					"Save new versions of every modified table\n" +
					"(Optionally, save new active groups and assign system aliases)" +
					"'/>";
			el.innerHTML = str;
			
			//create submit onmouseup handler
			document.getElementById(ConfigurationAPI._POP_UP_DIALOG_ID + 
					"-submitButton").onmouseup = function() {
				Debug.log("Submit mouseup");
				this.disabled = true;
				ConfigurationAPI.handleGroupCommentToggle(0,1); //force cache of group comment
				ConfigurationAPI.handlePopUpHeightToggle(h,gh);

				var savingGroups = 
						document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + 
								"-bumpGroupVersions").checked;
				var activatingSavedGroups = 
						document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + 
								"-activateBumpedGroupVersions").checked;

				ConfigurationAPI.saveModifiedTables(modifiedTables,responseHandler,
						true); //doNotIgnoreWarnings							

			}; //end submit onmouseup handler	

			//create cancel onclick handler
			document.getElementById(ConfigurationAPI._POP_UP_DIALOG_ID + 
					"-cancel").onclick = function(event) {
				Debug.log("Cancel click");
				var el = document.getElementById(ConfigurationAPI._POP_UP_DIALOG_ID);
				if(el) el.parentNode.removeChild(el); //close popup											
				responseHandler([],[],[]); //empty array indicates nothing done
				return false;
			}; //end submit onmouseup handler	


			//handle default dropdown selections for group alias
			for(var i=0;i<groups.length;++i)
				if(groupOptionIndex[i][0] != -1) //have a default
					document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-editAliasSelect-" + 
							i).selectedIndex = groupOptionIndex[i][0];

				},0,0,0,true //reqParam, progressHandler, callHandlerOnErr, showLoadingOverlay
		); //end of getGroupAliases handler

			},0,0,0,true //reqParam, progressHandler, callHandlerOnErr, showLoadingOverlay
	); //end of getActiveConfigGroups handler			


	document.body.appendChild(el); //add element to display div
	el.style.display = "block";

}


//=====================================================================================
//handleGroupCommentToggle ~~		
// toggles affected group comment box and handles details
ConfigurationAPI.handleGroupCommentToggle =  function(groupName,setHideVal)
{
	var el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupComment");			
	var hel = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupComment-header");
	var cel;
	
	var doHide = el.style.display != "none";
	if(setHideVal !== undefined)
		doHide = setHideVal;

	if(doHide) //cache (possibly modified) group comment
	{
		if(hel.textContent == "") return; //assume was a force hide when already hidden

		//get current groupName so we know where to cache comment
		var gn = hel.textContent.split("'")[1];
		Debug.log("gn " + gn);
		cel = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupComment-" + 
				gn);				
		cel.innerHTML = "";
		cel.appendChild(document.createTextNode(el.value));

		//setup group comment header properly
		hel.innerHTML = "";
		el.style.display = "none";

		//if for sure hiding, then done
		if(gn == groupName || setHideVal !== undefined) 
			return;
		//else show immediately the new selection
	}

	//show groupName comment
	{
		cel = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupComment-" + 
				groupName);
		el.value = cel.textContent;
		el.style.display = "block"; //show display before set caret (for Firefox)
		ConfigurationAPI.setCaretPosition(el,0,cel.textContent.length);

		hel.innerHTML = ("&apos;" + groupName + "&apos; group comment:");				
	}
}

//=====================================================================================
//handlePopUpHeightToggle ~~
//	checkbox was already set before this function call
//	this responds to current value
//
//	pass height and group height
ConfigurationAPI.handlePopUpHeightToggle = function(h,gh)
{
	var el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-bumpGroupVersions");			
	Debug.log("ConfigurationAPI.handlePopUpHeightToggle " + el.checked);

	var ael = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-activateBumpedGroupVersions");	

	var groupCommentEl = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupComment");
	var groupCommentHeight = 0;

	if(groupCommentEl && groupCommentEl.style.display != "none")
		groupCommentHeight += 100;

	var popEl = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "");			
	if(!el.checked)
	{
		//hide alias area and subtract the height

		document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupAliasArea").style.display = "none";								
		popEl.style.height = (h + groupCommentHeight) + "px";		
		ael.disabled = true;
	}
	else
	{
		//show alias area and add the height

		//count if grps is 1 or 2
		var grps = document.getElementsByClassName("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-affectedGroups");				
		popEl.style.height = (h + grps.length*gh + groupCommentHeight) + "px";
		document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-groupAliasArea").style.display = "block";
		ael.disabled = false;
	}			
}

//=====================================================================================
//handlePopUpAliasEditToggle ~~
ConfigurationAPI.handlePopUpAliasEditToggle = function(i) 
{	
	Debug.log("handlePopUpAliasEditToggle " + i);

	var sel = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-editAliasSelect-"+i);
	var tel = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-editAliasTextBox-"+i);
	Debug.log("sel.style.display  " + sel.style.display);
	if(sel.style.display == "none")
	{
		sel.style.display = "block";
		tel.style.display = "none";
	}
	else
	{
		tel.style.width = (sel.offsetWidth-2) + "px";
		sel.style.display = "none";
		tel.style.display = "block";
		ConfigurationAPI.setCaretPosition(tel,0,tel.value.length);
	}
}

//=====================================================================================
//saveModifiedTables ~~
//	Takes as input an array of modified tables and saves
//		those table temporary versions to persistent versions.
//		Optionally, save/activate affected groups and setup associated aliases.
//
//	By default, it will ignore warnings, save affected groups, and save 
//		the system aliases for affected groups (most similar system alias)
//
//	It will also generate popup messages indicating progress.
//
// <modifiedTables> is an array of Table objects (as returned from 
//		ConfigurationAPI.setFieldValuesForRecords)
//	
//	Note: when called from popup, uses info from popup.
//
//	when complete, the responseHandler is called with 3 array parameters.
//		on failure, the arrays will be empty.
//		on success, the arrays will be an array of Saved Table objects	
//		SavedTable := {}
//			obj.tableName   
//			obj.tableVersion
//			obj.tableComment
//
//			...and array of Saved Group objects	
//		SavedGroup := {}
//			obj.groupName   
//			obj.groupKey
//			obj.groupComment
//
//			...and array of Saved Alias objects	
//		SavedAlias := {}
//			obj.groupName   
//			obj.groupKey
//			obj.groupAlias
//
//
ConfigurationAPI.saveModifiedTables = function(modifiedTables,responseHandler,
		doNotIgnoreWarnings,doNotSaveAffectedGroups,
		doNotActivateAffectedGroups,doNotSaveAliases)
{	
	//copy from ConfigurationGUI::saveModifiedTree

	var savedTables = [];	
	var savedGroups = [];	
	var savedAliases = [];	
	
	if(!modifiedTables.length)
	{
		Debug.log("No tables were modified. Nothing to do.", Debug.WARN_PRIORITY);
		responseHandler(savedTables,savedGroups,savedAliases);
		return;
	}
	
	//for each modified table
	//	save new version
	//	update tree member table version based on result
	//if saving groups
	//	for each affected group
	//		save member list but with tree table versions
	//		modify root group name if changed
	//if saving aliases
	//	for each alias
	//		set alias for new group
	
	
	var numberOfRequests = 0;
	var numberOfReturns = 0;
	var allRequestsSent = false;		

	//localHandleAffectedGroups ~~
	function localHandleAffectedGroups()
	{
		Debug.log("Done with table saving.");

		//check if saving groups
		var savingGroups;
		var activatingSavedGroups;
		var doRequestAffectedGroups = false;
		try
		{
			savingGroups =
				document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-bumpGroupVersions").checked;
		
			activatingSavedGroups = 
				document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-activateBumpedGroupVersions").checked;
		}
		catch(err)
		{
			savingGroups = !doNotSaveAffectedGroups;
			activatingSavedGroups = !doNotActivateAffectedGroups;
			doRequestAffectedGroups = true; //popup doesn't exist, so need to do the work on own
		}

		if(!savingGroups) //then no need to proceed. exit!
		{
			//kill popup dialog
			var el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + ""); 
			if(el) el.parentNode.removeChild(el);	
			responseHandler(savedTables,savedGroups,savedAliases);
			return;
		}

		//identify root group name/key
		//var rootGroupEl = document.getElementById("treeView-ConfigGroupLink");
		//var rootGroupName = rootGroupEl.childNodes[0].textContent;
		//var rootGroupKey = rootGroupEl.childNodes[1].textContent;

		//Debug.log("rootGroup = " + rootGroupName + "(" + rootGroupKey + ")");
		Debug.log("On to saving groups");

		numberOfRequests = 0;		//re-use
		numberOfReturns = 0;		//re-use
		allRequestsSent = false; 	//re-use

		var affectedGroupNames = []; //to be populated for use by alias setting
		var affectedGroupComments = []; //to be populated for use by alias setting
		var affectedGroupConfigMap = []; //to be populated for use by alias setting
		
		var affectedGroupKeys = []; //to be populated after group save for use by alias setting
		
		if(doRequestAffectedGroups)
		{
			Debug.log("FIXME -- Need to replace temporary versions with new persistent versions",Debug.HIGH_PRIORITY);
			var modifiedTablesListStr = ""; //csv table, temporay version,...

			for(var j=0;j<modifiedTables.length;++j)
				if((modifiedTables[j].tableVersion|0) < -1)
				{
					if(modTblCount++)
						modTblStr += ",";
					modTblStr += modifiedTables[j].tableName;

					if(modifiedTablesListStr.length)
						modifiedTablesListStr += ",";
					modifiedTablesListStr += modifiedTables[j].tableName;
					modifiedTablesListStr += ",";
					modifiedTablesListStr += modifiedTables[j].tableVersion;
				}
			
			//get affected groups
			//	and save member map to hidden div for Save action			
			///////////////////////////////////////////////////////////
			DesktopContent.XMLHttpRequest("Request?RequestType=getAffectedActiveGroups" +	
					"&groupName=" + 
					"&groupKey=-1", //end get params
					"&modifiedTables=" + modifiedTablesListStr, //end post params
					function(req) 
					{
				var err = DesktopContent.getXMLValue(req,"Error");
				if(err) 
				{					
					Debug.log(err,Debug.HIGH_PRIORITY);
					el.innerHTML = str;
					responseHandler(savedTables,savedGroups,savedAliases);
					return;
				}
				//for each affected group
				//	 put csv: name,key,memberName,memberVersion...
				var groups = req.responseXML.getElementsByTagName("AffectedActiveGroup");				
				var memberNames, memberVersions;
				var xmlGroupName;
				modTblStr = ""; //re-use
				for(var i=0;i<groups.length;++i)
				{					
					affectedGroupNames.push( DesktopContent.getXMLValue(groups[i],"GroupName"));
					affectedGroupComments.push(decodeURIComponent(DesktopContent.getXMLValue(groups[i],"GroupComment")));

					memberNames = groups[i].getElementsByTagName("MemberName");
					memberVersions = groups[i].getElementsByTagName("MemberVersion");

					Debug.log("memberNames.length " + memberNames.length);

					//build member config map
					affectedGroupConfigMap[i] = "configList=";
					var memberVersion, memberName;
					for(var j=0;j<memberNames.length;++j)		
					{
						memberVersion = DesktopContent.getXMLValue(memberVersions[j])|0; //force integer
						memberName = DesktopContent.getXMLValue(memberNames[j]);
						if(memberVersion < -1) //there should be a new modified version
						{
							Debug.log("affectedArr " + memberName + "-v" + memberVersion);
							//find version
							for(var k=0;k<savedTables.length;++k)
								if(memberName == savedTables[k].tableName)
								{
									Debug.log("found " + savedTables[k].tableName + "-v" +
											savedTables[k].tableVersion);
									affectedGroupConfigMap[i] += memberName + "," + 
											savedTables[k].tableVersion + ",";
									break;
								}
						}
						else
							affectedGroupConfigMap[i] += memberName + 
								"," + memberVersion + ",";
					}
				}
					},0,0,0,true //reqParam, progressHandler, callHandlerOnErr, showLoadingOverlay
			); //end of getAffectedActiveGroups req
		}
		else
		{
			var affectedGroupEls = 
					document.getElementsByClassName(ConfigurationAPI._POP_UP_DIALOG_ID + 
							"-affectedGroups");
			var affectedGroupCommentEls = 
					document.getElementsByClassName(ConfigurationAPI._POP_UP_DIALOG_ID + 
							"-groupComment-cache");						

			//	for each affected group element
			for(var i=0;i<affectedGroupEls.length;++i)
			{
				Debug.log(affectedGroupEls[i].textContent);
				Debug.log("group comment: " + affectedGroupCommentEls[i].textContent);
				
				var affectedArr = affectedGroupEls[i].textContent.split(','); 
				
				affectedGroupComments.push(affectedGroupCommentEls[i].textContent);
				affectedGroupNames.push(affectedArr[0]);	
				
				//build member config map
				affectedGroupConfigMap[i] = "configList=";
				//member map starts after group name/key (i.e. [2])
				for(var a=2;a<affectedArr.length;a+=2)								
					if((affectedArr[a+1]|0) < -1) //there should be a new modified version
					{
						Debug.log("affectedArr " + affectedArr[a] + "-v" + affectedArr[a+1]);
						//find version
						for(var k=0;k<savedTables.length;++k)
							if(affectedArr[a] == savedTables[k].tableName)
							{
								Debug.log("found " + savedTables[k].tableName + "-v" +
										savedTables[k].tableVersion);
								affectedGroupConfigMap[i] += affectedArr[a] + "," + 
										savedTables[k].tableVersion + ",";
								break;
							}
					}
					else //use existing version
						affectedGroupConfigMap[i] += affectedArr[a] + "," + affectedArr[a+1] + ",";
			}
			
		}
			
		
		//	for each affected group
		for(var i=0;i<affectedGroupNames.length;++i)
		{	
			reqStr = ""; //reuse
			reqStr = "Request?RequestType=saveNewConfigurationGroup" +
					"&groupName=" + affectedGroupNames[i] +
					"&allowDuplicates=1" +
					"&ignoreWarnings=" + (doNotIgnoreWarnings?0:1) + 
					"&groupComment=" + encodeURIComponent(affectedGroupComments[i]);
			Debug.log(reqStr);
			Debug.log(affectedGroupConfigMap[i]);

			++numberOfRequests;
			///////////////////////////////////////////////////////////
			DesktopContent.XMLHttpRequest(reqStr, affectedGroupConfigMap[i], 
					function(req,affectedGroupIndex) 
					{

				var attemptedNewGroupName = DesktopContent.getXMLValue(req,"AttemptedNewGroupName");
				var treeErr = DesktopContent.getXMLValue(req,"TreeErrors");
				if(treeErr) 
				{	
					Debug.log(treeErr,Debug.HIGH_PRIORITY);
					Debug.log("There were problems identified in the tree view of the " +
							"attempted new group '" +
							attemptedNewGroupName +
							"'.\nThe new group was not created.\n" +
							"(Note: Other tables and groups may have been successfully created, " +
							"and would have success indications below this error info)\n\n" +
							"You can save the group anyway (if you think it is a good idea) by clicking " +
							"the button in the pop-up dialog " +
							"'<u>Save Groups with Warnings Ignored</u>.' " +
							"\n\nOtherwise, you can hit '<u>Cancel</u>.' and fix the tree. " +
							"Below you will find the description of the problem:",																		
							Debug.HIGH_PRIORITY);

					//change dialog save button
					var el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-submitButton");
					if(el)
					{
						el.onmouseup = function() {
							Debug.log("Submit mouseup");
							this.disabled = true;
							ConfigurationAPI.handleGroupCommentToggle(0,1); //force cache of group comment
							ConfigurationAPI.handlePopUpHeightToggle(h,gh);

							var savingGroups = 
									document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + 
											"-bumpGroupVersions").checked;
							var activatingSavedGroups = 
									document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + 
											"-activateBumpedGroupVersions").checked;

							ConfigurationAPI.saveModifiedTables(modifiedTables,responseHandler,
									false, //doNotIgnoreWarnings
									doNotSaveAffectedGroups,
									doNotActivateAffectedGroups,doNotSaveAliases
							);							
						};
						el.value = "Save Groups with Warnings Ignored";
						el.disabled = false;
					}
					return;
				}

				var err = DesktopContent.getXMLValue(req,"Error");
				if(err) 
				{					
					Debug.log(err,Debug.HIGH_PRIORITY);

					//kill popup dialog
					var el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + ""); 
					if(el) el.parentNode.removeChild(el);	
					responseHandler(savedTables,savedGroups,savedAliases);
					return;
				}			

				++numberOfReturns;

				var newGroupKey = DesktopContent.getXMLValue(req,"ConfigurationGroupKey");									
				affectedGroupKeys.push(newGroupKey);
				
				{
					var obj = {};
					obj.groupName = attemptedNewGroupName;
					obj.groupKey = newGroupKey;
					obj.groupComment = affectedGroupComments[affectedGroupIndex];
					savedGroups.push(obj);
				}

				//need to modify root group name if changed
				Debug.log("Successfully created new group '" + attemptedNewGroupName + 
						" (" + newGroupKey + ")'", Debug.INFO_PRIORITY);

				//activate if option was selected
				if(activatingSavedGroups)
					ConfigurationAPI.activateGroup(attemptedNewGroupName,newGroupKey,
							false /* ignoreWarnings */);
				

				if(allRequestsSent && 
						numberOfReturns == numberOfRequests)
				{
					Debug.log("Done with group saving.");

					Debug.log("Moving on to Alias creation...");										

					//check each alias checkbox
					//	for each alias that is checked
					//		set alias for new group
					//if any aliases modified, save and activate backbone

					//get checkboxes
					var setAliasCheckboxes;
					try
					{
						setAliasCheckboxes = 
							document.getElementsByClassName("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-setGroupAlias");
					}
					catch(err)
					{
						//no popup, so take from input and set for all affected groups
						setAliasCheckboxes = [];
						for(var i in affectedGroupNames)
							setAliasCheckboxes.push({"checked" : ((!doNotSaveAliases)?1:0) });
					}

					var groupAlias, groupName, groupKey;
					var setAliasCheckboxIndex = -1;
					var groupAliasName, groupAliasVersion;

					//in order to set alias, we need:
					//	groupAlias
					//	groupName
					//	groupKey				

					//for each set alias checkbox that is checked
					//	modify the active group alias table one after the other

					//localNextAliasHandler
					//	uses setAliasCheckboxIndex to iterate through setAliasCheckboxes
					//	and send the next request to modify the activegroupAlias table
					//	sequentially
					function localNextAliasHandler(retParams) 
					{
						//first time there is no setAliasCheckboxIndex == -1
						if(setAliasCheckboxIndex >= 0) 
						{	
							if(retParams)
							{
								if(retParams.newBackbone)
								{
									Debug.log("Successfully modified the active Backbone group " +
											" to set the System Alias '" + groupAlias + "' to " +
											" refer to the current group '" + groupName + 
											" (" + groupKey + ").'" +
											"\n\n" +
											"Backbone group '" + retParams.groupName + " (" + 
											retParams.groupKey + ")' was created and activated.",
											Debug.INFO_PRIORITY);
									
									{
										var obj = {};
										obj.groupName = groupName;
										obj.groupKey = groupKey;
										obj.groupAlias = groupAlias;
										savedAliases.push(obj);
									}
								}
								else
									Debug.log("Success, but no need to create a new Backbone group. " +
											"An existing Backbone group " +
											" already has the System Alias '" + groupAlias + "' " +
											" referring to the current group '" + groupName + 
											" (" + groupKey + ").'" +
											"\n\n" +
											"Backbone group '" + retParams.groupName + " (" + 
											retParams.groupKey + ")' was activated.",
											Debug.INFO_PRIORITY);
							}
							else
							{										
								Debug.log("Process interrupted. Failed to modify the currently active Backbone!",Debug.HIGH_PRIORITY);

								//kill popup dialog
								var el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + ""); 
								if(el) el.parentNode.removeChild(el);	
								responseHandler(savedTables,savedGroups,savedAliases);
								return;
							}	

							++setAliasCheckboxIndex; //req back, so ready for next index
						}
						else
							setAliasCheckboxIndex = 0; //ready for first checkbox

						//get next affected group index			
						while(setAliasCheckboxIndex < setAliasCheckboxes.length &&
								!setAliasCheckboxes[setAliasCheckboxIndex].checked)
							Debug.log("Skipping checkbox " + (++setAliasCheckboxIndex));

						if(setAliasCheckboxIndex >= setAliasCheckboxes.length)
						{
							Debug.log("Done with alias checkboxes ");

							if(!retParams)//req) 
							{
								Debug.log("No System Aliases were changed, so Backbone was not modified. Done.");

								//kill popup dialog
								var el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + ""); 
								if(el) el.parentNode.removeChild(el);
								responseHandler(savedTables,savedGroups,savedAliases);
								return;
							}

							Debug.log("Saving and activating Backbone done.");

							//kill popup dialog
							var el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + ""); 
							if(el) el.parentNode.removeChild(el);	
							responseHandler(savedTables,savedGroups,savedAliases);
							return;	
						}	

						//get next alias
						{
							var el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-editAliasSelect-" +
									setAliasCheckboxIndex);
							if(el.style.display == "none")
							{
								//get value from text box
								el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + "-editAliasTextBox-" + 
										setAliasCheckboxIndex);					
							}
							groupAlias = el.value;
						}

						groupName = affectedGroupNames[setAliasCheckboxIndex];
						groupKey = affectedGroupKeys[setAliasCheckboxIndex];

						Debug.log("groupAlias = " + groupAlias);
						Debug.log("groupName = " + groupName);
						Debug.log("groupKey = " + groupKey);

						ConfigurationAPI.setGroupAliasInActiveBackbone(groupAlias,groupName,groupKey,
								"SaveWiz",
								localNextAliasHandler,										
								true); //request return parameters		
					}

					localNextAliasHandler();

					Debug.log("Aliases set in motion");
				}

					},i,0,0,true  //reqParam, progressHandler, callHandlerOnErr, showLoadingOverlay
			); //end save new group request								
		} //end affected group for loop

		allRequestsSent = true;
		if(numberOfRequests == 0) //no groups to save
		{
			//this could happen if editing tables with no current active groups
			Debug.log("There were no groups to save!", Debug.INFO_PRIORITY);

			//kill popup dialog
			var el = document.getElementById("" + ConfigurationAPI._POP_UP_DIALOG_ID + ""); 
			if(el) el.parentNode.removeChild(el);
		}
	}	//end localHandleAffectedGroups


	//go through each modified table
	//	if modified table
	//		save new version
	//		update return object based on result
	for(var j=0;j<modifiedTables.length;++j)
		if((modifiedTables[j].tableVersion|0) < -1) //for each modified table
		{
			var reqStr = "Request?RequestType=saveSpecificConfiguration" + 
					"&dataOffset=0&chunkSize=0" +  
					"&configName=" + modifiedTables[j].tableName + 
					"&version="+modifiedTables[j].tableVersion +	
					"&temporary=0" +
					"&tableComment=" + 
					encodeURIComponent(modifiedTables[j].tableComment?modifiedTables[j].tableComment:"") +
					"&sourceTableAsIs=1"; 
			Debug.log(reqStr);

			++numberOfRequests;

			//	save new version
			///////////////////////////////////////////////////////////
			DesktopContent.XMLHttpRequest(reqStr, "", 
					function(req,modifiedTableIndex) 
					{
				var err = DesktopContent.getXMLValue(req,"Error");
				if(err) 
				{					
					Debug.log(err,Debug.HIGH_PRIORITY);

					//kill popup dialog
					var el = document.getElementById(ConfigurationAPI._POP_UP_DIALOG_ID); 
					if(el) el.parentNode.removeChild(el);
					responseHandler(savedTables,savedGroups,savedAliases);
					return;
				}						

				var configName = DesktopContent.getXMLValue(req,"savedName");
				var version = DesktopContent.getXMLValue(req,"savedVersion");

				Debug.log("Successfully created new table '" + configName + "-v" + 
						version + "'",Debug.INFO_PRIORITY);
				
				//update saved table version based on result
				{
					var obj = {};
					obj.tableName = configName;
					obj.tableVersion = version;
					obj.tableComment = modifiedTables[modifiedTableIndex].tableComment;
					savedTables.push(obj);
				}				

				++numberOfReturns;

				if(allRequestsSent && 
						numberOfReturns == numberOfRequests)
				{
					if(!doNotSaveAffectedGroups)
						localHandleAffectedGroups();							
				}
					},j,0,0,true  //reqParam, progressHandler, callHandlerOnErr, showLoadingOverlay
			);	//end save new table request
		}	//end modified table for loop

	allRequestsSent = true;
	if(numberOfRequests == 0) //no requests were sent, so go on to affected groups
	{
		//localHandleAffectedGroups();
		Debug.log("No tables were modified. Should be impossible to get here.", Debug.HIGH_PRIORITY);
	}
}


//=====================================================================================
//activateGroup ~~
ConfigurationAPI.activateGroup = function(groupName, groupKey, 
		ignoreWarnings)
{
	DesktopContent.XMLHttpRequest("Request?RequestType=activateConfigGroup&groupName=" + 
			groupName + "&groupKey=" + groupKey +
			(ignoreWarnings?("&ignoreWarnings=" + ignoreWarnings):""),
			"",
			function(req) 
			{
		
		var err = DesktopContent.getXMLValue(req,"Error");
		if(err) 
		{
			Debug.log(err,Debug.HIGH_PRIORITY);				

			//Debug.log(_OTS_RELAUNCH_STR,Debug.INFO_PRIORITY);

			//show activate with warnings link
			var str = "";			

				//add ignore warnings Activate link
			str += " <a href='#' onclick='javascript:ConfigurationAPI.activateGroup(\"" + 
					groupName +
					"\",\"" + groupKey + "\",true); return false;'>"; //ignore warnings
			str += "Activate " + 
					groupName + "(" + groupKey + ") w/warnings ignored</a>";
			
			Debug.log("If you are are sure it is a good idea you can try to " +
					"activate the group with warnings ignored: " +
					str,Debug.HIGH_PRIORITY);
		}
			},
			true, 0 ,0, //reqIndex, progressHandler, callHandlerOnErr
			true); //showLoadingOverlay
}

//=====================================================================================
//setGroupAliasInActiveBackbone ~~
//	Used to set a group alias.
//	This function will activate the resulting backbone group and 
//		call a done handler	
//
//      if doReturnParms
//          then the handler is called with an object 
//          describing the new backbone group object:
//                 retParams.newBackbone //true if successfully activated
//                 retParams.groupName   //backbone group name 
//                 retParams.groupKey    //backbone group key
//               
ConfigurationAPI.setGroupAliasInActiveBackbone = function(groupAlias,groupName,groupKey,
		newBackboneNameAdd,doneHandler,doReturnParams)
{
	Debug.log("setGroupAliasInActiveBackbone groupAlias=" + groupAlias);
	Debug.log("setGroupAliasInActiveBackbone groupName=" + groupName);
	Debug.log("setGroupAliasInActiveBackbone groupKey=" + groupKey);

	if(!groupName || groupName == "" || !groupKey || groupKey == "")
	{
		Debug.log("Process interrupted. Invalid group name and key given!",Debug.HIGH_PRIORITY);
		doneHandler(); //error so call done handler
		return;
	}

	if(!newBackboneNameAdd || newBackboneNameAdd == "")
		newBackboneNameAdd = "Wiz";
	newBackboneNameAdd += "Backbone";
	Debug.log("setGroupAliasInActiveBackbone newBackboneNameAdd=" + newBackboneNameAdd);

	DesktopContent.XMLHttpRequest("Request?RequestType=setGroupAliasInActiveBackbone" +
			"&groupAlias=" + groupAlias + 
			"&groupName=" + groupName +
			"&groupKey=" + groupKey, "", 
			ConfigurationAPI.newWizBackboneMemberHandler,
			[("GroupAlias" + newBackboneNameAdd),doneHandler,doReturnParams],
			0,0,true  //progressHandler, callHandlerOnErr, showLoadingOverlay
	);					
}

//=====================================================================================
//newWizBackboneMemberHandler
//	Used to handle the response from modifying a member of the backbone.
//	This handler will activate the resulting backbone group and
//		call a done handler.
//
//	params = [newBackboneGroupName, doneHandler, doReturnParams]
ConfigurationAPI.newWizBackboneMemberHandler = function(req,params)
{
	var err = DesktopContent.getXMLValue(req,"Error");
	if(err) 
	{
		Debug.log(err,Debug.HIGH_PRIORITY);
		Debug.log("Process interrupted. Failed to modify the currently active Backbone!",Debug.HIGH_PRIORITY);

		if(params[1])
			params[1](); //error so call done handler
		return;
	}		

	var groupAliasName = DesktopContent.getXMLValue(req,"savedName");
	var groupAliasVersion = DesktopContent.getXMLValue(req,"savedVersion");

	Debug.log("groupAliasName=" + groupAliasName);
	Debug.log("groupAliasVersion=" + groupAliasVersion);

	var configNames = req.responseXML.getElementsByTagName("oldBackboneName");
	var configVersions = req.responseXML.getElementsByTagName("oldBackboneVersion");

	//make a new backbone with old versions of everything except Group Alias 
	var configMap = "configList=";
	var name;
	for(var i=0;i<configNames.length;++i)
	{		
		name = configNames[i].getAttribute("value");

		if(name == groupAliasName)
		{
			configMap += name + "," + 
					groupAliasVersion + ",";
			continue;
		}
		//else use old member
		configMap += name + "," + 
				configVersions[i].getAttribute("value") + ",";							
	}

	ConfigurationAPI.saveGroupAndActivate(params[0],configMap,params[1],params[2]);			
}

//=====================================================================================
//saveGroupAndActivate
ConfigurationAPI.saveGroupAndActivate = function(groupName,configMap,doneHandler,doReturnParams)
{
	DesktopContent.XMLHttpRequest("Request?RequestType=saveNewConfigurationGroup&groupName=" +
			groupName, configMap, 
			function(req)
			{
		var err = DesktopContent.getXMLValue(req,"Error");
		var name = DesktopContent.getXMLValue(req,"ConfigurationGroupName");
		var key = DesktopContent.getXMLValue(req,"ConfigurationGroupKey");
		var newBackbone = true;
		if(err) 
		{
			if(!name || !key)
			{
				Debug.log(err,Debug.HIGH_PRIORITY);
				Debug.log("Process interrupted. Failed to create a new Backbone group!" +
						" Please see details below.",
						Debug.HIGH_PRIORITY);

				if(doneHandler)
					doneHandler(); //error so call done handler
				return;
			}
			else
			{
				Debug.log(err,Debug.WARN_PRIORITY);
				Debug.log("Process interrupted. Failed to create a new Backbone group!" +
						" (Likely the currently active Backbone already represents what is being requested)\n\n" +
						"Going on with existing backbone group, name=" + name + " & key=" + key,
						Debug.WARN_PRIORITY);
				newBackbone = false;
			}					
		}

		//now activate the new backbone group

		DesktopContent.XMLHttpRequest("Request?RequestType=activateConfigGroup" +
				"&groupName=" + name +
				"&groupKey=" + key, "", 
				function(req)
				{
			var err = DesktopContent.getXMLValue(req,"Error");
			if(err) 
			{
				Debug.log(err,Debug.HIGH_PRIORITY);
				return;
			}

			try
			{
				activateSystemConfigHandler(req);
			}
			catch(err) {} //ignore error, this is only used by ConfigurationGUI (or anyone implementing this extra handler)

			if(doneHandler)
			{
				//done so call done handler (and indicate success)
				if(!doReturnParams)
					doneHandler(); //done so call done handler
				else
				{
					var retParams = {
							"groupName" : name,
							"groupKey" : key,
							"newBackbone" : newBackbone									
					}
					doneHandler(retParams); 	 //(and indicate success)
				}
			}

				},0,0,0,true  //reqParam, progressHandler, callHandlerOnErr, showLoadingOverlay
		);	//end of activate new backbone handler

			},0,0,0,true  //reqParam, progressHandler, callHandlerOnErr, showLoadingOverlay
	); //end of backbone saveNewConfigurationGroup handler
}

//=====================================================================================
//bitMapDialog ~~
//	shows bitmap dialog at full window size (minus margins)
//	on ok, calls <okHandler> with finalBitMapValue parameter
//
//	<bitMapParams> is an array olf size 6:
//		rows,cols,cellFieldSize,minColor,midColor,maxColor
ConfigurationAPI.bitMapDialog = function(fieldName,bitMapParams,initBitMapValue,okHandler,cancelHandler)
{	
	Debug.log("ConfigurationAPI bitMapDialog");	

	var str = "";

	var el = document.getElementById(ConfigurationAPI._POP_UP_DIALOG_ID);
	if(!el)
	{
		el = document.createElement("div");			
		el.setAttribute("id", ConfigurationAPI._POP_UP_DIALOG_ID);
	}
	el.style.display = "none";
	el.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmapDialog");

	var padding = 10;
	var popSz;	
	
				
	//create bit map dialog
	// 	- header at top with field name and parameters
	//	- bitMap, w/mouseover and click and drag (same as windows sw)
	//		- center vertically
	//  - OK, CANCEL buttons at top right
	//	- also upload download buttons

	// 	Input parameters must match Table Editor handling for bitmaps:
	//	var _bitMapFieldsArr = [0 "Number of Rows",	
	//							1 "Number of Columns",
	//							2 "Cell Bit-field Size",
	//							3 "Min-value Allowed",
	//							4 "Max-value Allowed",
	//							5 "Value step-size Allowed",
	//							6 "Display Aspect H:W",
	//							7 "Min-value Cell Color",
	//							8 "Mid-value Cell Color",
	//							9 "Max-value Cell Color",
	//							10 "Absolute Min-value Cell Color",
	//							11 "Absolute Max-value Cell Color",
	//							12 "Display Rows in Ascending Order",
	//							13 "Display Columns in Ascending Order",
	//							14 "Snake Double Rows",
	//							15 "Snake Double Columns"];
	
	
	//	
	//	Local functions:
	//		localCreateCancelClickHandler()
	//      localCreateOkClickHandler()
	//		localCreateMouseHandler()
	//			localGetRowCol(x,y)
	//			el.onmousemove
	//			el.onmousedown
	//			el.onmouseup
	//			el.oncontextmenu 
	//			localSetBitMap(r,c)
	//		localValidateInputs()
	//		localInitBitmapData()
	//		localConvertGridToRowCol(r,c)
	//		localConvertValueToRGBA(val)
	//		localConvertFullGridToRowCol()
	//		localConvertFullRowColToGrid(srcMatrix)
	//		localCreateBitmap()
	//		localCreateGridButtons()
	//		localCreateHeader()
	//			ConfigurationAPI.bitMapDialog.localUpdateScroll(i)
	//			ConfigurationAPI.bitMapDialog.localUpdateTextInput(i)
	//			ConfigurationAPI.bitMapDialog.localUpdateButtonInput(i,dir)
	//			ConfigurationAPI.bitMapDialog.localDownloadCSV()
	//			ConfigurationAPI.bitMapDialog.locaPopupUploadCSV()
	//			ConfigurationAPI.bitMapDialog.locaUploadCSV()
	//		localPaint()
	//		localOptimizeAspectRatio()

	var rows, cols;

	var bitFieldSize;
	var bitMask;
	
	var minValue, maxValue; 
	var midValue; //used for color calcs
	var stepValue;
	
	var forcedAspectH, forcedAspectW;
	
	var minValueColor, midValueColor, maxValueColor;
	var ceilValueColor, floorValueColor;

	var doDisplayRowsAscending, doDisplayColsAscending;
	var doSnakeColumns, doSnakeRows;
				
	//validate and load input params
	if(!localValidateInputs())
	{
		Debug.log("Input parameters array to the Bitmap Dialog was as follows:\n " +
				bitMapParams, Debug.HIGH_PRIORITY);
		Debug.log("Input parameters to the Bitmap Dialog are invalid. Aborting.", Debug.HIGH_PRIORITY);
		return cancelHandler();
	}
	
	//give 5 pixels extra for each digit necessary to label rows
	var numberDigitW = 8, numberDigitH = 12;
	var axisPaddingExtra = numberDigitW;
	function localCalcExtraAxisPadding() {
		var lrows = rows;
		while((lrows /= 10) > 1) axisPaddingExtra += numberDigitW; 
	} localCalcExtraAxisPadding();			
	var butttonSz = 20;
	var axisPaddingMargin = 5;
	var axisPadding = axisPaddingMargin + axisPaddingExtra + axisPaddingMargin + butttonSz + axisPaddingMargin;
	var bmpGridThickness = 1;
	var bmpBorderSize = 1;
	
	
	var hdr; //header element
	var hdrX; 
	var hdrY;
	var hdrW;
	var hdrH;
	
	var bmp;  //bitmap element
	var bmpGrid; //bitmap grid element
	var allRowBtns, allColBtns, allBtn;
	var rowLeftNums, rowRightNums, colTopNums, colBottomNums;
	var bmpCanvas, bmpContext; //used to generate 2D bitmap image src
	var bmpData; //bitmap data shown and returned. 2D array
	var bmpDataImage; //visual interpretation of bitmap data
	var bmpX; 
	var bmpY;	
	var bmpW;
	var bmpH;
	var bmpOverlay;
	var cursorInfo, hdrCursorInfo;

	var cellW;
	var cellH;
	
	var clickColors = []; //2 element array: 0=left-click, 1=right-click
	var clickValues = []; //2 element array: 0=left-click, 1=right-click

	
	localCreateHeader(); //create header element content	
	localCreateBitmap(); //create bitmap element and data
	localCreateGridButtons(); //create all buttons
	
	localInitBitmapData(); //load bitmap data from input string
	
	localPaint();
	window.addEventListener("resize",localPaint);
		
	document.body.appendChild(el); //add element to body
	el.style.display = "block";
	
	
	//:::::::::::::::::::::::::::::::::::::::::
	//localCreateCancelClickHandler ~~
	//	create cancel onclick handler
	function localCreateCancelClickHandler()
	{
		document.getElementById(ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-cancel").onclick = function(event) {
			Debug.log("Cancel click");
			var el = document.getElementById(ConfigurationAPI._POP_UP_DIALOG_ID);
			if(el) el.parentNode.removeChild(el); //close popup
			window.removeEventListener("resize",localPaint); //remove paint listener
			cancelHandler(); //empty array indicates nothing done
			return false;
		}; //end submit onmouseup handler				
	} localCreateCancelClickHandler();

	//:::::::::::::::::::::::::::::::::::::::::
	//localCreateOkClickHandler ~~
	//	create OK onclick handler
	function localCreateOkClickHandler()
	{
		var convertFunc = localConvertFullGridToRowCol;
		document.getElementById(ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-ok").onclick = function(event) {
			Debug.log("OK click");
			var el = document.getElementById(ConfigurationAPI._POP_UP_DIALOG_ID);
			if(el) el.parentNode.removeChild(el); //close popup
			window.removeEventListener("resize",localPaint); //remove paint listener
			
			var transGrid = convertFunc();
			var dataJsonStr = "[\n";
			for(var r=0;r<transGrid.length;++r)
			{
				if(r) dataJsonStr += ",\n";
				dataJsonStr += "\t[";
				for(var c=0;c<transGrid[0].length;++c)
				{
					if(c) dataJsonStr += ",";
					dataJsonStr += transGrid[r][c];
				}
				dataJsonStr += "]";
			}
			dataJsonStr += "\n]";
			okHandler(dataJsonStr); //empty array indicates nothing done
			return false;
		}; //end submit onmouseup handler				
	} localCreateOkClickHandler();

	//:::::::::::::::::::::::::::::::::::::::::
	//localCreateMouseHandler ~~
	//	create mouseover handler
	function localCreateMouseHandler()
	{
		var stopProp = false; //used to stop contextmenu propagation
		var rLast = -1, cLast = -1; //to stop redoing calculations in mouse over
				//-2 is special all buttons
		var buttonDown = -1; //0 - left, 1 - middle, 2 - right

		//::::::::
		//localGetRowCol ~~
		//	returns -1 in r and c for nothing interesting
		//	returns -2 for special all buttons
		//	else returns r,c of cell identified by x,y
		function localGetRowCol(x,y) {
			x -= popSz.x + bmpX + 1;
			y -= popSz.y + bmpY + 1;
			var r = (y/cellH)|0;
			if(y < 0) r = -1; //handle negative fractions clipping to 0
			var c = (x/cellW)|0;
			if(x < 0) c = -1; //handle negative fractions clipping to 0
			var inRowBtnsX = (x >= - axisPaddingMargin - bmpBorderSize - butttonSz) && 
					(x <= - axisPaddingMargin - bmpBorderSize);
			var inColBtnsY = (y >= bmpH + axisPaddingMargin) && 
					(y <= bmpH + axisPaddingMargin + butttonSz + bmpBorderSize*2);

			//Debug.log("i x,y " + x + "," + y);
			//Debug.log("i r,c " + r + "," + c);
			//Debug.log("inRowBtnsX " + inRowBtnsX);
			//Debug.log("inColBtnsY " + inColBtnsY);
			
			//handle row buttons
			if(inRowBtnsX && r >= 0 && r < rows)
				return {"r":r, "c":-2};
			else if(inColBtnsY && c >= 0 && c < cols) //handle col buttons
				return {"r":-2, "c":c};
			else if(inRowBtnsX && inColBtnsY) //handle all button
				return {"r":-2, "c":-2};
			else if(r < 0 || c < 0 || r >= rows || c >= cols)
				return {"r":-1, "c":-1}; //is nothing
			return {"r":r, "c":c}; //else, is a cell
		} //end localGetRowCol()

		//::::::::
		//el.onmousemove ~~
		el.onmousemove = function(event) {
			var cell = localGetRowCol(event.pageX,event.pageY);
			var r = cell.r, c = cell.c;

			var cursorT = (event.pageX - popSz.x - bmpX);
			if(cursorT < 0) cursorT = 0;
			if(cursorT > bmpW) cursorT = bmpW;
			
			cursorInfo.style.left = (event.pageX - popSz.x + 
					//(c >= cols/2?-cursorInfo.innerHTML.length*8-20:2)) + 
					//smooth transition from left-most to right-most info position above cursor
					(cursorT)/bmpW*(-cursorInfo.innerHTML.length*8-20) + (bmpW-cursorT)/bmpW*(2))+
							"px";
			cursorInfo.style.top = (event.pageY - popSz.y - 35) + "px";

			//center header cursor info
			hdrCursorInfo.style.left = (bmpX + bmpW/2 +
					(-332)/2) + "px"; //hdrCursorInfo.style.width = "320px"; + padding*2 + border*2
			hdrCursorInfo.style.top = (bmpY - 45) + "px";
			
			
			if(rLast == r && cLast == c)
				return; //already done for this cell, so prevent excessive work 
			rLast = r; cLast = c;

			if(r == -1 || c == -1) //handle no select case
			{
				//mouse off interesting things
				rLast = -1; cLast = -1;
				bmpOverlay.style.display = "none";
				cursorInfo.style.display = "none";
				hdrCursorInfo.style.display = "none";
				return;				
			}

			cursorInfo.style.display = "block";
			//hdrCursorInfo.style.display = "block"; //removed from display.. could delete if unneeded?

			var transRC;
			var infoStr;

			//handle row buttons
			if(r != -2 && c == -2)
			{			
				if(doSnakeColumns)
					transRC = localConvertGridToRowCol(r,
							doDisplayColsAscending?0:cols-1);
				else
					transRC = localConvertGridToRowCol(r,0);
				
				//make mouse over bitmap			
				{
					bmpOverlay.src = ConfigurationAPI.getOnePixelPngData([216,188,188]);//canvas.toDataURL();

					bmpOverlay.style.left = (bmpX - axisPaddingMargin - bmpBorderSize - butttonSz) + "px";
					bmpOverlay.style.top = (bmpY + r*cellH - 1 + (r?bmpGridThickness+bmpBorderSize*2:0)) + "px";
					bmpOverlay.style.width = (butttonSz) + "px";
					bmpOverlay.style.height = (cellH - (r?bmpGridThickness+bmpBorderSize*2:0)) + "px";
					bmpOverlay.style.display = "block";
				}	

				infoStr = "Set all pixels in row " + transRC[0] + ".";
			}
			else if(r == -2 && c != -2) //handle col buttons
			{				
				if(doSnakeRows)
					transRC = localConvertGridToRowCol(
							doDisplayRowsAscending?0:rows-1,c);
				else
					transRC = localConvertGridToRowCol(0,c);
				
				
				//make mouse over bitmap			
				{
					bmpOverlay.src = ConfigurationAPI.getOnePixelPngData([216,188,188]);//canvas.toDataURL();

					bmpOverlay.style.left = (bmpX + c*cellW - 1 + (c?bmpGridThickness+bmpBorderSize*2:0)) + "px";
					bmpOverlay.style.top = (bmpY + bmpH + axisPaddingMargin - bmpBorderSize) + "px";
					bmpOverlay.style.width = (cellW + 1 - (c?bmpGridThickness+bmpBorderSize*2:0)) + "px";
					bmpOverlay.style.height = (butttonSz) + "px";
					bmpOverlay.style.display = "block";
				}

				infoStr = "Set all pixels in column " + transRC[1] + ".";
			}
			else if(r == -2 && c == -2) //handle all button
			{
				//make mouse over bitmap			
				{
					bmpOverlay.src = ConfigurationAPI.getOnePixelPngData([216,188,188]);//canvas.toDataURL();

					bmpOverlay.style.left = (bmpX - axisPaddingMargin - bmpBorderSize - butttonSz) + "px";
					bmpOverlay.style.top = (bmpY + bmpH + axisPaddingMargin - bmpBorderSize) + "px";
					bmpOverlay.style.width = (butttonSz) + "px";
					bmpOverlay.style.height = (butttonSz) + "px";
					bmpOverlay.style.display = "block";
				} 
				
				infoStr = "Set all pixels.";
			}
			else //pixel case
			{					
				transRC = localConvertGridToRowCol(r,c);
				
				//have mouse over bitmap			
				{
					//make a partial alpha overlay that lightens or darkens
					//	depending on pixel color
					var overClr = (bmpDataImage.data[(r*cols+c)*4+0] +
							bmpDataImage.data[(r*cols+c)*4+1] + 
							bmpDataImage.data[(r*cols+c)*4+2]) < (256+128)?255:0;
					
					bmpOverlay.src = ConfigurationAPI.getOnePixelPngData(
							[overClr,overClr,overClr,100]);
					
					bmpOverlay.style.left = (bmpX + c*cellW) + "px";
					bmpOverlay.style.top = (bmpY + r*cellH) + "px";
					bmpOverlay.style.width = (cellW) + "px";
					bmpOverlay.style.height = (cellH) + "px";
					bmpOverlay.style.display = "block";				
				}	
				
				//position cursor info
				infoStr = "Value = " + bmpData[r][c]  + " @ (Row,Col) = (" + 
						transRC[0] + "," + transRC[1] + ")";
			}
			cursorInfo.innerHTML = infoStr;
			hdrCursorInfo.innerHTML = infoStr;
			
			//Debug.log("r,c " + r + "," + c);
			if(r == -2 && c == -2)
				return;	//prevent mouse over execution of all button (assume it's accidental)
			
			if(buttonDown >= 0)
			{
				stopProp = true;			
				localSetBitMap(r,c); //set bitmap data
			}
			
		} //end mouse move
		
		//::::::::
		//el.onmousedown ~~
		el.onmousedown = function(event) {

			var cell = localGetRowCol(event.pageX,event.pageY);
			var r = cell.r, c = cell.c;
			
			//Debug.log("click which " + event.which);
			//Debug.log("click button " + event.button);
			buttonDown = event.button;			

			if(r == -1 || c == -1) //handle no select case
			{
				rLast = -1; cLast = -1; //reset for mouse move
				stopProp = false;
				return;				
			}
			
			rLast = r; cLast = c;			
			localSetBitMap(r,c); //set bitmap data

			stopProp = true;
			event.stopPropagation();
			
		} //end mouse down

		//::::::::
		//el.onmouseup ~~
		el.onmouseup = function(event) {
			//Debug.log("click up ");
			buttonDown = -1;
		} //end mouse up

		//::::::::
		//el.oncontextmenu ~~
		el.oncontextmenu = function(event) {
			//Debug.log("click stopProp " + stopProp);
			
			if(stopProp)
			{
				stopProp = false;
				event.stopPropagation();			
				return false;
			}
		} //end oncontextmenu

		//::::::::
		//localSetBitMap ~~
		function localSetBitMap(r,c) {			
			
			Debug.log("set r,c " + buttonDown + " @ " + r + "," + c );
			buttonDown = buttonDown?1:0; //	0=left-click, 1=right-click
			
			var maxr = r==-2?rows-1:r;
			var minr = r==-2?0:r;
			var maxc = c==-2?cols-1:c;
			var minc = c==-2?0:c;
						
			for(r=minr;r<=maxr;++r)
				for(c=minc;c<=maxc;++c)
				{
					bmpData[r][c] = clickValues[buttonDown];
					bmpDataImage.data[(r*cols + c)*4 + 0] = 
							clickColors[buttonDown][0];
					bmpDataImage.data[(r*cols + c)*4 + 1] = 
							clickColors[buttonDown][1];
					bmpDataImage.data[(r*cols + c)*4 + 2] =
							clickColors[buttonDown][2];
					bmpDataImage.data[(r*cols + c)*4 + 3] = 
							clickColors[buttonDown][3];
				}
			
			bmpContext.putImageData(bmpDataImage,0,0);
			bmp.src = bmpCanvas.toDataURL();
		}// end localSetBitMap
		
	} localCreateMouseHandler();
		
	//:::::::::::::::::::::::::::::::::::::::::
	//localValidateInputs ~~
	//	returns false if inputs are invalid
	//	else true.
	function localValidateInputs() {

		//veryify bitmap params is expected size
		if(bitMapParams.length != 16)
		{
			Debug.log("Illegal input parameters, expecting 16 parameters and count is " + bitMapParams.length + ". There is a mismatch in Table Editor handling of BitMap fields (contact an admin to fix)." + 
					"\nHere is a printout of the input parameters: " + bitMapParams,Debug.HIGH_PRIORITY);			
			return false;
		}	
		var DEFAULT = "DEFAULT";
		
		rows = bitMapParams[0]|0;
		cols = bitMapParams[1]|0;
		bitFieldSize = bitMapParams[2]|0;

		//js can only handle 31 bits unsigned!! hopefully no one needs it?
		
		if(rows < 1 || rows >= 1<<30)
		{
			Debug.log("Illegal input parameters, rows of " + rows + " is illegal. " +
					"(rows possible values are from 1 to " + ((1<<30)-1) + ".)",Debug.HIGH_PRIORITY);
			return false;
		}
		if(cols < 1 || cols >= 1<<30)
		{
			Debug.log("Illegal input parameters, cols of " + cols + " is illegal. " +
					"(cols possible values are from 1 to " + ((1<<30)-1) + ".)",Debug.HIGH_PRIORITY);
			return false;
		}
		if(bitFieldSize < 1 || bitFieldSize > 31)
		{
			Debug.log("Illegal input parameters, bitFieldSize of " + bitFieldSize + " is illegal. " +
					"(bitFieldSize possible values are from 1 to " + (31) + ".)",Debug.HIGH_PRIORITY);
			return false;
		}

		
		if(bitFieldSize > 30)
		{			
			bitMask = 0; 
			for(var i=0;i<bitFieldSize;++i)
				bitMask |= 1 << i; 
		}
		else
			bitMask = (1<<bitFieldSize) - 1; //wont work for 31 bits (JS is always signed)		

		minValue = bitMapParams[3] == "DEFAULT" || bitMapParams[3] == ""?0:(bitMapParams[3]|0);
		maxValue = bitMapParams[4] == "DEFAULT" || bitMapParams[4] == ""?bitMask:(bitMapParams[4]|0);
		if(maxValue < minValue)
			maxValue = bitMask;	
		midValue = (maxValue + minValue)/2; //used for color calcs
		stepValue = bitMapParams[5] == "DEFAULT" || bitMapParams[5] == ""?1:(bitMapParams[5]|0);
		
		if(minValue < 0 || minValue > bitMask)
		{
			Debug.log("Illegal input parameters, minValue of " + minValue + " is illegal. " +
					"(minValue possible values are from 0 to " + bitMask + ".)",Debug.HIGH_PRIORITY);
			return false;
		}
		if(maxValue < 0 || maxValue > bitMask)
		{
			Debug.log("Illegal input parameters, maxValue of " + maxValue + " is illegal. " +
					"(maxValue possible values are from 0 to " + bitMask + ".)",Debug.HIGH_PRIORITY);
			return false;
		}
		if(minValue > maxValue)
		{
			Debug.log("Illegal input parameters, minValue > maxValue is illegal.",Debug.HIGH_PRIORITY);
			return false;
		}
		if(stepValue < 1 || stepValue > maxValue - minValue)
		{
			Debug.log("Illegal input parameters, stepValue of " + stepValue + " is illegal. " +
					"(stepValue possible values are from 1 to " + (maxValue - minValue) + ".)",Debug.HIGH_PRIORITY);
			return false;
		}
		if((((maxValue-minValue)/stepValue)|0) != (maxValue-minValue)/stepValue)
		{
			Debug.log("Illegal input parameters, maxValue of " + maxValue + 
					" must be an integer number of stepValue (stepValue=" + stepValue + 
					") steps away from minValue (minValue=" + minValue + ").",Debug.HIGH_PRIORITY);
			return false;
		}
		
		if(bitMapParams[6] != "" && 
				bitMapParams[6] != DEFAULT)
		{
			forcedAspectH = bitMapParams[6].split(':');
			if(forcedAspectH.length != 2)
			{
				Debug.log("Illegal input parameter, expecting ':' in string defining cell display aspect ratio " +
						"Height:Width (e.g. 100:150)." +  
						"\nInput aspect ratio string '" + bitMapParams[6] + "' is invalid.",Debug.HIGH_PRIORITY);			
				return false;
			}	
			forcedAspectW = forcedAspectH[1].trim()|0;			
			forcedAspectH = forcedAspectH[0].trim()|0;
		}
		else //default to 1:1
			forcedAspectW = forcedAspectH = 1;

		
		//colors
		minValueColor = bitMapParams[7] == DEFAULT || bitMapParams[7] == ""?"red":bitMapParams[7];
		midValueColor = bitMapParams[8] == DEFAULT || bitMapParams[8] == ""?"yellow":bitMapParams[8];
		maxValueColor = bitMapParams[9] == DEFAULT || bitMapParams[9] == ""?"green":bitMapParams[9];
		floorValueColor = bitMapParams[10] == DEFAULT || bitMapParams[10] == ""?minValueColor:bitMapParams[10];
		ceilValueColor = bitMapParams[11] == DEFAULT || bitMapParams[11] == ""?maxValueColor:bitMapParams[11];

		//convert to arrays
		minValueColor = DesktopContent.getColorAsRGBA(minValueColor).split("(")[1].split(")")[0].split(",");
		midValueColor = DesktopContent.getColorAsRGBA(midValueColor).split("(")[1].split(")")[0].split(",");
		maxValueColor = DesktopContent.getColorAsRGBA(maxValueColor).split("(")[1].split(")")[0].split(",");
		ceilValueColor = DesktopContent.getColorAsRGBA(ceilValueColor).split("(")[1].split(")")[0].split(",");
		floorValueColor = DesktopContent.getColorAsRGBA(floorValueColor).split("(")[1].split(")")[0].split(",");

		//load bools
		doDisplayRowsAscending = bitMapParams[12] == "Yes"?1:0;
		doDisplayColsAscending = bitMapParams[13] == "Yes"?1:0;
		doSnakeColumns = bitMapParams[14] == "Yes"?1:0;
		doSnakeRows = bitMapParams[15] == "Yes"?1:0;

		if(doSnakeColumns && doSnakeRows)
		{
			Debug.log("Can not have a bitmap that snakes both rows and columns, please choose one or the other (or neither).",Debug.HIGH_PRIORITY);
			return false;
		}
		
		
		return true;
	}	

	//:::::::::::::::::::::::::::::::::::::::::
	//localInitBitmapData ~~
	//	load bitmap data from input string <initBitMapValue>
	//		and initialize bmpDataImage
	//	treat <initBitMapValue> as JSON 2D array string
	function localInitBitmapData()
	{
		//create empty array for bmpData		
		bmpData = [];
		
		try
		{			
			var jsonMatrix = JSON.parse(initBitMapValue);
			
			//create place holder 2D array for fill
			for(var r=0;r<rows;++r)
			{
				bmpData.push([]); //create empty row array
				
				for(var c=0;c<cols;++c)
					bmpData[r][c] = 0;
			}
			localConvertFullRowColToGrid(jsonMatrix); //also sets bmpDataImage
		}
		catch(err)
		{
			Debug.log("The input initial value of the bitmap is illegal JSON format. " +					
					"See error below: \n\n" + err,Debug.HIGH_PRIORITY);
			Debug.log("Defaulting to initial bitmap with min-value fill.",Debug.HIGH_PRIORITY);
			
			//min-value fill			
			var color;		
			for(var r=0;r<rows;++r)		
			{
				bmpData.push([]); //create empty row array
			
				for(var c=0;c<cols;++c)
				{
					bmpData[r][c] = minValue; //min-value entry in column

					color = localConvertValueToRGBA(bmpData[r][c]);
					bmpDataImage.data[(r*cols + c)*4+0]=color[0];
					bmpDataImage.data[(r*cols + c)*4+1]=color[1];
					bmpDataImage.data[(r*cols + c)*4+2]=color[2];
					bmpDataImage.data[(r*cols + c)*4+3]=color[3];
				}
			}

			bmpContext.putImageData(bmpDataImage,0,0);
			bmp.src = bmpCanvas.toDataURL();			
		}		
	}

	//:::::::::::::::::::::::::::::::::::::::::
	//localConvertGridToRowCol ~~
	//	grid row col is always 0,0 in top left
	//	but there might be translation for user (imagine snaked columns)
	//		inputs: doDisplayRowsAscending, doDisplayColsAscending, doSnakeColumns, doSnakeRows,  
	//			...rows, cols
	//	return translated row,col 
	function localConvertGridToRowCol(r,c)
	{
		var retVal = [r,c];
		if(!doDisplayRowsAscending) //reverse row order so flip row
			retVal[0] = rows - 1 - retVal[0];
		if(!doDisplayColsAscending) //reverse col order so flip col
			retVal[1] = cols - 1 - retVal[1];	
		if(doSnakeRows && retVal[0]%2 == 1) //snake row so flip col
			retVal[1] = cols + (cols - 1 - retVal[1]);
		if(doSnakeColumns && retVal[1]%2 == 1) //snake col so flip row
			retVal[0] = rows + (rows - 1 - retVal[0]);		
			
		return retVal;
	}
	
	//:::::::::::::::::::::::::::::::::::::::::
	//localConvertValueToRGBA ~~
	//	conver bitfield value to RGBA based on input parameters
	function localConvertValueToRGBA(val)
	{
		if(val >= maxValue)			
			return [ceilValueColor[0],
					ceilValueColor[1],
					ceilValueColor[2],					  
					  255]; //always max alpha

		if(val <= minValue)			
			return [floorValueColor[0],
					floorValueColor[1],
					floorValueColor[2],					  
					  255]; //always max alpha

		if(val == midValue)	//avoid dividing by 0 in blend	
			return [midValueColor[0],
					midValueColor[1],
					midValueColor[2],					  
					  255]; //always max alpha

		//blend lower half
		var t;
		if(val <= midValue)		
		{
			t = (val - minValue)/(midValue - minValue); 
			return [minValueColor[0]*(1-t) + t*midValueColor[0],
					minValueColor[1]*(1-t) + t*midValueColor[1],
					minValueColor[2]*(1-t) + t*midValueColor[2],					  
					  255]; //always max alpha
		}
		//blend upper half
		//if(val >= midValue)		
		{
			t = (val - midValue)/(maxValue - midValue); 
			return [midValueColor[0]*(1-t) + t*maxValueColor[0],
					midValueColor[1]*(1-t) + t*maxValueColor[1],
					midValueColor[2]*(1-t) + t*maxValueColor[2],					  
					  255]; //always max alpha
		}
	}
	

	//:::::::::::::::::::::::::::::::::::::::::
	//localConvertFullGridToRowCol ~~
	//	convert bmpData matrix to a matrix with translated Row,Col pairs
	function localConvertFullGridToRowCol()
	{
		var retArr = [];
		var convertedRC;
		for(var r=0;r<rows;++r)
			for(var c=0;c<cols;++c)
			{
				convertedRC = localConvertGridToRowCol(r,c);
				//if doSnakeColumns, odd columns are considered to be in even column
				if(doSnakeColumns)
					convertedRC[1] = (convertedRC[1]/2)|0;
				//if doSnakeRows, odd rows are considered to be in even row
				if(doSnakeRows) 
					convertedRC[0] = (convertedRC[0]/2)|0;
				
				if(retArr[convertedRC[0]] === undefined)
					retArr[convertedRC[0]] = []; //create row for first time
				retArr[convertedRC[0]][convertedRC[1]] = bmpData[r][c];
			}
		return retArr;
	}

	//:::::::::::::::::::::::::::::::::::::::::
	//localConvertFullRowColToGrid ~~
	//	convert a matrix with translated Row,Col pairs to bmpData matrix
	//		updates bmpDataImage also and bmp display
	function localConvertFullRowColToGrid(srcMatrix)
	{
		var convertedRC;
		var color;
		var noErrors = true;
		for(var r=0;r<rows;++r)
			for(var c=0;c<cols;++c)
			{
				convertedRC = localConvertGridToRowCol(r,c);

				//if doSnakeColumns, odd columns are considered to be in even column
				if(doSnakeColumns)
					convertedRC[1] = (convertedRC[1]/2)|0;
				//if doSnakeRows, odd rows are considered to be in even row
				if(doSnakeRows) 
					convertedRC[0] = (convertedRC[0]/2)|0;
				try
				{
					bmpData[r][c] = srcMatrix[convertedRC[0]][convertedRC[1]]|0;
					if(bmpData[r][c] < minValue)
						throw("There was an illegal value less than minValue: " +
								bmpData[r][c] + " < " + minValue + " @ (row,col) = (" +
								convertedRC[0] + "," + convertedRC[0] + ")");
					if(bmpData[r][c] > maxValue)
						throw("There was an illegal value greater than maxValue: " +
								bmpData[r][c] + " > " + maxValue + " @ (row,col) = (" +
								convertedRC[0] + "," + convertedRC[0] + ")");
					if((((bmpData[r][c]-minValue)/stepValue)|0) != (bmpData[r][c]-minValue)/stepValue)					
						throw("There was an illegal value not following stepValue from minValue: " +
								bmpData[r][c] + " != " + 
								(stepValue*(((bmpData[r][c]-minValue)/stepValue)|0)) + 
								" @ (row,col) = (" +
								convertedRC[0] + "," + convertedRC[0] + ")");
					color = localConvertValueToRGBA(bmpData[r][c]);
					bmpDataImage.data[(r*cols + c)*4+0]=color[0];
					bmpDataImage.data[(r*cols + c)*4+1]=color[1];
					bmpDataImage.data[(r*cols + c)*4+2]=color[2];
					bmpDataImage.data[(r*cols + c)*4+3]=color[3];
				}
				catch(err)
				{noErrors = false;} //ignore errors
			}
		bmpContext.putImageData(bmpDataImage,0,0);
		bmp.src = bmpCanvas.toDataURL();
		
		if(!noErrors)
			throw("There was a mismatch in row/col dimensions. Input matrix was " + 
					"dimension [row,col] = [" + srcMatrix.length + "," +
					(srcMatrix.length?srcMatrix[0].length:0) + "]");		
	}
	
	//:::::::::::::::::::::::::::::::::::::::::
	//localCreateBitmap ~~
	//	create bitmap
	function localCreateBitmap()
	{
		bmp =  document.createElement("img");
		bmp.setAttribute("id", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap");
		
		bmpGrid = document.createElement("div"); //div of row and col grid divs
		bmpGrid.setAttribute("id", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-grid");
		
		bmpOverlay =  document.createElement("img");
		bmpOverlay.setAttribute("id", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-overlay");
		
		cursorInfo = document.createElement("div"); //div of row and col grid divs
		cursorInfo.setAttribute("id", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-cursor-info");
		hdrCursorInfo = document.createElement("div"); //div of row and col grid divs
		hdrCursorInfo.setAttribute("id", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-cursor-hdrInfo");
				
		//create divs for r,c text display
		rowLeftNums = document.createElement("div");
		rowRightNums = document.createElement("div");	
		colTopNums = document.createElement("div");
		colBottomNums = document.createElement("div");
		rowLeftNums.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-numbers-rowLeft");	
		rowRightNums.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-numbers-rowRight");
		colTopNums.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-numbers-colTop");	
		colBottomNums.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-numbers-colBottom");
		 
		var tmpEl;
		 
		//group creation of row/col elements
		{			
			bmpCanvas=document.createElement("canvas");
			bmpCanvas.width = cols;
			bmpCanvas.height = rows;
			bmpContext = bmpCanvas.getContext("2d");		

			if(bmpDataImage) delete bmpDataImage;
			bmpDataImage = bmpContext.createImageData(cols,rows);
			
			//add outside box div as child 0
			tmpEl = document.createElement("div");
			tmpEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-grid-box");
			bmpGrid.appendChild(tmpEl);
			
			for(var i=0;i<rows;++i)
			{
				if(i < rows - 1) //add internal row divs to start
				{
					tmpEl = document.createElement("div");
					tmpEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-grid-row-dark");
					bmpGrid.appendChild(tmpEl);
					tmpEl = document.createElement("div");
					tmpEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-grid-row");
					bmpGrid.appendChild(tmpEl);
				}
				
				for(var j=0;j<cols;++j)
				{
					if(i == rows-1 & j < cols-1) //add internal col divs at end
					{
						tmpEl = document.createElement("div");
						tmpEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-grid-col-dark");
						bmpGrid.appendChild(tmpEl);
						tmpEl = document.createElement("div");
						tmpEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-grid-col");
						bmpGrid.appendChild(tmpEl);
					}						
				}
			}
			
			bmpContext.putImageData(bmpDataImage,0,0);
			bmp.src = bmpCanvas.toDataURL();
		}	

		bmp.style.position = "absolute";		
		bmp.draggable = false; //prevent dragging
		
		bmpGrid.style.position = "absolute";	
		
		bmpOverlay.style.display = "none";
		bmpOverlay.style.position = "absolute";	
		bmpOverlay.draggable = false; //prevent dragging
				
		cursorInfo.style.position = "absolute";	
		cursorInfo.style.display = "none";
		hdrCursorInfo.style.position = "absolute";	
		hdrCursorInfo.style.display = "none";
		hdrCursorInfo.style.width = "320px";

		rowLeftNums.style.position = "absolute";	
		rowRightNums.style.position = "absolute";	
		colTopNums.style.position = "absolute";	
		colBottomNums.style.position = "absolute";	
		
		el.appendChild(bmp);
		el.appendChild(bmpGrid);
		el.appendChild(bmpOverlay);

		el.appendChild(hdrCursorInfo); //insert hdrInfo first so cursorInfo goes over top of it
		el.appendChild(cursorInfo);
		
		el.appendChild(rowLeftNums);
		el.appendChild(rowRightNums);
		el.appendChild(colTopNums);
		el.appendChild(colBottomNums);		
	}

	//:::::::::::::::::::::::::::::::::::::::::
	//localCreateGridButtons ~~
	//	create all (row,col) buttons
	function localCreateGridButtons()
	{
		allRowBtns = document.createElement("div"); //div of all row button divs
		
		allColBtns = document.createElement("div"); //div of all col button divs
				
		allBtn = document.createElement("div"); //div of all button			
		allBtn.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-btn-all");	
		
		var tmpEl;
		for(var i=0;i<rows;++i)
		{
			tmpEl = document.createElement("div");
			tmpEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-btn-all");
			tmpEl.style.position = "absolute";
			allRowBtns.appendChild(tmpEl);
		}
		for(var i=0;i<cols;++i)
		{
			tmpEl = document.createElement("div");
			tmpEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-btn-all");
			tmpEl.style.position = "absolute";
			allColBtns.appendChild(tmpEl);
		}
		
		allRowBtns.style.position = "absolute";
		el.appendChild(allRowBtns);
		allColBtns.style.position = "absolute";
		el.appendChild(allColBtns);
		allBtn.style.position = "absolute";
		el.appendChild(allBtn);
	}
	
	//:::::::::::::::::::::::::::::::::::::::::
	//localCreateHeader ~~
	//	create header
	function localCreateHeader()
	{
		hdr = document.createElement("div");
		hdr.setAttribute("id", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-header");

		var str = "";

		str += "<div style='float:left; margin: 0 0 20px 0;'>"; //field name and info container
		str += "<div style='float:left; '>";
		str += fieldName;
//		fieldName = fieldName.split('\n');
//		str += "Target UID/Field: &quot;" + 
//				fieldName[0].split(" : ")[1] + "/" +
//				fieldName[1].split(" : ")[0] + "&quot;";
		str += "</div>";

		str += "<div style='float:left; margin-left: 50px;'>";
		str += "Number of [Rows,Cols]: " + "[" + rows + "," + cols + "]";
		str += "</div>";
		str += "</div>";//end field name and info container

		str += "<div style='float:right; '>";
		str += "<a id='" + 
				ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-cancel' href='#'>Cancel</a>";
		str += "</div>";

		str += "<div id='clearDiv'></div>";
		
		str += "<div style='float:right; margin: 40px 20px -50px 0;'>";
		str += "<a id='" + 
				ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-ok' href='#'>OK</a>";
		str += "</div>";

		str += "<div style='float:left; margin: 0 0 0 0;'>";
		for(var clickIndex=0;clickIndex<2;++clickIndex)
		{
			str += "<div style='float:left; margin: 5px 0 0 0;'>";
			str += "<div style='float:left; width:180px; text-align:right; margin-top: 3px;'>";
			str += (clickIndex?"Right":"Left") + "-Click Value:";
			str += "</div>";
			str += "<input class='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
					"-bitmap-scrollbar' style='float:left;' " + 
					"type='range' min='" + minValue + 
					"' max='" + maxValue + "' value='" + (clickIndex?maxValue:minValue) + 
					"' step='" + stepValue + 
					"' oninput='ConfigurationAPI.bitMapDialog.localUpdateScroll(" + clickIndex + ")' />";
			str += "<input class='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
					"-bitmap-btnInput' style='float:left; margin: 0 1px 0 5px;' " +
					"type='button' value='<' " +
					"onmousedown='ConfigurationAPI.bitMapDialog.localUpdateButtonInput(" + clickIndex + ",0,0)' " +
					"onmouseup='ConfigurationAPI.bitMapDialog.localUpdateButtonInput(" + clickIndex + ",0,1)' " +
					"/> ";
			str += "<input class='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
					"-bitmap-btnInput' style='float:left;' " +			
					"type='button' value='>' " +
					"onmousedown='ConfigurationAPI.bitMapDialog.localUpdateButtonInput(" + clickIndex + ",1,0)' " +
					"onmouseup='ConfigurationAPI.bitMapDialog.localUpdateButtonInput(" + clickIndex + ",1,1)' " +
					"/> ";
			str += "<input class='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
					"-bitmap-textInput' style='float:left; margin: 0 5px 0 5px; width: 50px;' " +
					"type='text' " + //value come from scroll update at start
					"onchange='ConfigurationAPI.bitMapDialog.localUpdateTextInput(" + clickIndex + ",1)' " +	
					"onkeydown='ConfigurationAPI.bitMapDialog.localUpdateTextInput(" + clickIndex + ",0)' " +
					"onkeyup='ConfigurationAPI.bitMapDialog.localUpdateTextInput(" + clickIndex + ",0)' " +				
					"/>";
			str += "<img class='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
					"-bitmap-colorSample' style='float:left;width:25px; height:25px; margin: -2px 0 2px 0;' " +
					"ondragstart='return false;' " + //ondragstart for firefox
					"draggable='false'" +	//draggable for chrome				
					"'/>";
	
			
			str += "</div>";
	
			str += "<div id='clearDiv'></div>";
		}
		str += "</div>";
		
		//add download upload buttons
		str += "<div style='float:left; margin: 5px 0 0 40px;'>";
		str += "<input class='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-bitmap-btnCsv' style='float:left;' " +			
				"type='button' value='Download as CSV' " +
				"onclick='ConfigurationAPI.bitMapDialog.localDownloadCSV()' " +
				"/> ";
		str += "<input class='" + ConfigurationAPI._POP_UP_DIALOG_ID + 
				"-bitmap-btnCsv' style='float:left; margin: 0 0 0 10px;' " +			
				"type='button' value='Upload CSV' " +
				"onclick='ConfigurationAPI.bitMapDialog.locaPopupUploadCSV()' " +
				"/> ";
		str += "</div>";

		hdr.innerHTML = str;
		hdr.style.overflowY = "auto";
		hdr.style.position = "absolute";

		var scrollEls = hdr.getElementsByClassName(ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-scrollbar");	
		var textInputEls = hdr.getElementsByClassName(ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-textInput");	
		var colorSampleEls = hdr.getElementsByClassName(ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-colorSample");
			

		//::::::::::
		//localUpdateScroll ~~
		ConfigurationAPI.bitMapDialog.localUpdateScroll = function(i)		
		{
			Debug.log("localUpdateScroll " + i);
			
			clickValues[i] = scrollEls[i].value|0;
			clickColors[i] = localConvertValueToRGBA(clickValues[i]);
			
			textInputEls[i].value = clickValues[i];
			colorSampleEls[i].src = ConfigurationAPI.getOnePixelPngData(clickColors[i]);
		};  //end localUpdateScroll

		//::::::::::
		//localUpdateTextInput ~~
		ConfigurationAPI.bitMapDialog.localUpdateTextInput = function(i,finalChange) 
		{
			Debug.log("localUpdateTextInput " + textInputEls[i].value + " " + finalChange);

			clickValues[i] = textInputEls[i].value|0;

			if(finalChange)
			{
				if(clickValues[i] < minValue) clickValues[i] = minValue;
				if(clickValues[i] > maxValue) clickValues[i] = maxValue;
				clickValues[i] = (((clickValues[i]-minValue)/stepValue)|0)*stepValue + minValue; //lock to step
				textInputEls[i].value = clickValues[i]; //fix value
			}
			else //try to continue with change, but if invalid just return
			{
				if(clickValues[i] < minValue) return;
				if(clickValues[i] > maxValue) return;
				if((((clickValues[i]-minValue)/stepValue)|0) != (clickValues[i]-minValue)/stepValue)
					return; //no locked to step value
				Debug.log("displaying change");
			}
			clickColors[i] = localConvertValueToRGBA(clickValues[i]);

			scrollEls[i].value = clickValues[i];
			colorSampleEls[i].src = ConfigurationAPI.getOnePixelPngData(clickColors[i]);
		};  //end localUpdateTextInput

		//::::::::::
		//localUpdateButtonInput ~~
		var mouseDownTimer = 0;
		ConfigurationAPI.bitMapDialog.localUpdateButtonInput = function(i,dir,mouseUp,delay) 
		{
			window.clearInterval(mouseDownTimer);
			if(mouseUp) //mouse up	
			{				
				Debug.log("cancel mouse down"); 
				return;
			}
			//else mouse is down so set repeat interval
			mouseDownTimer = window.setInterval(function()
					{
				//faster and faster
				if(delay > 50) delay -= 50;
				ConfigurationAPI.bitMapDialog.localUpdateButtonInput(i,dir,0,50); //same as this call
					},delay!==undefined?delay:300);

			Debug.log("localUpdateButtonInput " + textInputEls[i].value + " " + dir);

			clickValues[i] = clickValues[i] + (dir?stepValue:-stepValue);
			if(clickValues[i] < minValue) clickValues[i] = minValue;
			if(clickValues[i] > maxValue) clickValues[i] = maxValue;

			clickColors[i] = localConvertValueToRGBA(clickValues[i]);

			textInputEls[i].value = clickValues[i];
			scrollEls[i].value = clickValues[i];
			colorSampleEls[i].src = ConfigurationAPI.getOnePixelPngData(clickColors[i]);

		};  //end localUpdateButtonInput

		//::::::::::
		//localDownloadCSV ~~
		// online people complain that this doesn't work for big files.
		// Note: if files are too big, could have server create csv file
		//	then link to <a href='/WebPath/download.csv' download></a>
		//	Note: href must be  encodeURI(dataStr) if data not already encoded
		ConfigurationAPI.bitMapDialog.localDownloadCSV = function()
		{
			var transGrid = localConvertFullGridToRowCol();
			console.log(transGrid);

			var dataStr = "data:text/csv;charset=utf-8,";
			
			for(var r=0;r<transGrid.length;++r)
			{
				if(r) dataStr += encodeURI("\n"); //encoded \n
				for(var c=0;c<transGrid[0].length;++c)
				{
					if(c) dataStr += ",";
					dataStr += transGrid[r][c];
				}
			}
			
			Debug.log("ConfigurationAPI.bitMapDialog.localDownloadCSV dataStr=" + dataStr);
						
			var link = document.createElement("a");
			link.setAttribute("href", dataStr); //double encode, so encoding remains in CSV
			link.setAttribute("style", "display:none");
			link.setAttribute("download", _currentConfigName + "_" + 
					fieldName + "_download.csv");
			document.body.appendChild(link); // Required for FF

			link.click(); // This will download the data file named "my_data.csv"

			link.parentNode.removeChild(link);
		}; //end localDownloadCSV
		
		
		
		//::::::::::
		//locaUploadCSV ~~
		ConfigurationAPI.bitMapDialog._csvUploadDataStr; //uploaded csv table ends up here
		ConfigurationAPI.bitMapDialog.locaUploadCSV = function()
		{
			Debug.log("locaUploadCSV ConfigurationAPI.bitMapDialog._csvUploadDataStr = " + ConfigurationAPI.bitMapDialog._csvUploadDataStr);
			var srcDataStr = ConfigurationAPI.bitMapDialog._csvUploadDataStr.split('\n');
			var src = []; //src = [r][c]
			for(var i=0;i<srcDataStr.length;++i)
				src.push(srcDataStr[i].split(','));
			console.log(src);
			
			try
			{
				localConvertFullRowColToGrid(src);
			
				Debug.log("Successfully uploaded CSV file to bitmap!", Debug.INFO_PRIORITY);

				//on succes remove popup
				el = document.getElementById("popUpDialog"); 
				if(el) el.parentNode.removeChild(el);
			}
			catch(err)			
			{
				Debug.log("Errors occured during upload. Bitmap may not reflect contents of CSV file." + 
						"\nHere is the error description: \n" + err, Debug.HIGH_PRIORITY);
				
				//enable button so upload can be tried again
				document.getElementById('popUpDialog-submitButton').disabled = false;
			}
		}

		//::::::::::
		//locaPopupUploadCSV ~~
		ConfigurationAPI.bitMapDialog.locaPopupUploadCSV = function()
		{			
			Debug.log("ConfigurationAPI.bitMapDialog.locaPopupUploadCSV");	
			ConfigurationAPI.bitMapDialog._csvUploadDataStr = ""; //clear previous upload

			var str = "";

			var pel = document.getElementById("popUpDialog");
			if(!pel)
			{
				pel = document.createElement("div");			
				pel.setAttribute("id", "popUpDialog");
			}
			pel.style.display = "none";

			//set position and size
			var w = 380;
			var h = 195;
			ConfigurationAPI.setPopUpPosition(pel,w /*w*/,h /*h*/);
			
			var str = "<a id='" + 
					"popUpDialog" + //clear upload string on cancel!
					"-header' href='#' onclick='javascript:ConfigurationAPI.bitMapDialog._csvUploadDataStr = \"\"; var pel = document.getElementById(" +
					"\"popUpDialog\"); if(pel) pel.parentNode.removeChild(pel); return false;'>Cancel</a><br><br>";

			str += "<div id='popUpDialog-div'>";	

			str += "Please choose a CSV formatted data file (i.e. commas for columns, and new lines for rows) " +
					"to upload:<br><br>";

			str += "<center>";

			str += "<input type='file' id='popUpDialog-fileUpload' " + 
					"accept='.csv' enctype='multipart/form-data' />";			
		
			// done with special handling
			// continue with pop-up prompt
			str += "</center></div><br><br>"; //close main popup div

			var onmouseupJS = "";
			onmouseupJS += "document.getElementById(\"popUpDialog-submitButton\").disabled = true;";
			onmouseupJS += "ConfigurationAPI.bitMapDialog.locaUploadCSV();";			

			str += "<input id='popUpDialog-submitButton' disabled type='button' onmouseup='" + 
					onmouseupJS + "' " +
					"value='Upload File' title='" +
					"Upload the chosen file to replace the row,col data in the current bitmap." +					
					"'/>";

			pel.innerHTML = str;
			el.appendChild(pel); //add element to bitmap div
			pel.style.display = "block";

			document.getElementById('popUpDialog-fileUpload').addEventListener(
					'change', function(evt) {
				var files = evt.target.files;
				var file = files[0];           
				var reader = new FileReader();
				reader.onload = function() {
					//store uploaded file and enable button
					ConfigurationAPI.bitMapDialog._csvUploadDataStr = this.result;
					Debug.log("ConfigurationAPI.bitMapDialog._csvUploadDataStr = " + ConfigurationAPI.bitMapDialog._csvUploadDataStr);							
					document.getElementById('popUpDialog-submitButton').disabled = false;
				}
				reader.readAsText(file);
			}, false);
		
		}; //end locaUploadCSV
				

		el.appendChild(hdr);

		ConfigurationAPI.bitMapDialog.localUpdateScroll(0);
		ConfigurationAPI.bitMapDialog.localUpdateScroll(1);

	} //end localCreateHeader()

	//:::::::::::::::::::::::::::::::::::::::::
	//localPaint ~~
	//	called every time window is resized
	function localPaint()
	{
		Debug.log("localPaint");
		
		popSz = ConfigurationAPI.setPopUpPosition(el,undefined,undefined,padding,undefined,
				30 /*margin*/, true /*doNotResize*/);
		
		hdrW = popSz.w;
		//axisPadding = 40 + axisPaddingExtra; 
		hdrX = padding; 
		hdrY = padding;
		hdrW = popSz.w;
		hdrH = 150;
		bmpX = padding; 
		bmpY = hdrY+hdrH+padding;	
		bmpW = popSz.w - 2*axisPadding;
		bmpH = popSz.h - hdrH - padding - 2*axisPadding;

		cellW = bmpW/cols;
		cellH = bmpH/rows;
		
		localOptimizeAspectRatio();	//sets up bmpX,Y based on aspect ratio (inputs are cellW/cellH, bmpW/bmpH, rows/cols)
		
		//place header
		hdr.style.left = hdrX + "px";
		hdr.style.top = hdrY + "px";
		hdr.style.width = hdrW + "px";
		hdr.style.height = hdrH + "px";
		
		//place bitmap
		bmp.style.left = bmpX + "px";
		bmp.style.top = bmpY + "px";
		bmp.style.width = bmpW + "px";
		bmp.style.height = bmpH + "px";
		

		//place bitmap grid and buttons
		{
			
			bmpGrid.style.left = (bmpX-bmpBorderSize) + "px";
			bmpGrid.style.top = (bmpY-bmpBorderSize) + "px";
			bmpGrid.style.width = (bmpW) + "px";
			bmpGrid.style.height = (bmpH) + "px";			
			
			var bmpGridChildren = bmpGrid.childNodes;

			//place all rows div 
			allRowBtns.style.left = (bmpX - bmpBorderSize - axisPaddingMargin - bmpBorderSize - butttonSz) + "px";
			allRowBtns.style.top = (bmpY - bmpBorderSize) + "px";
			//place all cols div
			allColBtns.style.left = (bmpX - bmpBorderSize) + "px";
			allColBtns.style.top = (bmpY + bmpH + axisPaddingMargin - bmpBorderSize*2) + "px";
			//place all div
			allBtn.style.left = (bmpX - bmpBorderSize - axisPaddingMargin - bmpBorderSize - butttonSz) + "px";
			allBtn.style.top = (bmpY + bmpH + axisPaddingMargin - bmpBorderSize*2) + "px";
			allBtn.style.width = butttonSz + "px";
			allBtn.style.height = butttonSz + "px";
			
			var allRowsChildren = allRowBtns.childNodes;
			var allColsChildren = allColBtns.childNodes;

			
			//place number divs
			rowLeftNums.style.left = (bmpX - bmpBorderSize - axisPaddingMargin - bmpBorderSize - butttonSz + (- bmpBorderSize - axisPaddingMargin - axisPaddingExtra)) + "px";
			rowLeftNums.style.top = (bmpY - bmpBorderSize) + "px";
			rowRightNums.style.left = (bmpX + bmpW + axisPaddingMargin + bmpBorderSize) + "px";
			rowRightNums.style.top = (bmpY - bmpBorderSize) + "px";
			colTopNums.style.left = (bmpX - bmpBorderSize) + "px";
			colTopNums.style.top = (bmpY - bmpBorderSize*2 - numberDigitH) + "px";
			colBottomNums.style.left = (bmpX - bmpBorderSize) + "px";
			colBottomNums.style.top = (bmpY + bmpH + bmpBorderSize + axisPaddingMargin + bmpBorderSize + butttonSz + bmpBorderSize) + "px";
			rowLeftNums.innerHTML = ""; //clear all children
			rowRightNums.innerHTML = ""; //clear all children
			colTopNums.innerHTML = ""; //clear all children
			colBottomNums.innerHTML = ""; //clear all children
			
			var thresholdNumberSpacing = 100; //once threshold is reached another number is shown
			var numberLoc = []; //used to keep track of spacing
			var oldNumberLoc = [-thresholdNumberSpacing,-thresholdNumberSpacing]; //used to keep track of spacing
			var numberEl; 
			var translatedRC;
			
			//outer box
			bmpGridChildren[0].style.left = 0 + "px";
			bmpGridChildren[0].style.top = 0 + "px";
			bmpGridChildren[0].style.width = (bmpW) + "px";
			bmpGridChildren[0].style.height = (bmpH) + "px";
			
			//place rows
			for(var i=0;i<rows;++i)
			{
				if(i<rows-1)
				{
					//dark
					bmpGridChildren[1+i*2].style.left = bmpBorderSize + "px";
					bmpGridChildren[1+i*2].style.top = ((i+1)*cellH) + "px";//((i+1)*cellH + bmpBorderSize) + "px";
					bmpGridChildren[1+i*2].style.width = (bmpW) + "px";
					bmpGridChildren[1+i*2].style.height = (bmpGridThickness+bmpBorderSize*2) + "px";
	
					//light
					bmpGridChildren[1+i*2+1].style.left = 0 + "px";
					bmpGridChildren[1+i*2+1].style.top = ((i+1)*cellH + bmpBorderSize) + "px";//((i+1)*cellH + bmpBorderSize*2) + "px";
					bmpGridChildren[1+i*2+1].style.width = (bmpW + bmpBorderSize*2) + "px";
					bmpGridChildren[1+i*2+1].style.height = bmpGridThickness + "px";//((doSnakeRows && i%2 == 1)?0:bmpGridThickness) + "px";

					bmpGridChildren[1+i*2+1].style.backgroundColor = //change color if snaking
							(doSnakeRows && i%2 == 1)?"rgb(100,100,100)":"#efeaea";
				}
				
				//row button
				allRowsChildren[i].style.left = 0 + "px";
				allRowsChildren[i].style.top = (i*cellH + (i?bmpGridThickness+bmpBorderSize*2-1:0)) + "px";
				allRowsChildren[i].style.width = (butttonSz) + "px";
				allRowsChildren[i].style.height = (cellH - 1 + (i?-bmpBorderSize*2:0)) + "px";
				
				//numbers
				{
					numberLoc[0] = (i*cellH - 1 + cellH/2 - numberDigitH/2 + (i?bmpGridThickness+bmpBorderSize*2:0));
										
					//rowLeft numbers
					translatedRC = localConvertGridToRowCol(i,0);
					if(numberLoc[0] - oldNumberLoc[0] >= thresholdNumberSpacing && 
							translatedRC[0]%5 == 0)
					{
						//add a number
						numberEl = document.createElement("div");
						numberEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-number");
						numberEl.innerHTML = translatedRC[0];
						numberEl.style.top = numberLoc[0] + "px";
						numberEl.style.width = axisPaddingExtra + "px";						
						rowLeftNums.appendChild(numberEl);
						oldNumberLoc[0] = numberLoc[0];
					}

					//rowRight numbers
					translatedRC = localConvertGridToRowCol(i,cols>1?1:0);
					if(numberLoc[0] - oldNumberLoc[1] >= thresholdNumberSpacing && 
							translatedRC[0]%5 == 0)
					{
						//add a number
						numberEl = document.createElement("div");
						numberEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-number");
						numberEl.innerHTML = translatedRC[0];
						numberEl.style.top = numberLoc[0] + "px";
						numberEl.style.width = axisPaddingExtra + "px";						
						rowRightNums.appendChild(numberEl);
						oldNumberLoc[1] = numberLoc[0];
					}
				}
			}
			
			oldNumberLoc = [-thresholdNumberSpacing,-thresholdNumberSpacing]; //reset, used to keep track of spacing
			//place cols
			for(var i=0;i<cols;++i)
			{
				if(i<cols-1)
				{
					//if snaking cols, then darken every other
					//	by making light width = 0
					
					//dark
					bmpGridChildren[1+(rows-1)*2+i*2].style.top = bmpBorderSize + "px";
					bmpGridChildren[1+(rows-1)*2+i*2].style.left = ((i+1)*cellW + bmpBorderSize) + "px";
					bmpGridChildren[1+(rows-1)*2+i*2].style.height = (bmpH) + "px";
					bmpGridChildren[1+(rows-1)*2+i*2].style.width = (bmpGridThickness+bmpBorderSize*2) + "px";
										
					//light
					bmpGridChildren[1+(rows-1)*2+i*2+1].style.top = 0 + "px";
					bmpGridChildren[1+(rows-1)*2+i*2+1].style.left = ((i+1)*cellW + bmpBorderSize*2) + "px";
					bmpGridChildren[1+(rows-1)*2+i*2+1].style.height = (bmpH + bmpBorderSize*2) + "px";
					bmpGridChildren[1+(rows-1)*2+i*2+1].style.width = bmpGridThickness + "px"; //((doSnakeColumns && i%2 == 1)?0:bmpGridThickness) + "px";
					
					bmpGridChildren[1+(rows-1)*2+i*2+1].style.backgroundColor = //change color if snaking
							(doSnakeColumns && i%2 == 1)?"rgb(100,100,100)":"#efeaea";
				}
				
				//row button
				allColsChildren[i].style.left = (i*cellW - 1 + (i?bmpGridThickness+bmpBorderSize*2:0)) + "px";
				allColsChildren[i].style.top = 0 + "px";
				allColsChildren[i].style.width = (cellW + 1 - (i?bmpGridThickness+bmpBorderSize*2:0)) + "px";
				allColsChildren[i].style.height = (butttonSz) + "px";
				
				//numbers
				{
					numberLoc[0] =  (i*cellW + cellW/2 - axisPaddingExtra/2 + (i?bmpGridThickness+bmpBorderSize*2:0));

					//colTop numbers
					translatedRC = localConvertGridToRowCol(0,i);
					if(numberLoc[0] - oldNumberLoc[0] >= thresholdNumberSpacing && 
							translatedRC[1]%5 == 0)
					{
						//add a number
						numberEl = document.createElement("div");
						numberEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-number");
						numberEl.innerHTML = translatedRC[1];
						numberEl.style.left = numberLoc[0] + "px";
						numberEl.style.width = axisPaddingExtra + "px";						
						colTopNums.appendChild(numberEl);
						oldNumberLoc[0] = numberLoc[0];
					}

					//colBottom numbers
					translatedRC = localConvertGridToRowCol(rows>1?1:0,i);
					if(numberLoc[0] - oldNumberLoc[1] >= thresholdNumberSpacing && 
							translatedRC[1]%5 == 0)
					{
						//add a number
						numberEl = document.createElement("div");
						numberEl.setAttribute("class", ConfigurationAPI._POP_UP_DIALOG_ID + "-bitmap-number");
						numberEl.innerHTML = translatedRC[1];
						numberEl.style.left = numberLoc[0] + "px";
						numberEl.style.width = axisPaddingExtra + "px";						
						colBottomNums.appendChild(numberEl);
						oldNumberLoc[1] = numberLoc[0];
					}
				}
			}
		}
		
		
		
		
		
	
	} //end localPaint()
	
	//:::::::::::::::::::::::::::::::::::::::::
	//localOptimizeAspectRatio ~~
	//	inputs are cellW/cellH, bmpW/bmpH, rows/cols
	//	
	//	optimize aspect ratio for viewing window
	//	hdr and bmp positions are known after this
	function localOptimizeAspectRatio()
	{
		var cellSkew = (cellW>cellH)?cellW/cellH:cellH/cellW;
		var MAX_SKEW = 3;


		if(forcedAspectH !== undefined)
		{
			var offAspectH = forcedAspectH/cellH;
			var offAspectW = forcedAspectW/cellW;

			Debug.log("Adjusting skew factor = " + forcedAspectH + "-" + forcedAspectW);

			if(offAspectH < offAspectW) //height is too big
				bmpH = bmpW/cols*forcedAspectH/forcedAspectW*rows;
			else //width is too big
				bmpW = bmpH/rows*forcedAspectW/forcedAspectH*cols;
		}
		else if(cellSkew > MAX_SKEW) //re-adjust bitmap
		{
			var adj = cellSkew/MAX_SKEW;
			//to much skew in cell shape.. so let's adjust
			Debug.log("Adjusting skew factor = " + adj);
			if(cellW > cellH)
			{
				bmpW /= adj;
			}
			else
				bmpH /= adj;
		}
		//recalculate new cells
		cellW = bmpW/cols;
		cellH = bmpH/rows;

		//center bitmap
		bmpX = padding + (popSz.w-bmpW)/2; 
		bmpY = bmpY + (popSz.h-bmpY-bmpH)/2;	
		hdrY = bmpY - padding - hdrH;
	} //end localOptimizeAspectRatio()
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

//=====================================================================================
//setCaretPosition ~~
ConfigurationAPI.setCaretPosition = function(elem, caretPos, endPos) 
{
	elem.focus();
	elem.setSelectionRange(caretPos, endPos);
}


//=====================================================================================
//setPopUpPosition ~~
//	centers element based on width and height constraint
//	
//	Note: assumes a padding and border size if not specified
//  Note: if w,h not specified then fills screen (minus margin)
ConfigurationAPI.setPopUpPosition = function(el,w,h,padding,border,margin,doNotResize)
{
	if(padding === undefined) padding = 10;
	if(border === undefined) border = 1;	
	if(margin === undefined) margin = 0;	

	var x,y;

	//:::::::::::::::::::::::::::::::::::::::::
	//popupResize ~~
	//	set position and size	
	ConfigurationAPI.setPopUpPosition.popupResize = function() {
		
		try //check if element still exists
		{
			if(!el) //if element no longer exists.. then remove listener and exit
			{
				window.removeEventListener("resize",ConfigurationAPI.setPopUpPosition.popupResize);
				window.removeEventListener("scroll",ConfigurationAPI.setPopUpPosition.popupResize);
				return;
			}
		}
		catch(err) {return;} //do nothing on errors
		
		//else resize el
		Debug.log("ConfigurationAPI.setPopUpPosition.popupResize");


		var ww = DesktopContent.getWindowWidth()-(padding+border)*2;
		var wh = DesktopContent.getWindowHeight()-(padding+border)*2;

		//ww & wh are max window size at this point

		if(w === undefined || h === undefined)
		{
			w = ww-(margin)*2;
			h = wh-(margin)*2;
		}
		//else w,h are inputs and margin is ignored

		x = (DesktopContent.getWindowScrollLeft() + ((ww-w)/2));
		y = (DesktopContent.getWindowScrollTop() + ((wh-h)/2));

		el.style.left = x + "px";
		el.style.top = y + "px"; 
	}; 
	ConfigurationAPI.setPopUpPosition.popupResize();
	
	//window width and height are not manipulated on resize, only setup once
	el.style.width = w + "px";
	el.style.height = h + "px";
	
	if(!doNotResize)
	{
		window.addEventListener("resize",ConfigurationAPI.setPopUpPosition.popupResize);
		window.addEventListener("scroll",ConfigurationAPI.setPopUpPosition.popupResize);
	}
	
	return {"w" : w, "h" : h, "x" : x, "y" : y};
}


//=====================================================================================
//setPopUpPosition ~~
//	centers element based on width and height constraint
//	
//	Note: assumes a padding and border size if not specified
//  Note: if w,h not specified then fills screen (minus margin)
//
//	alpha is optional, will assume full 255 alpha
ConfigurationAPI.getOnePixelPngData = function(rgba)
{
	if(ConfigurationAPI.getOnePixelPngData.canvas ===  undefined)
	{
		//create only first time this functino is called
		ConfigurationAPI.getOnePixelPngData.canvas = document.createElement("canvas");
		ConfigurationAPI.getOnePixelPngData.canvas.width = 1;
		ConfigurationAPI.getOnePixelPngData.canvas.height = 1;
		ConfigurationAPI.getOnePixelPngData.ctx = ConfigurationAPI.getOnePixelPngData.canvas.getContext("2d");
		ConfigurationAPI.getOnePixelPngData.bmpOverlayData = ConfigurationAPI.getOnePixelPngData.ctx.createImageData(1,1);
	}	
	
	ConfigurationAPI.getOnePixelPngData.bmpOverlayData.data[0]=rgba[0];
	ConfigurationAPI.getOnePixelPngData.bmpOverlayData.data[1]=rgba[1];
	ConfigurationAPI.getOnePixelPngData.bmpOverlayData.data[2]=rgba[2];
	ConfigurationAPI.getOnePixelPngData.bmpOverlayData.data[3]=rgba[3]!==undefined?rgba[3]:255;
	
	ConfigurationAPI.getOnePixelPngData.ctx.putImageData(
			ConfigurationAPI.getOnePixelPngData.bmpOverlayData,0,0);
	return ConfigurationAPI.getOnePixelPngData.canvas.toDataURL();
}




//=====================================================================================
//createEditableFieldElement ~~
//
//	Creates div element with editable and highlight features
//
//	Input field must be a value node.
//	Input object is single field as returned from ConfigurationAPI.getFieldsOfRecords
//
//	fieldIndex is unique integer for the field 
//	depthIndex (optional) is indicator of depth, e.g. for tree display
//	
//	Field := {}
//			obj.fieldTableName 
//			obj.fieldUID 
//			obj.fieldColumnName
//			obj.fieldRelativePath 
//			obj.fieldColumnType
//			obj.fieldColumnDataType
//			obj.fieldColumnDataChoicesArr[]
//
ConfigurationAPI.editableFieldEditingCell_ = 0;
ConfigurationAPI.editableFieldEditingIdString_;
ConfigurationAPI.editableFieldEditingNodeType_;
ConfigurationAPI.editableFieldEditingOldValue_;
ConfigurationAPI.editableFieldEditingInitValue_;
ConfigurationAPI.editableFieldHoveringCell_ = 0;
ConfigurationAPI.editableFieldSelectedCell_ = 0;
ConfigurationAPI.editableFieldSelectedIdString_;
ConfigurationAPI.editableFieldHandlersSubscribed_ = false;
ConfigurationAPI.editableFieldMouseIsSelecting_ = false;
ConfigurationAPI.editableField_SELECTED_COLOR_ = "rgb(251, 245, 53)";
ConfigurationAPI.createEditableFieldElement = function(fieldObj,fieldIndex,depthIndex /*optional*/)
{
	var str = "";
	var depth = depthIndex|0;
	var uid = fieldIndex|0;
	
	if(!ConfigurationAPI.editableFieldHandlersSubscribed_)
	{
		ConfigurationAPI.editableFieldHandlersSubscribed_ = true;		
		
		//be careful to not override the window.onmousemove DesktopContent action
		DesktopContent.mouseMoveSubscriber(ConfigurationAPI.handleEditableFieldBodyMouseMove); 
	}
	
	var fieldEl = document.createElement("div");
	fieldEl.setAttribute("class", "ConfigurationAPI-EditableField");
	fieldEl.setAttribute("id", "ConfigurationAPI-EditableField-" + 
			( depth + "-" + uid ));
	
	Debug.log("Field type " + fieldObj.fieldColumnType);
	console.log(fieldObj);
	
	var valueType = fieldObj.fieldColumnType;
	var choices = fieldObj.fieldColumnDataChoicesArr;
	var value = "";
	var path = fieldObj.fieldRelativePath;
	var nodeName = fieldObj.fieldColumnName;

	var pathHTML = path;
	//make path html safe
	pathHTML = pathHTML.replace(/</g, "&lt");
	pathHTML = pathHTML.replace(/>/g, "&gt");	
	
	str += "<div class='treeNode-Path' style='display:none' id='treeNode-path-" +
			( depth + "-" + uid ) + "'>" + // end path id
			pathHTML + //save path for future use.. and a central place to edit when changes occur
			"</div>";
								
	
	if(valueType == "FixedChoiceData")
	{
		//add CSV choices div
		str += 
				"<div class='treeNode-FixedChoice-CSV' style='display:none' " + 
				"id='treeNode-FixedChoice-CSV-" +
				( depth + "-" + uid ) + "'>";

		for(var j=0;j<choices.length;++j)
		{
			if(j) str += ",";
			str += choices[j];
		}
		str += "</div>";
	}
	else if(valueType == "BitMap")
	{
		//add bitmap params div
		str += 
				"<div class='treeNode-BitMap-Params' style='display:none' " + 
				"id='treeNode-BitMap-Params-" +
				( depth + "-" + uid ) + "'>";

		for(var j=1;j<choices.length;++j) //skip the first DEFAULT param
		{
			if(j-1) str += ";"; //assume no ';' in fields, so likely no issue to replace ; with ,
			str += choices[j].replace(/;/g,","); //change all ; to , for split safety
		}
		str += "</div>";
	}

	//normal value node and edit icon
	{
		//start value node
		str += 
				"<div class='treeNode-Value treeNode-ValueType-" + valueType +
				"' " +
				"id='treeNode-Value-" +
				(depth + "-" + uid) + "' " +

				"onclick='ConfigurationAPI.handleEditableFieldClick(" +							
				depth + "," + uid + "," + 	
				"0,\"value\")' " +

				"onmousemove='ConfigurationAPI.handleEditableFieldHover(" +							
				depth + "," + uid + "," + 	
				"event)' " +

				">";	

		titleStr = "~ Leaf Value Node ~\n";
		titleStr +=	"Path: \t" + path + nodeName + "\n";

		//left side of value
		str += 
				"<div style='float:left' title='" + titleStr + "'>" +
				"<b class='treeNode-Value-leafNode-fieldName bold-header'>" + 
				nodeName + "</b>" + 
				"</div><div style='float:left'>&nbsp;:</div>";

		//normal edit icon
		str += 
				"<div class='treeNode-Value-editIcon' id='treeNode-Value-editIcon-" +
				(depth + "-" + uid) + "' " +
				"onclick='ConfigurationAPI.handleEditableFieldClick(" +							 
				depth + "," + uid + "," + 	
				"1,\"value\"); event.stopPropagation();' " +
				"title='Edit the value of this node.' " +
				"></div>";						
	}

	str += "<div style='float:left; margin-left:9px;' id='treeNode-Value-leafNode-" +
			(depth + "-" + uid) +			
			"' class='" +
			"treeNode-Value-leafNode-ColumnName-" + nodeName +
			"' " +
			">";

	if(valueType == "OnOff" || 
			valueType == "YesNo" || 
			valueType == "TrueFalse")
	{
		//colorize true false														
		str += "<div style='float:left'>";										
		str += value;											
		str += "</div>";

		var color = (value == "On" || value == "Yes" || value == "True")?
				"rgb(16, 204, 16)":"rgb(255, 0, 0);";
		str += "<div style='width:10px;height:10px;" +
				"background-color:" + color + ";" +
				"float: left;" +
				"border-radius: 7px;" +
				"border: 2px solid white;" +
				"margin: 2px 0 0 6px;" +								
				"'></div>";								
	}
	else if(valueType == "Timestamp")				
		str += ConfigurationAPI.getDateString(new Date((value|0)*1000));						
	else					
		str += value;

	
	Debug.log(str);
	
	fieldEl.innerHTML = str;
	
	return fieldEl;
}

//=====================================================================================
//getEditableFieldValue ~~
//	return value is the string value
ConfigurationAPI.getEditableFieldValue = function(fieldObj,fieldIndex,depthIndex /*optional*/)
{
	//TODO copy from tree save
}

//=====================================================================================
//setEditableFieldValue ~~
//	input value is expected to be the string value
ConfigurationAPI.setEditableFieldValue = function(fieldObj,value,fieldIndex,depthIndex /*optional*/)
{
	//TODO copy from elem create
}



//=====================================================================================
//handleEditableFieldClick ~~
//	handler for click event for editable field elements
//
//	copied from ConfigurationGUI.html handleTreeNodeClick() ..but only 
//		defines functionality for value nodes.
ConfigurationAPI.handleEditableFieldClick = function(depth,uid,editClick,type)
{
	var idString = depth + "-" + uid;
	ConfigurationAPI.editableFieldEditingIdString_ = idString;

	Debug.log("handleEditableFieldClick editClick " + editClick);
	Debug.log("handleEditableFieldClick idString " + idString);
	
	var el = document.getElementById("treeNode-Value-" + idString);
	
	if(!el)
	{
		Debug.log("Invalid element pointed to by idString. Ignoring and exiting.");
		return;
	}

	if(ConfigurationAPI.editableFieldHoveringCell_)
	{
		//Debug.log("handleTreeNodeClick editClick clearing ");
		ConfigurationAPI.handleEditableFieldBodyMouseMove();
	}

	if(ConfigurationAPI.editableFieldEditingCell_) //already have the edit box open, cancel it
	{
		if(ConfigurationAPI.editableFieldEditingCell_ == el) //if same cell do nothing
			return true;
		ConfigurationAPI.handleEditableFieldEditOK(); //if new cell, click ok on old cell before continuing
	}

	var path = document.getElementById("treeNode-path-" + idString).textContent;

	//			Debug.log("handleEditableFieldClick el       " + el.innerHTML);
	//Debug.log("handleEditableFieldClick idString    " + idString);
	//Debug.log("handleEditableFieldClick uid      " + uid);
	//			Debug.log("handleEditableFieldClick nodeName " + nodeName);
	Debug.log("handleEditableFieldClick path     " + path);
	//Debug.log("handleEditableFieldClick editClick " + editClick);
	Debug.log("handleEditableFieldClick type     " + type);
	
	//determine type clicked:
	//	- value
	//
	//allow different behavior for each	depending on single or edit(2x) click	
	//	- value		 	
	//		1x = select node 
	//		2x = edit record Value mode (up/down tab/shtab enter esc active)
	//
	//on tree node edit OK	
	//	- value
	//		save value back to field element
	//
	//on tree node edit cancel
	//	return previous value back to field element


	//==================
	//take action based on editClick and type string:
	//	- value
	//
	//params:
	//	(el,depth,uid,path,editClick,type,delayed)

	if(editClick)	//2x click
	{
		
		if(type == "value")
		{
			//edit ID (no keys active)
			Debug.log("edit value mode");

			selectThisTreeNode(idString,type);
			function selectThisTreeNode(idString,type)
			{				
				//edit column entry in record
				//	data type matters here, also don't edit author, timestamp
				var el = document.getElementById("treeNode-Value-leafNode-" + idString);
				var vel = document.getElementById("treeNode-Value-" + idString);
				
				//if value node, dataType is in element class name
				var colType = vel.className.split(' ')[1].split('-');
				if(colType[1] == "ValueType")
					colType = colType[2];

				var fieldName = el.className.substr(("treeNode-Value-leafNode-ColumnName-").length);
				
				Debug.log("fieldName=" + fieldName);
				Debug.log("colType=" + colType);
				
				if(colType == "Author" || 
						colType == "Timestamp")
				{
					Debug.log("Can not edit Author or Timestamp fields.",
							Debug.WARN_PRIORITY);
					return false;
				}
				
				
				var str = "";		
				var optionIndex = -1;
				
				
				if(colType == "YesNo" || 
						colType == "TrueFalse" || 
						colType == "OnOff")  //if column type is boolean, use dropdown
				{
					type += "-bool";
					ConfigurationAPI.editableFieldEditingOldValue_ = el.innerHTML;
					
					var initVal = el.childNodes[0].textContent;
					ConfigurationAPI.editableFieldEditingInitValue_ = initVal;
					
					var boolVals = [];
					if(colType == "YesNo")
						boolVals = ["No","Yes"];
					else if(colType == "TrueFalse")
						boolVals = ["False","True"];
					else if(colType == "OnOff")
						boolVals = ["Off","On"];


					str += "<select  onkeydown='ConfigurationAPI.handleEditableFieldKeyDown(event)' " +
							"onmousedown='ConfigurationAPI.editableFieldMouseIsSelecting_ = true; Debug.log(ConfigurationAPI.editableFieldMouseIsSelecting_);' " +
							"onmouseup='ConfigurationAPI.editableFieldMouseIsSelecting_ = false; Debug.log(ConfigurationAPI.editableFieldMouseIsSelecting_); event.stopPropagation();' " +
							"onclick='event.stopPropagation();'" +
							"style='margin:-8px -2px -2px -1px; height:" + (el.offsetHeight+6) + "px'>";
					for(var i=0;i<boolVals.length;++i)
					{			
						str += "<option value='" + boolVals[i] + "'>";
						str += boolVals[i];	//can display however
						str += "</option>";
						if(boolVals[i] == initVal)
							optionIndex = i; //get starting sel index					
					}			
					str += "</select>";
					if(optionIndex == -1) optionIndex = 0; //use False option by default					
				}
				else if(colType == "FixedChoiceData")
				{
					ConfigurationAPI.editableFieldEditingOldValue_ = el.textContent;
					ConfigurationAPI.editableFieldEditingInitValue_ = ConfigurationAPI.editableFieldEditingOldValue_;
					
					str += "<select  onkeydown='ConfigurationAPI.handleEditableFieldKeyDown(event)' " +
							"onmouseup='event.stopPropagation();' " +
							"onclick='event.stopPropagation();' " +
							"style='margin:-8px -2px -2px -1px; height:" + (el.offsetHeight+6) + "px'>";

					//default value is assumed in list

					var vel = document.getElementById("treeNode-FixedChoice-CSV-" +
							idString);
					var choices = vel.textContent.split(',');
					
					for(var i=0;i<choices.length;++i)
					{			
						str += "<option>";
						str += decodeURIComponent(choices[i]);	//can display however
						str += "</option>";
						if(decodeURIComponent(choices[i]) 
								== ConfigurationAPI.editableFieldEditingOldValue_)
							optionIndex = i; //get starting sel index
					}				
					str += "</select>";	
				}
				else if(colType == "BitMap")
				{
					Debug.log("Handling bitmap select");
					
					ConfigurationAPI.editableFieldEditingOldValue_ = el.textContent;
										
					//let API bitmap dialog handle it
					ConfigurationAPI.bitMapDialog(
							//_editingCellElOldTitle, //field name
							"Target Field: &quot;" + 
							fieldName_ + "&quot;",
							document.getElementById("treeNode-BitMap-Params-" +
														idString).textContent.split(';'), 
							ConfigurationAPI.editableFieldEditingOldValue_,
							function(val)
							{
						Debug.log("yes " + val);
						el.innerHTML = "";
						el.appendChild(document.createTextNode(val));
						ConfigurationAPI.editableFieldEditingCell_ = el;
						
						type += "-bitmap";
						editTreeNodeOK();
							
							},
							function() //cancel handler
							{							
								//remove the editing cell selection
								Debug.log("cancel bitmap");
								ConfigurationAPI.editableFieldEditingCell_ = 0;
							});
					return true;
				}
				else if(colType == "MultilineData")
				{
					ConfigurationAPI.editableFieldEditingOldValue_ = el.textContent;
					ConfigurationAPI.editableFieldEditingInitValue_ = ConfigurationAPI.editableFieldEditingOldValue_;
					
					str += "<textarea rows='4' onkeydown='ConfigurationAPI.handleEditableFieldKeyDown(event)' cols='50' style='font-size: 14px; " +
							"margin:-8px -2px -2px -1px;width:" + 
							(el.offsetWidth-6) + "px; height:" + (el.offsetHeight-8) + "px' ";
					str += " onmousedown='ConfigurationAPI.editableFieldMouseIsSelecting_ = true; Debug.log(ConfigurationAPI.editableFieldMouseIsSelecting_);' " +
							"onmouseup='ConfigurationAPI.editableFieldMouseIsSelecting_ = false; Debug.log(ConfigurationAPI.editableFieldMouseIsSelecting_);event.stopPropagation();' " +
							"onclick='event.stopPropagation();'" +
							">";
					str += ConfigurationAPI.editableFieldEditingOldValue_;
					str += "</textarea>";				
				}
				else // normal cells, with text input
				{
					if(colType == "GroupID") //track type if it is groupid field
						type += "-groupid";
					
					ConfigurationAPI.editableFieldEditingOldValue_ = el.textContent;
					ConfigurationAPI.editableFieldEditingInitValue_ = ConfigurationAPI.editableFieldEditingOldValue_;
					
					var ow = el.offsetWidth+6;
					if(ow < 150) //force a minimum input width
						ow = 150;
					str += "<input type='text' onkeydown='ConfigurationAPI.handleEditableFieldKeyDown(event)' style='margin:-8px -2px -2px -1px;width:" + 
							(ow) + "px; height:" + (el.offsetHeight>20?el.offsetHeight:20) + "px' value='";
					str += ConfigurationAPI.editableFieldEditingOldValue_;
					str += "' onmousedown='ConfigurationAPI.editableFieldMouseIsSelecting_ = true; Debug.log(ConfigurationAPI.editableFieldMouseIsSelecting_);' " +
							"onmouseup='ConfigurationAPI.editableFieldMouseIsSelecting_ = false; Debug.log(ConfigurationAPI.editableFieldMouseIsSelecting_);event.stopPropagation();' " +
							"onclick='event.stopPropagation();'" +
							">";					
				}


				str += ConfigurationAPI._OK_CANCEL_DIALOG_STR;	

				el.innerHTML = str;
				
				//handle default selection
				if(colType == "YesNo" || 
						colType == "TrueFalse" || 
						colType == "OnOff")  //if column type is boolean, use dropdown
				{					//select initial value
					el.getElementsByTagName("select")[0].selectedIndex = optionIndex;
					el.getElementsByTagName("select")[0].focus();
				}
				else if(colType == "FixedChoiceData")
				{
					el.getElementsByTagName("select")[0].selectedIndex = optionIndex;
					el.getElementsByTagName("select")[0].focus();				
				}
				else if(colType == "MultilineData")
					ConfigurationAPI.setCaretPosition(el.getElementsByTagName("textarea")[0],0,ConfigurationAPI.editableFieldEditingOldValue_.length);
				else 					//select text in new input
					ConfigurationAPI.setCaretPosition(el.getElementsByTagName("input")[0],0,ConfigurationAPI.editableFieldEditingOldValue_.length);

				
				//wrapping up
				ConfigurationAPI.editableFieldEditingCell_ = el;
				ConfigurationAPI.editableFieldEditingNodeType_ = type;
			}
		}
		else
		{
			Debug.log("This should be impossible - tell a developer how you got here!", Debug.HIGH_PRIORITY);
			return;
		}
	}
	else		//1x click
	{
		if(type == "value")
		{
			//Mark selected
			Debug.log("Selecting field");
			
			//add previously selected 
			if(ConfigurationAPI.editableFieldSelectedCell_)
				ConfigurationAPI.editableFieldSelectedCell_.style.backgroundColor = "transparent";
			
			//add newly selected 
			var vel = document.getElementById("treeNode-Value-" + 
					idString);
			vel.style.backgroundColor = ConfigurationAPI.editableField_SELECTED_COLOR_;
			ConfigurationAPI.editableFieldSelectedCell_ = vel;
		}
		else
		{
			Debug.log("This should be impossible - tell a developer how you got here!", Debug.HIGH_PRIORITY);
			return;
		}
	}


}

//=====================================================================================
//handleEditableFieldHover ~~
//	handler for mousemove event for editable field elements
ConfigurationAPI.handleEditableFieldHover = function(depth,uid,event)
{
	var idString = depth + "-" + uid;

	//Debug.log("handleEditableFieldHover idString " + idString);
	
	event.stopPropagation();
	DesktopContent.mouseMove(event); //keep desktop content happy

	
	if(ConfigurationAPI.editableFieldEditingCell_) return; //no setting while editing

	var el = document.getElementById("treeNode-Value-editIcon-" + idString);
	if(ConfigurationAPI.editableFieldHoveringCell_ == el) return;

	if(ConfigurationAPI.editableFieldHoveringCell_)
	{
		//Debug.log("bodyMouseMoveHandler clearing ");
		bodyMouseMoveHandler();
	}

	//Debug.log("handleTreeNodeMouseMove setting ");
	ConfigurationAPI.editableFieldHoveringIdString_ = idString;
	ConfigurationAPI.editableFieldHoveringCell_ = el;
	ConfigurationAPI.editableFieldHoveringCell_.style.display = "block";	
	var vel = document.getElementById("treeNode-Value-" + 
			ConfigurationAPI.editableFieldHoveringIdString_);
	vel.style.backgroundColor = "rgb(218, 194, 194)";
}

//=====================================================================================
//handleEditableFieldBodyMouseMove ~~
ConfigurationAPI.handleEditableFieldBodyMouseMove = function(e)
{
	if(ConfigurationAPI.editableFieldHoveringCell_)
	{
		//Debug.log("bodyMouseMoveHandler clearing ");
		ConfigurationAPI.editableFieldHoveringCell_.style.display = "none";
		ConfigurationAPI.editableFieldHoveringCell_ = 0;

		var vel = document.getElementById("treeNode-Value-" + 
				ConfigurationAPI.editableFieldHoveringIdString_);
		if(vel)
		{
			if(vel == ConfigurationAPI.editableFieldSelectedCell_)
				vel.style.backgroundColor = ConfigurationAPI.editableField_SELECTED_COLOR_;
			else				
				vel.style.backgroundColor = "transparent";
		}
	}
}

//=====================================================================================
//handleEditableFieldKeyDown ~~
//	copied from ConfigurationGUI keyHandler but modified for only value cells
ConfigurationAPI.handleEditableFieldKeyDown = function(e,keyEl)
{
	var TABKEY = 9;
	var ENTERKEY = 13;
	var UPKEY = 38;
	var DNKEY = 40;
	var ESCKEY = 27;
	//Debug.log("key " + e.keyCode);
	
	var shiftIsDown;
	if (window.event) 
	{
		key = window.event.keyCode;
		shiftIsDown = !!window.event.shiftKey; // typecast to boolean
	} 
	else
	{
		key = e.which;
		shiftIsDown = !!e.shiftKey;	// typecast to boolean
	}
	//Debug.log("shift=" + shiftIsDown);
	
		
	//handle text area specially
	if(!shiftIsDown)
	{
		var tel;
		if(ConfigurationAPI.editableFieldEditingCell_ && 
				(tel = ConfigurationAPI.editableFieldEditingCell_.getElementsByTagName("textarea")).length)
		{
			tel = tel[0];
			//handle special keys for text area
			if(e.keyCode == TABKEY)
			{
				Debug.log("tab.");
				if(e.preventDefault) 
					e.preventDefault();

				var i = tel.selectionStart;
				var j = tel.selectionEnd;
				tel.value =  tel.value.substr(0,i) + 
						'\t' + tel.value.substr(j);
				tel.selectionStart = tel.selectionEnd = j+1;
			}
			return false; //done if text area was identified
		}
	}
	
	//tab key jumps to next cell with a CANCEL
	//	(enter key does same except saves/OKs value)
	//	shift is reverse jump
	if(e.keyCode == TABKEY || e.keyCode == ENTERKEY || 
			e.keyCode == UPKEY || e.keyCode == DNKEY) 				
	{
		//this.value += "    ";
		if(e.preventDefault) 
			e.preventDefault();

		//save idString
		var idString = ConfigurationAPI.editableFieldEditingIdString_;
		 
		ConfigurationAPI.handleEditableFieldEditOK();
		
		
		//enter key := done
		//tab/shift+tab := move to next field at depth
		//down/up	:= move to next field at depth
		
		
		if(e.keyCode == ENTERKEY) //dont move to new cell
			return false;

		var depth = idString.split('-')[0];
		var uid = idString.split('-')[1];
				
		if((!shiftIsDown && e.keyCode == TABKEY) || e.keyCode == DNKEY) //move to next field
			++uid;
		else if((shiftIsDown && e.keyCode == TABKEY) || e.keyCode == UPKEY) //move to prev field
			--uid;
		if(uid < 0) return false; //no more fields, do nothing
		
		//assume handleEditableFieldClick handles invalid uids on high side gracefully
		ConfigurationAPI.handleEditableFieldClick(depth,uid,1,"value");
		Debug.log("new uid=" + uid);
		
		return false;
	}
	else if(e.keyCode == ESCKEY) 
	{				
		if(e.preventDefault) 
			e.preventDefault();
		ConfigurationAPI.handleEditableFieldEditCancel();
		return false;
	}
	else if((e.keyCode >= 48 && e.keyCode <= 57) || 
			(e.keyCode >= 96 && e.keyCode <= 105))// number 0-9
	{
		//if child link cell or boolean cell			
		var sel;
		if((sel = ConfigurationAPI.editableFieldEditingCell_.getElementsByTagName("select")).length)
		{
			if(keyEl) //if input element, use it
				sel = keyEl;
			else
				sel = sel[sel.length-1]; //assume the last select in the cell is the select
			
			//select based on number
			var selNum;
			if(e.keyCode >= 96)
				selNum = e.keyCode - 96;
			else
				selNum = e.keyCode - 48;
			
			sel.selectedIndex = selNum % (sel.options.length);
			sel.focus();	
			
			Debug.log("number select =" + sel.selectedIndex);
			if(sel.onchange) //if onchange implemented call it
				sel.onchange();
		}
	}
}

//=====================================================================================
//handleEditableFieldEditCancel ~~
//	copied from ConfigurationGUI editCellCancel but modified for only value cells
ConfigurationAPI.handleEditableFieldEditCancel = function()
{	
	if(!ConfigurationAPI.editableFieldEditingCell_) return;
	Debug.log("handleEditableFieldEditCancel type " + ConfigurationAPI.editableFieldEditingNodeType_);

	if(ConfigurationAPI.editableFieldEditingNodeType_ == "value-bool") 
	{
		//take old value as HTML for bool values
		ConfigurationAPI.editableFieldEditingCell_.innerHTML = ConfigurationAPI.editableFieldEditingOldValue_;		
	}
	else
	{
		ConfigurationAPI.editableFieldEditingCell_.innerHTML = "";
		ConfigurationAPI.editableFieldEditingCell_.appendChild(
				document.createTextNode(ConfigurationAPI.editableFieldEditingOldValue_));
	}

	ConfigurationAPI.editableFieldEditingCell_ = 0;
}

//=====================================================================================
//handleEditableFieldEditOK ~~
//	copied from ConfigurationGUI editCellOK but modified for only value cells
ConfigurationAPI.handleEditableFieldEditOK = function()
{							
	if(!ConfigurationAPI.editableFieldEditingCell_) return;
	Debug.log("handleEditableFieldEditOK type " + ConfigurationAPI.editableFieldEditingNodeType_);
		

	var el = ConfigurationAPI.editableFieldEditingCell_;
	var type = ConfigurationAPI.editableFieldEditingNodeType_;
	
	///////////////////////////////////////////////
	//localEditTreeNodeOKRequestsComplete
	function localEditTreeNodeOKRequestsComplete(newValue)
	{				
		// all value types, clear the cell first		
		el.innerHTML = "";
		
		
		if(type == "value" || 
				type == "value-bitmap")
		{
			//if(type == "MultilineData")
			//MultilineData and normal
			//normal data
			//bitmap data (do nothing would be ok, value already set)
			el.appendChild(document.createTextNode(decodeURIComponent(newValue)));
			
		}
		else if(type == "value-bool")
		{
			var str = "";
			
			//colorize true false														
			str += "<div style='float:left'>";										
			str += newValue;											
			str += "</div>";

			var color = (newValue == "On" || newValue == "Yes" || newValue == "True")?
					"rgb(16, 204, 16)":"rgb(255, 0, 0);";
			str += "<div style='width:10px;height:10px;" +
					"background-color:" + color + ";" +
					"float: left;" +
					"border-radius: 7px;" +
					"border: 2px solid white;" +
					"margin: 2px 0 0 6px;" +								
					"'></div>"; 
			el.innerHTML = str;
		}			
		else if(type == "value-groupid")
		{					
			el.appendChild(document.createTextNode(newValue));
		}
		else	//unrecognized type!?
		{
			Debug.log("Unrecognizd tree edit type! Should be impossible!",Debug.HIGH_PRIORITY);
			ConfigurationAPI.handleEditableFieldEditCancel(); return;
		}
		
		//requests are done, so end editing
		ConfigurationAPI.editableFieldEditingCell_ = 0;
	} //end localEditTreeNodeOKRequestsComplete
	///////////////////////////////////////////////


	if(		
			type == "value" ||
			type == "value-bool" || 
			type == "value-bitmap" || 
			type == "value-groupid")
						
	{
		var newValue;
		
		if(type == "value-bool")
		{
			var sel = el.getElementsByTagName("select")[0];
			newValue = sel.options[sel.selectedIndex].value;
		}
		else if(type == "value-bitmap")
		{
			newValue = encodeURIComponent(el.textContent);
		}		
		else	//value (normal or multiline data)
		{
			var sel;
			if((sel = el.getElementsByTagName("textarea")).length) //for MultilineData					
				newValue = sel[0].value; //assume the first textarea in the cell is the textarea
			else if((sel = el.getElementsByTagName("select")).length) //for FixedChoiceData					
				newValue = sel[0].options[sel[0].selectedIndex].value; //assume the first select dropbox in the cell is the one
			else
				newValue = el.getElementsByTagName("input")[0].value;
			
			newValue = encodeURIComponent(newValue.trim());
		}
		
		Debug.log("CfgGUI editTreeNodeOK editing " + type + " node = " +
				newValue);
		
		if(ConfigurationAPI.editableFieldEditingInitValue_ == newValue)
		{
			Debug.log("No change. Do nothing.");
			ConfigurationAPI.handleEditableFieldEditCancel();
			return;
		}
		
		
		//		if saved successfully
		//			update value in field			

		localEditTreeNodeOKRequestsComplete(newValue);				
		
	}
	else	//unrecognized type!?
	{
		Debug.log("Unrecognizd tree edit type! Should be impossible!",Debug.HIGH_PRIORITY);
		editCellCancel(); return;
	}
}


//=====================================================================================
//hasClass ~~
ConfigurationAPI.hasClass = function(ele,cls) 
{
    return !!ele.className.match(new RegExp('(\\s|^)'+cls+'(\\s|$)'));
}

//=====================================================================================
//addClass ~~
ConfigurationAPI.addClass = function(ele,cls) 
{
    if (!ConfigurationAPI.hasClass(ele,cls)) ele.className += " "+cls;
}

//=====================================================================================
//removeClass ~~
ConfigurationAPI.removeClass = function(ele,cls) 
{
    if (ConfigurationAPI.hasClass(ele,cls)) 
    {
    	var reg = new RegExp('(\\s|^)'+cls+'(\\s|$)');
    	ele.className=ele.className.replace(reg,'');
    }
}




























