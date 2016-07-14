// Created by swu at fnal dot gov
//  February 2016
//

	//Function List:
		//init
	    //redrawWindow
		//callWrite
		//callRead
	var CMDHISTDIVINDEX = 0;
	var SEQINDEX = 0;
	var MACROINDEX = 0;
	var FEELEMENTS = [];
	var macroString = [];
	var sortable;
	var stringOfAllMacros = [];
	var tempString = [];
	
	var theAddressStrForRead = ""; // for callread and its handler
	var isOnMacroMakerPage = false;
	var EVENTCOUNTER = 0;
	var timeIntervalID;
	var isMacroRunning = false;
	var isMacroReading = false;
	var SEQFORMAT = "hex";
	
	var arrayOfCommandsForEdit = [];
	var oldMacroNameForEdit = "";
	var newMacroNameForEdit = "";
	var macroDateForEdit = "";
	var macroNotesForEdit = "";
	
	var macroNameForRename = "";
	
	var lastDeletedMacro = "";

	function init() 
	{			
		Debug.log("init() was called");
		DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=FElist","",FElistHandlerFunction);
		block1El = document.getElementById('fecList');
		block2El = document.getElementById('macroLib');
		block3El = document.getElementById('main');
		block4El = document.getElementById('instruction');
		block5El = document.getElementById('history');
		block6El = document.getElementById('sequence');
		block7El = document.getElementById('maker');
		block8El = document.getElementById('popupEditMacro');
		historybox = document.getElementById('historyContent');
		sequencebox = document.getElementById('sequenceContent');
		macrobox = document.getElementById('listOfMacros');
		macroSequenceEdit = document.getElementById("macroSequenceEdit");
		window.onresize = redrawWindow;
		redrawWindow(); //redraw window for the first time
		loadExistingMacros();
		loadUserHistory();
	}
	
	//Handling window resizing
	function redrawWindow() 
	{
		Debug.log("Window redraw to " + window.innerWidth + " - " + window.innerHeight);
						
		var w = window.innerWidth;
		var h = window.innerHeight;
		
		//square [x,y] [w,h]
		var _MARGIN = 5;
		
		var b1 = [_MARGIN, _MARGIN+4*_MARGIN, w/3, h/2-_MARGIN]; //left top
		var b2 = [_MARGIN, h/2+2*_MARGIN, w/3-_MARGIN, h/2-_MARGIN]; //left bottom
		var b3 = [w/3, _MARGIN+4*_MARGIN, w/3, h/2-_MARGIN]; //top middle 
		var b4 = [w/3, h/2+2*_MARGIN, w/3, h/2-_MARGIN]; //bottom middle 
		var b5 = [w*2/3,_MARGIN+4*_MARGIN,w/3-_MARGIN, h-2*_MARGIN]; //right 
		var b6 = [_MARGIN, _MARGIN+4*_MARGIN,w/3-2*_MARGIN, h-2*_MARGIN]; //left
		var b7 = [w/3, _MARGIN+4*_MARGIN, w/3, h/2-_MARGIN];//middle
		var b8 = [w/2-200,h/5,2*w/3,3*h/5];
		
		block1El.style.left = b1[0] + "px";
		block1El.style.top =  b1[1] + "px";
		block1El.style.width =  b1[2] + "px";
		block1El.style.height =  b1[3] + "px";
		
		block2El.style.left = b2[0] + "px";
		block2El.style.top =  b2[1] + "px";
		block2El.style.width =  b2[2] + "px";
		block2El.style.height =  b2[3] + "px";
		
		block3El.style.left = b3[0] + "px";
		block3El.style.top =  b3[1] + "px";
		block3El.style.width =  b3[2] + "px";
		block3El.style.height =  b3[3] + "px";
		
		block4El.style.left = b4[0] + "px";
		block4El.style.top =  b4[1] + "px";
		block4El.style.width =  b4[2] + "px";
		block4El.style.height =  b4[3] + "px";
		
		block5El.style.left = b5[0] + "px";
		block5El.style.top =  b5[1] + "px";
		block5El.style.width =  b5[2] + "px";
		block5El.style.height =  b5[3] + "px";
		
		block6El.style.left = b6[0] + "px";
		block6El.style.top =  b6[1] + "px";
		block6El.style.width =  b6[2] + "px";
		block6El.style.height =  b6[3] + "px";
		
		block7El.style.left = b7[0] + "px";
		block7El.style.top =  b7[1] + "px";
		block7El.style.width =  b7[2] + "px";
		block7El.style.height =  b7[3] + "px";
		
		block8El.style.left = b8[0] + "px";
		block8El.style.top =  b8[1] + "px";
		block8El.style.height =  b8[3] + "px";
		
		historybox.style.height =  h*0.88 + "px";
		sequencebox.style.height =  h*0.88 + "px";
		macrobox.style.height =  h*0.40 + "px";
		macroSequenceEdit.style.height = h*0.6-250 + "px";
	}
			 
	function FElistHandlerFunction(req) 
	{
		Debug.log("FElistHandlerFunction() was called. Req: " + req.responseText);
	    FEELEMENTS = req.responseXML.getElementsByTagName("FE");
		console.log(FEELEMENTS);
		
		//Make search box for the list
		var noMultiSelect = false; 									
				
	    var keys = [];
	    var vals = [];
	    var types = [];
	    var fullnames = [];
	    //Only displays the first 11 letters, mouse over display full name
	    for(var i=0;i<FEELEMENTS.length;++i)
		{
			keys[i] = "one";
			fullnames[i] = FEELEMENTS[i].getAttribute("value");
			var sp = fullnames[i].split(":");
			if (sp[0].length < 11) vals[i] = fullnames[i];
			else{
				var display = sp[0].substr(0,4)+"...:"+sp[1]+":"+sp[2];
				vals[i] = "<abbr title='" + fullnames[i] + "'>"+display+"</abbr>";
			}
			types[i] = "number";
			Debug.log(vals[i]);
		}
	    var listoffecs = document.getElementById('list');  
		MultiSelectBox.createSelectBox(listoffecs,
				"box1",
				"Please select from below:",
				vals,keys,types,"listSelectionHandler",noMultiSelect);            
	    //End of making box
	}

	function listSelectionHandler(listoffecs)
	{
	 	 var splits = listoffecs.id.split('_');
	 	 console.log(splits);
		 elementIndex = splits[splits.length-1] | 0;
		 MultiSelectBox.dbg("Chosen element index:",elementIndex);
	}
            
    function clearData()
    {
         document.getElementById('dataInput').value = "";
    }
    
    function clearMacroData()
    {
		document.getElementById('macroDataInput').value = "";
    }

    function clearAddress()
    {
         document.getElementById('addressInput').value = "";
    }
    
    function clearMacroAddress()
	{
		 document.getElementById('macroAddressInput').value = "";
	}

    function callWrite(address,data)
    {
    	var reminderEl = document.getElementById('reminder');
    	if(isArrayAllZero(selected))
    		reminderEl.innerHTML = "Please select at least one interface from the list";
		else 
		{ 
			var addressFormatStr = document.getElementById("addressFormat").value;
			var dataFormatStr = document.getElementById("dataFormat").value;
			
			if (typeof address === 'undefined') 
			{ 
				var addressStr = document.getElementById('addressInput').value.toString();
				var dataStr = document.getElementById('dataInput').value.toString();
				if(addressStr == "") 
				{
					reminderEl.innerHTML = "Please enter an address to write to";
					return;
				}
				else if(dataStr == "") 
				{
					reminderEl.innerHTML = "Please enter your data";
					return;
				}
			}
			else
			{
				var addressStr = address.toString();
				var dataStr = data.toString();
			}
			
			if (addressStr.substr(0,2)=="0x") addressStr = addressStr.substr(2);
			if (dataStr.substr(0,2)=="0x") dataStr = dataStr.substr(2);

			var selectionStrArray = [];
			var supervisorIndexArray = [];
			var interfaceIndexArray = [];
			for (var i = 0; i < selected.length; i++) 
			{
				if (selected[i]!==0) 
			    {
					var oneInterface = FEELEMENTS[i].getAttribute("value")
					selectionStrArray.push(oneInterface);
					supervisorIndexArray.push(oneInterface.split(":")[1]);
					interfaceIndexArray.push(oneInterface.split(":")[2]);
			    }
			}
			var contentEl = document.getElementById('historyContent');
			var innerClass = "class=\"innerClass1\"";
			if (CMDHISTDIVINDEX%2) innerClass = "class=\"innerClass2\"";
			
			var update = "<div " + innerClass + " id = \"" + CMDHISTDIVINDEX + "\"  title=\"" + "Entered: " 
					+ Date().toString() + "\nSelected interface: " + selectionStrArray 
					+ "\" onclick=\"histCmdWriteDivOnclick(" + "'" + addressStr + "','" + dataStr + "','" 
					+ addressFormatStr + "','" + dataFormatStr + "')\">Write [" + dataFormatStr + "]<b>"
					+ dataStr + LSBchecker() + "</b> into register [" + addressFormatStr + "]<b> " 
					+ addressStr + LSBchecker() + "</b></div>";
			
			var convertedAddress = reverseLSB(convertToHex(addressFormatStr,addressStr));
			var convertedData = reverseLSB(convertToHex(dataFormatStr,dataStr));
					
			DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=writeData&Address="
					+convertedAddress+"&Data="+convertedData+"&supervisorIndex="+supervisorIndexArray
					+"&interfaceIndex="+interfaceIndexArray+"&time="+Date().toString()
					+"&interfaces="+selectionStrArray+"&addressFormatStr="+addressFormatStr
					+"&dataFormatStr="+dataFormatStr,"",writeHandlerFunction);
			contentEl.innerHTML += update;
			CMDHISTDIVINDEX++;
			contentEl.scrollTop = contentEl.scrollHeight;
			reminderEl.innerHTML = "Data successfully written!";
		}
    }
  
    function callRead(address)
    {
    	var timeIntervalForRead = setInterval(function(){
			if (isMacroReading == true) return;
				
			else
			{
				var reminderEl = document.getElementById('reminder');
				if(isArrayAllZero(selected))
					reminderEl.innerHTML = "Please select at least one interface from the list";
				else 
				{ 
					isMacroReading = true;
					var addressFormatStr = document.getElementById("addressFormat").value;
					var dataFormatStr = document.getElementById("dataFormat").value;
				
					if (typeof address === 'undefined') 
					{
						theAddressStrForRead = document.getElementById('addressInput').value.toString();
						if(theAddressStrForRead === "") 
						{
							reminderEl.innerHTML = "Please enter an address to read from";
							return;
						}
					}
					else
						theAddressStrForRead = address.toString();
					
					if (theAddressStrForRead.substr(0,2)=="0x") theAddressStrForRead = theAddressStrForRead.substr(2);
					
					var supervisorIndexArray = [];
					var interfaceIndexArray = [];
					for (var i = 0; i < selected.length; i++) 
					{
						if (selected[i]!==0) 
						{
							var oneInterface = FEELEMENTS[i].getAttribute("value")
							supervisorIndexArray.push(oneInterface.split(":")[1]);
							interfaceIndexArray.push(oneInterface.split(":")[2]);
						}
					}
					var convertedAddress = reverseLSB(convertToHex(addressFormatStr,theAddressStrForRead));
					var selectionStrArray = [];
					for (var i = 0; i < selected.length; i++) 
					{
						if (selected[i]!==0) selectionStrArray.push(FEELEMENTS[i].getAttribute("value"));
					}
					DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=readData&Address="
							+convertedAddress+"&supervisorIndex="+supervisorIndexArray
							+"&interfaceIndex="+interfaceIndexArray+"&time="+Date().toString()
							+"&interfaces="+selectionStrArray+"&addressFormatStr="+addressFormatStr
							+"&dataFormatStr="+dataFormatStr,"",readHandlerFunction);
					clearInterval(timeIntervalForRead);
				}
			}
		},100);
    	
    }
    
    function writeHandlerFunction(req)
	{
		Debug.log("writeHandlerFunction() was called. Req: " + req.responseText);
		EVENTCOUNTER--;
    }
    
    function readHandlerFunction(req)
	{
    	var addressFormatStr = document.getElementById("addressFormat").value;
    	var dataFormatStr = document.getElementById("dataFormat").value;
    	var reminderEl = document.getElementById('reminder');
		Debug.log("readHandlerFunction() was called. Req: " + req.responseText);
		var dataOutput = DesktopContent.getXMLValue(req,"readData");
		isMacroReading = false;
		var convertedOutput;
		if (Number(dataOutput)===0) convertedOutput = "<span class='red'>Time out Error</span>";
		else convertedOutput = reverseLSB(convertFromHex(dataFormatStr,dataOutput));
	
		var selectionStrArray = [];
		for (var i = 0; i < selected.length; i++) 
		{
			if (selected[i]!==0) selectionStrArray.push(FEELEMENTS[i].getAttribute("value"));
		}
		var innerClass = "class=\"innerClass1\"";
		if (CMDHISTDIVINDEX%2) innerClass = "class=\"innerClass2\"";
		var contentEl = document.getElementById('historyContent');

		var update = "<div " + innerClass + " id = \"" + CMDHISTDIVINDEX + "\" title=\"" + "Entered: " + Date().toString()
				+ "\nSelected interface: " + selectionStrArray + "\" onclick=\"histCmdReadDivOnclick(" +"'" 
				+ theAddressStrForRead + "','" + addressFormatStr + "'" + ")\">Read [" + dataFormatStr + "]<b>" 
				+ convertedOutput + LSBchecker()
				+ "</b> from register [" + addressFormatStr + "]<b>" + theAddressStrForRead + LSBchecker() + "</b></div>";
		theAddressStrForRead = "";
		contentEl.innerHTML += update;
		CMDHISTDIVINDEX++; 
		contentEl.scrollTop = contentEl.scrollHeight;
		reminderEl.innerHTML = "Data read: " + convertedOutput;
		EVENTCOUNTER--;
	}
    
    function isArrayAllZero(arr)
    {
        for(var j = 0; j < arr.length; j++)
        {
          if (arr[j]!==0) return false;
        }
        return true;
    }
    
    function convertToHex(format,target)
    {
		switch (format) 
		{
			case "hex": 
				return target;
			case "dec": //dec
				return Number(target).toString(16);
			case "ascii": //ascii
				var output = [];
				for(var i = target.length-1; i>=0; i--)
					 output.push(target.charCodeAt(i).toString(16));
				return output.join('');
		}
    }
    
    function convertFromHex(format,target)
    {
		switch (format) 
		{
	      case "hex":
			return target;
		  case "dec":
			return parseInt(target,16).toString();
		  case "ascii":
			var str = '';
			for (var i = 0; i < target.length; i += 2)
			str += String.fromCharCode(parseInt(target.substr(i, 2), 16));
			return str;
		}
    }
    
    function toggleLSBF() //Only listens to addressFormat
    {
    	var addressFormatStr = document.getElementById("addressFormat").value;
        var macroAddressInputEl = document.getElementById('macroAddressInput');

        switch (addressFormatStr) {
          case "hex":
            document.getElementById("lsbFirst").checked = true;
            document.getElementById("lsbFirst").disabled = false;
        //    macroAddressInputEl.setAttribute("placeholder", "0x64");
            break;
          case "dec":
            document.getElementById("lsbFirst").checked = false;
            document.getElementById("lsbFirst").disabled = true;
         //   macroAddressInputEl.setAttribute("placeholder", "1234");
            break;
          case "ascii":
            document.getElementById("lsbFirst").checked = false;
            document.getElementById("lsbFirst").disabled = false;
      //      macroAddressInputEl.setAttribute("placeholder", "Hi");
            break;
        }
    }
    
    function reverseLSB(original)
    {
    	if(document.getElementById("lsbFirst").checked)
		{
			var str = '';
			if(original.length%2) 
				original = "0"+original;
			for (var i = original.length-2; i > -2; i -= 2)
			  str += original.substr(i,2);
			return str;
		}
    	else return original;
    }
    
    function LSBchecker()
    {
    	if(document.getElementById("lsbFirst").checked) return "*";
    	else return "";
    }
    
    function toggleDisplay(onMacro)
    {
    	 var fecListEl = document.getElementById("fecList");
    	 var macroLibEl = document.getElementById("macroLib");
    	 var sequenceEl = document.getElementById("sequence");
    	 var instructionEl = document.getElementById("instruction");
    	 var mainEl = document.getElementById("main");
    	 var makerEl = document.getElementById("maker");
    	 
    	 if (onMacro) {
    		 isOnMacroMakerPage = true;
    		 fecListEl.style.display = "none";
    		 macroLibEl.style.display = "none";
    		 sequenceEl.style.display = "block";
    		 instructionEl.style.display = "none";
    		 mainEl.style.display = "none";
    		 makerEl.style.display = "block";
    	 } else {
    		 isOnMacroMakerPage = false;
    		 fecListEl.style.display = "block";
    		 macroLibEl.style.display = "block";
    		 sequenceEl.style.display = "none";
    		 instructionEl.style.display = "block";
    		 mainEl.style.display = "block";
    		 makerEl.style.display = "none";
    	 }
    }
   
	
    function addCommand(command,address,data)
    {
		var contentEl = document.getElementById('sequenceContent');
		var macroReminderEl = document.getElementById('macroReminder');
		macroReminderEl.innerHTML = "";
		switch(command)
		{
		case 'w':
	    	if (typeof address === 'undefined') 
			{ 
				var addressStr = document.getElementById('macroAddressInput').value.toString();
				var dataStr = document.getElementById('macroDataInput').value.toString();
	    		if(addressStr === "") 
				{
					macroReminderEl.innerHTML = "Please enter an address to write to";
					return;
				} 
	    		else if(dataStr === "") 
	    		{
					macroReminderEl.innerHTML = "Please enter your data";
					return;
				}
			} 
	    	else 
			{
				var addressStr = address.toString();
				var dataStr = data.toString();
			}
				var update = "<div id = \"seq" + SEQINDEX + "\" data-id =" + SEQINDEX 
						+ " onmouseout=\"hideDeletex(" + SEQINDEX + ")\" onmouseover=\"showDeletex(" + SEQINDEX + ")\" ondragstart=\"hideDeletex(" + SEQINDEX + ")\" ondragend=\"getOrder()\"  class=\"seqDiv\"><p class=\"insideSEQ textSEQ\">Write <b>" + convertFromHex(SEQFORMAT,dataStr) + "</b> into <b>" 
						+ convertFromHex(SEQFORMAT,addressStr) + "</b></p><div id=\"deletex" + SEQINDEX + "\" class=\"insideSEQ deletex\" onclick=\"removeCommand(" 
						+ SEQINDEX + ")\"><b>X</b></div></div>";
				var writeMacroString = SEQINDEX + ":w:" + addressStr + ":" + dataStr;
				macroString.push(writeMacroString);
			break;
		case 'r':
			if (typeof address === 'undefined') 
			{ 
				var addressStr = document.getElementById('macroAddressInput').value.toString();
				if(addressStr === "") 
				{
					macroReminderEl.innerHTML = "Please enter an address to read from";
					return;
				}
			} 
			else var addressStr = address.toString();
			var update = "<div id = \"seq" + SEQINDEX + "\" data-id =" + SEQINDEX 
					+ " onmouseout=\"hideDeletex(" + SEQINDEX + ")\" onmouseover=\"showDeletex(" + SEQINDEX + ")\" ondragstart=\"hideDeletex(" + SEQINDEX + ")\" ondragend=\"getOrder()\" class=\"seqDiv\"><p class=\"insideSEQ\">Read from <b>" + convertFromHex(SEQFORMAT,addressStr)
					+ "</b></p><div id=\"deletex" + SEQINDEX + "\" class=\"insideSEQ deletex\" onclick=\"removeCommand(" 
					+ SEQINDEX + ")\"><b>X</b></div></div>";
			var readMacroString = SEQINDEX+":r:"+addressStr;
			macroString.push(readMacroString);
			break;
		case 'd':
			if (typeof address === 'undefined') 
			{ 
				var delayStr = document.getElementById('delayInput').value.toString();
				if(addressStr === "") 
				{
					macroReminderEl.innerHTML = "Please enter a delay";
					return;
				}
			}
			else var delayStr = address.toString();
			var update = "<div id = \"seq" + SEQINDEX + "\" data-id =" + SEQINDEX 
					+ " onmouseout=\"hideDeletex(" + SEQINDEX + ")\" onmouseover=\"showDeletex(" + SEQINDEX + ")\" ondragstart=\"hideDeletex(" + SEQINDEX + ")\" ondragend=\"getOrder()\" class=\"seqDiv\"><p class=\"insideSEQ\">Delay <b>" + delayStr
					+ "</b> s</p><div id=\"deletex" + SEQINDEX + "\" class=\"insideSEQ deletex\" onclick=\"removeCommand(" 
					+ SEQINDEX + ")\"><b>X</b></div></div>";
			var delayMacroString = SEQINDEX+":d:"+delayStr;
			macroString.push(delayMacroString);
			break;
		default: 
			Debug.log("So if it's not write, read, or delay, what is it??");
		}
		contentEl.innerHTML += update;
		SEQINDEX++;
		contentEl.scrollTop = contentEl.scrollHeight;
		sortable = Sortable.create(contentEl,{
				chosenClass: 'chosenClassInSequence',
				ghostClass:'ghostClassInSequence'
		});//Works like magic!
		getOrder();
    }
    
    function hideDeletex(seqIndex)
    {
    	var deleteID = "deletex"+seqIndex;
    	document.getElementById(deleteID).style.display = "none"; 
    }
    
    function showDeletex(seqIndex)
    {
    	var deleteID = "deletex"+seqIndex;
    	var deleteEl = document.getElementById(deleteID);
    	deleteEl.style.top = (deleteEl.parentNode.offsetTop + 1) + "px";
    	deleteEl.style.left = (deleteEl.parentNode.offsetLeft + 
    			deleteEl.parentNode.offsetWidth - 20) + "px";
    	deleteEl.style.display = "block";    	
    }
    
    function getOrder()
    {
    	tempString = [];
		var order = sortable.toArray();
		var sorting = order.slice();
		sorting.sort();
		for(var i = 0; i < macroString.length; i++)
			tempString.push(macroString[sorting.indexOf(order[i])]);
		console.log("new sequence: "+tempString);
    }
    
    function removeCommand(seqIndex)
    {
    	document.getElementById("undoDelete").disabled = false;
    	var child = document.getElementById("seq"+seqIndex);
		var parent = document.getElementById('sequenceContent');
		parent.removeChild(child);
		for (var i = 0; i < macroString.length; i++)
		{
		    if (seqIndex == macroString[i].split(":")[0])
		    {
		      lastDeletedMacro = macroString[i];
		      macroString.splice(i,1);
		    }	  
		}
		getOrder();
    }
    
    function undoDelete()
    {
    	addCommand(lastDeletedMacro.split(":")[1],lastDeletedMacro.split(":")[2],lastDeletedMacro.split(":")[3]);
    	document.getElementById("undoDelete").disabled = true;
    }
    
    function showPopupClearAllConfirm()
    {
		var popupClearAllConfirm = document.getElementById("popupClearAllConfirm");
		popupClearAllConfirm.style.display = "block";
    }
    
    function showPopupClearHistoryConfirm()
    {
		var popupClearAllConfirm = document.getElementById("popupClearHistoryConfirm");
		popupClearAllConfirm.style.display = "block";
    }
	
    function clearAll(el)
    {
		var contentEl = document.getElementById('sequenceContent');
		contentEl.innerHTML = "";
		macroString = [];
		hideSmallPopup(el);
    }
    
    function clearHistory(el)
    {
		DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=clearHistory","",clearHistoryHandlerFunction);
		var contentEl = document.getElementById('historyContent');
		contentEl.innerHTML = "";
		hideSmallPopup(el);
    }
    
    function clearHistoryHandlerFunction(req)
	{
		Debug.log("clearHistoryHandlerFunction() was called. Req: " + req.responseText);
		loadUserHistory();
	}
    
    function hideSmallPopup(el)
    {
    	var wholeDiv = el.parentNode.parentNode.parentNode;
    	wholeDiv.style.display = "none";
    }
    
    function saveMacro()
    {	
    	if (macroString.length === 0) 
    		document.getElementById('macroReminder').innerHTML = "Macro sequence cannot be empty";
    	else
    	{
			var popupSaveMacro = document.getElementById("popupSaveMacro");
			popupSaveMacro.style.display = "block";
    	}
    }
    
    function hidePopupSaveMacro()
    {
    	var popupSaveMacro = document.getElementById("popupSaveMacro");
    	popupSaveMacro.style.display = "none";
        document.getElementById("macroName").value="";
        document.getElementById("macroNotes").value="";
    }

    function hidePopupEditMacro()
    {
    	var popupEditMacro = document.getElementById("popupEditMacro");
    	popupEditMacro.style.display = "none";
        arrayOfCommandsForEdit = [];        
    }
   
    function saveAsMacro()
    {
    	getOrder();
    	var macroName = document.getElementById("macroName").value;
    	var Regex = /^[a-z0-9]+$/i;
    	if (!Regex.test(macroName)) alert("Sorry, special characters and spaces are not allowed.");
    	else
    	{
    	var macroNotes = document.getElementById("macroNotes").value;
    	var macroLibEl = document.getElementById('listOfMacros');
    	stringOfAllMacros[MACROINDEX] = tempString;
    	DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=createMacro&Name="+macroName+
    				"&Sequence="+tempString+"&Time="+Date().toString()+"&Notes="
					+macroNotes,"",createMacroHandlerFunction);
    	loadExistingMacros();
    	hidePopupSaveMacro();
    	macroLibEl.scrollTop = macroLibEl.scrollHeight - macroLibEl.clientHeight; 
    	}
    }
    
    function createMacroHandlerFunction(req)
	{
		Debug.log("createMacroHandlerFunction() was called. Req: " + req.responseText);
	}
    
    function runMacro(stringOfCommands,macroName)
    {
    	
		isMacroRunning = true;
		var contentEl = document.getElementById('historyContent');
		
		EVENTCOUNTER = 0;
		var start = "<p class=\"red\"><b><small>-- Start of Macro: " + macroName + " --</small></b></p>";
		contentEl.innerHTML += start;
		contentEl.scrollTop = contentEl.scrollHeight;
		for (var i = 0; i < stringOfCommands.length; i++)
		{
			var Command = stringOfCommands[i].split(":")
			var commandType = Command[1];
			if(commandType=='w'){
				callWrite(Command[2],Command[3]);
				console.log("write "+Command[3]+" into "+Command[2]);
				EVENTCOUNTER++;
			}else if(commandType=='r'){
				callRead(Command[2]);
				console.log("read from "+Command[2]);
				EVENTCOUNTER++;
			}else if(commandType=='d')
				console.log("delay "+Command[2]+"ms");
			else
				console.log("ERROR! Command type "+commandType+" not found");

		}
		timeIntervalID = setInterval(function(){
			if (EVENTCOUNTER !== 0)
				return;
			else
			{
				var end = "<p class=\"red\"><b><small>-- End of Macro: " + macroName + " --</small></b></p>";
				contentEl.innerHTML += end;
				clearInterval(timeIntervalID);
				contentEl.scrollTop = contentEl.scrollHeight;
				isMacroRunning = false;
			}
		},100);
    }
    
    function loadExistingMacros()
    {
    	DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=loadMacros","",loadingMacrosHandlerFunction);
    }
    
    function loadUserHistory()
	{
		DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=loadHistory","",loadingHistHandlerFunction);
	}
    
    function loadingMacrosHandlerFunction(req)
    {
    	Debug.log("loadingMacrosHandlerFunction() was called. Req: " + req.responseText);
    	var hugeStringOfMacros = DesktopContent.getXMLValue(req,"returnMacroStr");
    	
    	if (hugeStringOfMacros && hugeStringOfMacros.length > 0)
    	{
			var macrosArray = hugeStringOfMacros.split("@");
			var out = "";
			console.log(macrosArray);
			for(var i = 0; i < macrosArray.length; i++) 
			{
				var arr = JSON.parse(macrosArray[i]);
				console.log(arr);
				var macroString = arr.sequence.split(",");
				var forDisplay = []; //getting rid of the first element (macroIndex) for display
				for (var j = 0; j < macroString.length; j++)
				    forDisplay.push(macroString[j].split(":").slice(1).join(":"));
				
				stringOfAllMacros[MACROINDEX] = macroString;
				out += "<div title='Sequence: " + forDisplay.join(",") + "\nNotes: "
						+ arr.notes + "\nCreated: " + arr.time
						+ "\' class='macroDiv' data-id=\"" + arr.name + "\" data-sequence=\"" 
						+ macroString + "\" data-notes=\"" 
						+ arr.notes + "\" data-time=\"" 
						+ arr.time + "\" onclick='dealWithVariables(stringOfAllMacros[" 
						+ MACROINDEX + "],\"" + arr.name + "\")'><b>" + arr.name + "</b></br></div>"; 
				MACROINDEX++;
			}
			document.getElementById("listOfMacros").innerHTML = out;
    	}
    	else 
    		document.getElementById("listOfMacros").innerHTML = "";
    }
    
    function loadingHistHandlerFunction(req)
    {
    	Debug.log("loadingHistHandlerFunction() was called. Req: " + req.responseText);
		var hugeStringOfHistory = DesktopContent.getXMLValue(req,"returnHistStr");
		var contentEl = document.getElementById('historyContent');
		if ( !hugeStringOfHistory ) return; //this happens when history doesn't exist
		
		var commandHistArray = hugeStringOfHistory.split("#");
		var out = "";
		for(var i = 0; i < commandHistArray.length; i++) 
		{
			var innerClass = "class=\"innerClass1\"";
			if (CMDHISTDIVINDEX%2) innerClass = "class=\"innerClass2\"";
			
			var arr = JSON.parse(commandHistArray[i]);
			var oneCommand = arr.Command.split(":");
			var commandType = oneCommand[0];
			var addressFormat = arr.Format.split(":")[0];
			var dataFormat = arr.Format.split(":")[1];		
			var convertedAddress = convertFromHex(addressFormat,oneCommand[1]);
			var convertedData = convertFromHex(dataFormat,oneCommand[2]);
			if(commandType=='w')
			{
				out += "<div " + innerClass + " id = \"" + CMDHISTDIVINDEX + "\"  title=\"" + "Entered: " 
						+ arr.Time + "\nSelected interface: " + arr.Interfaces
						+ "\" onclick=\"histCmdWriteDivOnclick(" + "'" + convertedAddress + "','" + convertedData + "','" 
						+ addressFormat + "','" + dataFormat + "')\">Write [" + dataFormat + "]<b>"
						+ convertedData + "</b> into register [" + addressFormat + "]<b> " 
						+ convertedAddress + "</b></div>";
				CMDHISTDIVINDEX++;
			}
			else if(commandType=='r')
			{
				if (Number(convertedData)===0) convertedData = "<span class='red'>Time out Error</span>";
				out += "<div " + innerClass + " id = \"" + CMDHISTDIVINDEX + "\" title=\"" + "Entered: " 
						+ arr.Time + "\nSelected interface: " + arr.Interfaces + "\" onclick=\"histCmdReadDivOnclick(" 
						+ "'" + convertedAddress + "','" + addressFormat + "'" + ")\">Read [" + dataFormat + "]<b>" 
						+ convertedData + "</b> from register [" + addressFormat + "]<b>" + convertedAddress + "</b></div>";
				CMDHISTDIVINDEX++;
			}
			else if(commandType=='d')
				console.log("delay "+oneCommand[1]+"ms");
			else
				console.log("ERROR! Command type "+commandType+" not found");

			contentEl.innerHTML = out;
			contentEl.scrollTop = contentEl.scrollHeight;
		}
    }
    
    function histCmdWriteDivOnclick(addressStr, dataStr, addressFormatStr, dataFormatStr)
    {
		var convertedAddress = reverseLSB(convertToHex(addressFormatStr,addressStr));
		var convertedData = reverseLSB(convertToHex(dataFormatStr,dataStr));
    	if(isOnMacroMakerPage)
    	{
    		addCommand("w",convertedAddress,convertedData);
    	}
    	else callWrite(addressStr, dataStr);
    }
    
    function histCmdReadDivOnclick(addressStr, addressFormatStr)
	{
    	var convertedAddress = reverseLSB(convertToHex(addressFormatStr,addressStr));
		if(isOnMacroMakerPage)
		{
			addCommand("r",convertedAddress)
		}
		else callRead(addressStr);
	}
    
    function macroActionOnRightClick(macroName, macroAction, macroSequence, macroNotes, macroDate)
    {
    	console.log("macroName" + macroName+ "macroAction" +macroAction + "macroSequence" + macroSequence+ "macroNotes" + macroNotes+ "macroDate" +macroDate);
    	switch(macroAction)
    	{
    	case "Delete":
    		DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=deleteMacro&MacroName="
					+macroName,"",deleteMacroHandlerFunction);
    		break;
    	case "Edit":
    		var popupEditMacro = document.getElementById("popupEditMacro");
    		popupEditMacro.style.display = "block";
    		
    		oldMacroNameForEdit = macroName;
    		macroNotesForEdit = macroNotes;
    		macroDateForEdit = macroDate;
    		var seqID = 0;
    		
    		var macroSequenceEditEl = document.getElementById("macroSequenceEdit");
    		arrayOfCommandsForEdit = macroSequence.split(",");
    		var output = "";
    	    		
    		for (var i = 0; i < arrayOfCommandsForEdit.length; i++)
			{
				var Command = arrayOfCommandsForEdit[i].split(":")
				var commandType = Command[1];
				var markColor = "1";
				var disable = "";
				var markColorData = "1";
				var disableData = "";
				if(commandType=='w'){
					if(isNaN(Command[2]))
					{
						markColor = "2";
						disable = "disabled";
					}
					if(isNaN(Command[3]))
					{
						markColorData = "2";
						disableData = "disabled";
					}
					var writeEdit = "<lable>Write 0x<textarea  " + disableData + " cols='12' rows='1' onchange=\"editCommands(this," + seqID + ",3)\">" + Command[3]
						+ "</textarea><div class='variableMark" + markColorData + "' title='Set field to variable' onclick='setFieldToVariable(this," + seqID 
						+ ",2)'>V</div> into address 0x<textarea " + disable + " cols='12' rows='1' onchange=\"editCommands(this," + seqID + ",2)\">" + Command[2] 
						+ "</textarea><div class='variableMark" + markColor + "' title='Set field to variable' onclick='setFieldToVariable(this," + seqID 
						+ ",2)'>V</div><br/></lable>";
					seqID++;
					output += writeEdit;
				}else if(commandType=='r'){
					if(isNaN(Command[2]))
					{
						markColor = "2";
						disable = "disabled";
					}
					var readEdit = "<lable>Read from address 0x<textarea " + disable + " cols='12' rows='1' onchange=\"editCommands(this," + seqID + ",2)\">" + Command[2]
						+ "</textarea><div class='variableMark" + markColor + "' title='Set field to variable' onclick='setFieldToVariable(this," + seqID 
						+ ",2)'>V</div><br/></lable>";
					seqID++;
					output += readEdit;
				}else if(commandType=='d'){
					if(isNaN(Command[2]))
					{
						markColor = "2";
						disable = "disabled";
					}
					var delayEdit = "<lable>Delay <textarea " + disable + " cols='12' rows='1' onchange=\"editCommands(this," + seqID + ",2)\">" + Command[2]
						+ "</textarea><div class='variableMark" + markColor + "' title='Set field to variable' onclick='setFieldToVariable(this," + seqID 
						+ ",2)'>V</div> seconds<br/></lable>";
					seqID++;
				    output += delayEdit;
				}else
					console.log("ERROR! Command type "+commandType+" not found");
			}
    		macroSequenceEditEl.innerHTML = output;
    	
    		var macroNameEl = document.getElementById("macroNameEdit");
    		macroNameEl.value = macroName;
    		var macroNotesEl = document.getElementById("macroNotesEdit");
    		var date = new Date();    		
			var minutes = "";
    		if(date.getMinutes() < 10) 
				 minutes = "0"+date.getMinutes().toString();
    		else  minutes = date.getMinutes();
    		var time = date.getHours() + ":" + minutes + " " + date.toLocaleDateString();
    		macroNotesForEdit = "[Modified " + time + "] " + macroNotes;
    		macroNotesEl.value = macroNotesForEdit;
    		break;
    	case "Start":
    		var sequenceContentEl = document.getElementById("sequenceContent");
    		var temp = sequenceContentEl.innerHTML;
    		sequenceContentEl.innerHTML = "";
    		var arrayOfCommands = macroSequence.split(",");
			for (var i = 0; i < arrayOfCommands.length; i++)
			{
				var Command = arrayOfCommands[i].split(":");
				addCommand(Command[1],Command[2],Command[3]);
			}
			sequenceContentEl.innerHTML += temp;
			getOrder();
			toggleDisplay(1);
    		break;
    	case "End":
    		var arrayOfCommands = macroSequence.split(",");
			for (var i = 0; i < arrayOfCommands.length; i++)
			{
				var Command = arrayOfCommands[i].split(":");
				addCommand(Command[1],Command[2],Command[3]);
			}
			toggleDisplay(1);
    		break;
    	case "Export":
    		DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=exportMacro&MacroName="
    							+macroName+"&MacroSequence="+macroSequence,"",exportMacroHandlerFunction);
    		break;
    	}
    }
    
    function exportMacroHandlerFunction(req)
   	{
   		Debug.log("exportMacroHandlerFunction() was called. Req: " + req.responseText);
   	}
       
    function editCommands(textarea, seqID, index)
    {
    	var x = arrayOfCommandsForEdit[seqID].split(":");
    	x[index] = textarea.value;
    	arrayOfCommandsForEdit[seqID] = x.join(":");
    	console.log(seqID);
    }
    
    function deleteMacroHandlerFunction(req)
	{
		Debug.log("deleteMacroHandlerFunction() was called. Req: " + req.responseText);
		var deletedMacroName = DesktopContent.getXMLValue(req,"deletedMacroName");
		var reminderEl = document.getElementById('reminder');
		reminderEl.innerHTML = "Successfully deleted " + deletedMacroName;
		loadExistingMacros();  
	}
    
    function saveChangedMacro()
    {
    	newMacroNameForEdit = document.getElementById("macroNameEdit").value;
    	var Regex = /^[a-z0-9]+$/i;
		if (!Regex.test(newMacroNameForEdit)) alert("Sorry, special characters and spaces are not allowed.");
		else
		{
    	macroNotesForEdit = document.getElementById('macroNotesEdit').value;
    	DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=editMacro&oldMacroName="
    					+oldMacroNameForEdit+"&newMacroName="+newMacroNameForEdit+"&Sequence="
						+arrayOfCommandsForEdit+"&Time="+macroDateForEdit+"&Notes="
						+macroNotesForEdit,"",saveChangedMacroHandlerFunction);
    	hidePopupEditMacro();
		}
    }
    
    function saveChangedMacroHandlerFunction()
    {
    	Debug.log("saveChangedMacroHandlerFunction() was called.");
//		var newMacroName = DesktopContent.getXMLValue(req,"newMacroName");
//		var reminderEl = document.getElementById('reminder');
//		if (newMacroName != "ERROR")
//			reminderEl.innerHTML = "Renamed " + macroNameForRename + " to " + newMacroName;
//		else 	
//			reminderEl.innerHTML = "Error renaming " + macroNameForRename + "!";
		loadExistingMacros();  
    }
    
    function reloadMacroSequence()
	{
		var sequenceContentEl = document.getElementById("sequenceContent");
		sequenceContentEl.innerHTML = "";
		macroString = [];
    	SEQFORMAT = document.getElementById("sequenceFormat").value;
    	var macroStringForReload = tempString.slice();
    	for (var i = 0; i < macroStringForReload.length; i++)
    	{
			var Command = macroStringForReload[i].split(":");
			console.log(Command);
			addCommand(Command[1],Command[2],Command[3]);
    	}
    }

    function setFieldToVariable(div, seqID, index)
    {
    	var popupNameVariableEl = document.getElementById("popupNameVariable");
    	popupNameVariableEl.style.display = "block";
    	var nameVariablePromptEl = document.getElementById("nameVariablePrompt");
    	var textareaEl = div.previousSibling;
		document.getElementById('popupNameVariableCancelButton').onclick = function() {
			popupNameVariableEl.style.display = "none";
			document.getElementById("nameVariable").value = "";
			return;
		};
    	if(isNaN(textareaEl.value))
    	{
    		nameVariablePromptEl.innerHTML = "Would you like a set value instead of a variable?";
    		document.getElementById('popupNameVariableSaveButton').onclick = function() {
				var variableName = document.getElementById("nameVariable").value.toString();
				if(isNaN("0x"+variableName))
				{
					nameVariablePromptEl.innerHTML = "<span class='red'>The value has to be a hex number.</span>";
					return;
				}
				div.style.backgroundColor = "#002a52";
				textareaEl.value = variableName;
				textareaEl.disabled = false;
				var x = arrayOfCommandsForEdit[seqID].split(":");
				x[index] = variableName;
				arrayOfCommandsForEdit[seqID] = x.join(":");
				console.log(arrayOfCommandsForEdit);
				document.getElementById("nameVariable").value = "";
				popupNameVariableEl.style.display = "none";
			};
    	}
    	else
    	{
			nameVariablePromptEl.innerHTML = "Setting field to variable! How would you like to name it?";
			document.getElementById('popupNameVariableSaveButton').onclick = function() {
				var variableName = document.getElementById("nameVariable").value.toString();
				if(!isNaN("0x"+variableName))
				{
					nameVariablePromptEl.innerHTML = "<span class='red'>Name of the variable cannot be a number.</span>";
					return;
				}
				div.style.backgroundColor = "#ff0101";
				textareaEl.value = variableName;
				textareaEl.disabled = true;
				var x = arrayOfCommandsForEdit[seqID].split(":");
				x[index] = variableName;
				arrayOfCommandsForEdit[seqID] = x.join(":");
				console.log(arrayOfCommandsForEdit);
				document.getElementById("nameVariable").value = "";
				popupNameVariableEl.style.display = "none";
			};
    	}
    }
    

    function dealWithVariables(stringOfCommands,macroName)
    {
    	var reminderEl = document.getElementById('reminder');
    	var waitForUserInput = 0;
    	var copyOfStringOfCommands = stringOfCommands.slice();
    	var i = 0;
    	var toChange = 0;
    	var newCommand = [];
    	var dictionary = {};
    	if(isMacroRunning)
    		reminderEl.innerHTML = "Please wait till the current macro ends";
    	else if(isArrayAllZero(selected))
    		reminderEl.innerHTML = "Please select at least one interface from the list";
    	else
    	{
    		var askEl = document.getElementById('popupAskForVariableValue');
    		timeIntervalID = setInterval(function(){
    			if(i < stringOfCommands.length && waitForUserInput === 0)
    			{
    				var Command = stringOfCommands[i].split(":");
    				if (dictionary[Command[2].toString()] !== undefined)
    				{
        				newCommand = stringOfCommands[i].split(":");
    					newCommand[2] = dictionary[Command[2].toString()];
    					copyOfStringOfCommands[i] = newCommand.join(":");
    				}
    				else if (isNaN("0x"+Command[2]))
    				{
    					waitForUserInput = 1;
        				newCommand = stringOfCommands[i].split(":");
    					var variableNameAtRunTime = Command[2];
    					toChange = i;
    					if(waitForUserInput === 0)
    						return;
    					else
    					{
    						askEl.style.display = "block";
    						document.getElementById('variableNameAtRunTime').innerHTML = variableNameAtRunTime;
    					}
    				}
    				i++;
    			}
    			else if(i == stringOfCommands.length && waitForUserInput === 0)
    			{
    				clearInterval(timeIntervalID);
    				console.log("Final command to send to run: " + copyOfStringOfCommands);
    				runMacro(copyOfStringOfCommands, macroName);
    			}
    		},200);
    	}

    	document.getElementById('popupAskForVariableValueContinue').onclick = function() {
    		var variableValue = document.getElementById("valueAtRunTime").value.toString();
    		if(isNaN("0x"+variableValue))
    		{
    			getElementById("assignValuePrompt").innerHTML = "<span class='red'>The value has to be a hex number.</span>";
    			return;
    		}
    		else
    		{
     			dictionary[newCommand[2].toString()] = variableValue;
    			newCommand[2] = variableValue;
    			askEl.style.display = "none";
    			copyOfStringOfCommands[toChange] = newCommand.join(":");
    			waitForUserInput = 0;
    			document.getElementById("valueAtRunTime").value = "";
    			console.log(copyOfStringOfCommands);
    			return;
    		}
    	};
    }