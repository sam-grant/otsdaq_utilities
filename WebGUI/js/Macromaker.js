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
	var stringOfAllMacros = [];
	
	var theAddressStrForRead = ""; // for callread and its handler
	var isOnMacroMakerPage = false;
	var EVENTCOUNTER = 0;
	var timeIntervalID;
	var isMacroRunning = false;
	
	var arrayOfCommandsForEdit = [];
	var oldMacroNameForEdit = "";
	var newMacroNameForEdit = "";
	var macroDateForEdit = "";
	var macroNotesForEdit = "";
	
	var macroNameForRename = "";
	
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
		historybox = document.getElementById('historyContent');
		sequencebox = document.getElementById('sequenceContent');
		macrobox = document.getElementById('listOfMacros');
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
		
		historybox.style.height =  h*0.88 + "px";
		sequencebox.style.height =  h*0.88 + "px";
		macrobox.style.height =  h*0.40 + "px";
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
    	var reminderEl = document.getElementById('reminder');
    	if(isArrayAllZero(selected))
    		reminderEl.innerHTML = "Please select at least one interface from the list";
    	else 
    	{ 
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
    	}
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
		console.log("Hello everyone. We are in read handler and theAddressStrForRead is "+theAddressStrForRead);

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
            macroAddressInputEl.setAttribute("placeholder", "0x64");
            break;
          case "dec":
            document.getElementById("lsbFirst").checked = false;
            document.getElementById("lsbFirst").disabled = true;
            macroAddressInputEl.setAttribute("placeholder", "1234");
            break;
          case "ascii":
            document.getElementById("lsbFirst").checked = false;
            document.getElementById("lsbFirst").disabled = false;
            macroAddressInputEl.setAttribute("placeholder", "Hi");
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
		switch(command)
		{
		case 'write':
	    	if (typeof address === 'undefined') 
			{ 
				var addressStr = document.getElementById('macroAddressInput').value.toString();
				var dataStr = document.getElementById('macroDataInput').value.toString();
	    		if(addressStr === "") 
				{
					macroReminderEl.innerHTML = "Please enter an address to write to";
					return;
				} else if(dataStr === "") {
					macroReminderEl.innerHTML = "Please enter your data";
					return;
				}
			} else {
				var addressStr = address.toString();
				var dataStr = data.toString();
			}
				var update = "<div id = \"seq" + SEQINDEX + "\" class=\"seqDiv\" onclick=\"removeCommand(" + SEQINDEX + ")\">Write <b>"
						+ dataStr + "</b> into <b>" 
						+ addressStr + "</b></div>";
				var writeMacroString = SEQINDEX + ":w:" + addressStr + ":" + dataStr;
				macroString.push(writeMacroString);
			break;
		case 'read':
			if (typeof address === 'undefined') 
			{ 
				var addressStr = document.getElementById('macroAddressInput').value.toString();
				if(addressStr === "") 
				{
					macroReminderEl.innerHTML = "Please enter an address to read from";
					return;
				}
			} else 
			var addressStr = address.toString();
			var update = "<div id = \"seq" + SEQINDEX + "\" class=\"seqDiv\" onclick=\"removeCommand(" + SEQINDEX + ")\">Read from <b>"
					+ addressStr + "</b></div>";
			var readMacroString = SEQINDEX+":r:"+addressStr;
			macroString.push(readMacroString);
			break;
		case 'delay':
			var delayStr = document.getElementById('delayInput').value;
			if(delayStr === "") 
			{
				macroReminderEl.innerHTML = "Please enter a delay";
				return;
			} else {
			var update = "<div id = \"seq" + SEQINDEX + "\" class=\"seqDiv\" onclick=\"removeCommand(" + SEQINDEX + ")\">Delay: <b>"
					+ delayStr + "</b> ms</div>";
			var delayMacroString = SEQINDEX+":d:"+delayStr;
			macroString.push(delayMacroString);
			break;
			}
		default: 
			Debug.log("So if it's not write, read, or delay, what is it??");
		}
		contentEl.innerHTML += update;
		SEQINDEX++;
		contentEl.scrollTop = contentEl.scrollHeight;
		Sortable.create(contentEl,{
				chosenClass: 'chosenClassInSequence',
				ghostClass:'ghostClassInSequence'
		});//Works like magic!
    }
   
   
    
    function removeCommand(seqIndex)
    {
    	var child = document.getElementById("seq"+seqIndex);
		var parent = document.getElementById('sequenceContent');
		parent.removeChild(child);
		for (var i = 0; i < macroString.length; i++)
		  {
		    if (seqIndex == macroString[i].split(":")[0])
		      macroString.splice(i,1);
		  }
    }
    
    function clearAll()
    {
    	var contentEl = document.getElementById('sequenceContent');
    	contentEl.innerHTML = "<b>Click to delete unwanted commands.</b><br>";
    	macroString = [];
    }
    
    function saveMacro()
    {
    	 var popupSaveMacro = document.getElementById("popupSaveMacro");
    	 popupSaveMacro.style.display = "block";
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
    
    function hidePopupRenameMacro()
    {
    	var popupRenameMacro = document.getElementById("popupRenameMacro");
    	popupRenameMacro.style.display = "none";
    	document.getElementById("RenameMacro").value="";
    }
    
    function saveAsMacro()
    {
    	var macroName = document.getElementById("macroName").value;
    	var macroNotes = document.getElementById("macroNotes").value;
    	var macroLibEl = document.getElementById('listOfMacros');
    	stringOfAllMacros[MACROINDEX] = macroString;
    	console.log(stringOfAllMacros);
    	DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=createMacro&Name="+macroName+
    				"&Sequence="+macroString+"&Time="+Date().toString()+"&Notes="
					+macroNotes,"",createMacroHandlerFunction);
    	loadExistingMacros();
    	hidePopupSaveMacro();
    	macroLibEl.scrollTop = macroLibEl.scrollHeight - macroLibEl.clientHeight;
    }
    
    function createMacroHandlerFunction(req)
	{
		Debug.log("createMacroHandlerFunction() was called. Req: " + req.responseText);
	}
    
    function runMacro(stringOfCommands,macroName)
    {
    	console.log(macroName);
    	var reminderEl = document.getElementById('reminder');
    	if(isMacroRunning)
    	    reminderEl.innerHTML = "Please wait till the current macro ends";
    	else if(isArrayAllZero(selected))
    	    reminderEl.innerHTML = "Please select at least one interface from the list";
    	else
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
    	if (hugeStringOfMacros.length > 0)
    	{
			var macrosArray = hugeStringOfMacros.split("@");
			var out = "";
			for(var i = 0; i < macrosArray.length; i++) 
			{
				var arr = JSON.parse(macrosArray[i]);
				console.log(arr);
				var macroString = arr.sequence.split(",");
				stringOfAllMacros[MACROINDEX] = macroString;
				out += "<div title='Sequence: " + arr.sequence + "\nNotes: "
						+ arr.notes + "\nCreated: " + arr.time
						+ "\' class='macroDiv' data-id=\"" + arr.name + "\" data-sequence=\"" 
						+ macroString + "\" data-notes=\"" 
						+ arr.notes + "\" data-time=\"" 
						+ arr.time + "\" onclick='runMacro(stringOfAllMacros[" 
						+ MACROINDEX + "],\"" + arr.name + "\")'><b>" + arr.name + "</b></br></div>"; 
				MACROINDEX++;
			}
			document.getElementById("listOfMacros").innerHTML = out;
    	}
    	else document.getElementById("listOfMacros").innerHTML = "";
    }
    
    function loadingHistHandlerFunction(req)
    {
    	Debug.log("loadingHistHandlerFunction() was called. Req: " + req.responseText);
		var hugeStringOfHistory = DesktopContent.getXMLValue(req,"returnHistStr");
		var contentEl = document.getElementById('historyContent');
		if (typeof hugeStringOfHistory === undefined) return;
		else if (hugeStringOfHistory.length > 0)
		{
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
				if(commandType=='w'){
					out += "<div " + innerClass + " id = \"" + CMDHISTDIVINDEX + "\"  title=\"" + "Entered: " 
							+ arr.Time + "\nSelected interface: " + arr.Interfaces
							+ "\" onclick=\"histCmdWriteDivOnclick(" + "'" + convertedAddress + "','" + convertedData + "','" 
							+ addressFormat + "','" + dataFormat + "')\">Write [" + dataFormat + "]<b>"
							+ convertedData + "</b> into register [" + addressFormat + "]<b> " 
							+ convertedAddress + "</b></div>";
					CMDHISTDIVINDEX++;
				}else if(commandType=='r'){
					if (Number(convertedData)===0) convertedData = "<span class='red'>Time out Error</span>";
					out += "<div " + innerClass + " id = \"" + CMDHISTDIVINDEX + "\" title=\"" + "Entered: " 
							+ arr.Time + "\nSelected interface: " + arr.Interfaces + "\" onclick=\"histCmdReadDivOnclick(" 
							+ "'" + convertedAddress + "','" + addressFormat + "'" + ")\">Read [" + dataFormat + "]<b>" 
							+ convertedData + "</b> from register [" + addressFormat + "]<b>" + convertedAddress + "</b></div>";
					CMDHISTDIVINDEX++;
				}else if(commandType=='d')
					console.log("delay "+oneCommand[1]+"ms");
				else
					console.log("ERROR! Command type "+commandType+" not found");

			}
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
    		addCommand("write",convertedAddress,convertedData);
    	}
    	else callWrite(addressStr, dataStr);
    }
    
    function histCmdReadDivOnclick(addressStr, addressFormatStr)
	{
    	var convertedAddress = reverseLSB(convertToHex(addressFormatStr,addressStr));
		if(isOnMacroMakerPage)
		{
			addCommand("read",convertedAddress)
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
				if(commandType=='w'){
					var writeEdit = "<div>Write <textarea cols='10' rows='1' onchange=editCommands(this," + seqID + ",3)>" + Command[3]
						+ "</textarea> into <textarea cols='10' rows='1' onchange=editCommands(this," + seqID + ",2)>" + Command[2] + "</textarea></div>";
					seqID++;
					output += writeEdit;
				}else if(commandType=='r'){
					var readEdit = "<div>Read from <textarea cols='10' rows='1' onchange=editCommands(this," + seqID + ",2)>" + Command[2]
						+ "</textarea></div>";
					seqID++;
					output += readEdit;
				}else if(commandType=='d'){
					var delayEdit = "<div>Delay <textarea cols='10' rows='1' onchange=editCommands(this," + seqID + ",2)>" + Command[2]
						+ "</textarea> seconds</div>";
					seqID++;
				    output += delayEdit;
				}else
					console.log("ERROR! Command type "+commandType+" not found");
			}
    		macroSequenceEditEl.innerHTML = output;
    	
    		var macroNameEl = document.getElementById("macroNameEdit");
    		macroNameEl.value = macroName;
    		var macroNotesEl = document.getElementById("macroNotesEdit");
    		macroNotesForEdit = "[Modified on " + (new Date()).toLocaleDateString() + "] " + macroNotes;
    		macroNotesEl.value = macroNotesForEdit;
    		break;
    	case "Rename":
    		var popupRenameMacro = document.getElementById("popupRenameMacro");
    		popupRenameMacro.style.display = "block";
    		macroNameForRename = macroName;
			break;
    	}
    }
    
    function editCommands(textarea, seqID, index)
    {
    	var x = arrayOfCommandsForEdit[seqID].split(":");
    	x[index] = textarea.value;
    	arrayOfCommandsForEdit[seqID] = x.join(":");
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
    	macroNotesForEdit = document.getElementById('macroNotesEdit').value;
    	DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=editMacro&oldMacroName="
    					+oldMacroNameForEdit+"&newMacroName="+newMacroNameForEdit+"&Sequence="
						+arrayOfCommandsForEdit+"&Time="+macroDateForEdit+"&Notes="
						+macroNotesForEdit,"",saveChangedMacroHandlerFunction);
    	hidePopupEditMacro();
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
    
    function renameMacro()
    {
		var newName = document.getElementById('RenameMacro').value;
		DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=renameMacro&oldMacroName="
				+macroNameForRename+"&newMacroName="+newName,"",renameMacroHandlerFunction);
    }
    
    function renameMacroHandlerFunction(req)
  	{
  		Debug.log("renameMacroHandlerFunction() was called. Req: " + req.responseText);
  		var newMacroName = DesktopContent.getXMLValue(req,"renamedMacro");
  		var reminderEl = document.getElementById('reminder');
  		if (newMacroName != "ERROR")
  			reminderEl.innerHTML = "Renamed " + macroNameForRename + " to " + newMacroName;
  		else 	
  			reminderEl.innerHTML = "Error renaming " + macroNameForRename + "!";
  		hidePopupRenameMacro();
  		loadExistingMacros();  
  	}
