// Created by swu at fnal dot gov
//  February 2016
//
//3 global vars: DIVINDEX, FEELEMENTS and selected
	//Function List:
		//init
	    //redrawWindow
		//callWrite
		//callRead
	var DIVINDEX = 0;
	var FEELEMENTS = [];
	
	function init() 
	{			
		Debug.log("init() was called");
		DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=FElist","",FElistHandlerFunction);
		block1El = document.getElementById('fecList');
		block2El = document.getElementById('macroLib');
		block3El = document.getElementById('main');
		block4El = document.getElementById('instruction');
		block5El = document.getElementById('history');
		historybox = document.getElementById('historyContent');
		window.onresize = redrawWindow;
		redrawWindow(); //redraw window for the first time
	}
	
	//Handling window resizing
	function redrawWindow() 
	{
		Debug.log("Window redraw to " + window.innerWidth + " - " + window.innerHeight);
						
		var w = window.innerWidth;
		var h = window.innerHeight;
		
		//square [x,y] [w,h]
		var _MARGIN = 5;
		
		var b1 = [_MARGIN, _MARGIN, w/3, h/2-_MARGIN]; //left column red
		var b2 = [_MARGIN, h/2, w/3-_MARGIN, h/2-_MARGIN]; //left column red
		var b3 = [w/3, _MARGIN, w/3, h/2-_MARGIN]; //top middle yellow
		var b4 = [w/3, h/2, w/3, h/2-_MARGIN]; //bottom middle blue
		var b5 = [w*2/3,_MARGIN,w/3-_MARGIN, h-2*_MARGIN]; //right column green
		
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
		
		historybox.style.height =  h*0.9 + "px";
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

	    for(var i=0;i<FEELEMENTS.length;++i)
		{
			keys[i] = "one";
			vals[i] = FEELEMENTS[i].getAttribute("value");
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
    
    function clearAddress()
    {
         document.getElementById('addressInput').value = "";
    }

    function callWrite(address,data)
    {
    	var reminderEl = document.getElementById('reminder');
		if(isArrayAllZero(selected))
			reminderEl.innerHTML = "Please select at least one interface from the list";
		else 
		{ 
			var addressFormat = document.getElementById("addressFormat");
			var addressFormatIndex = addressFormat.options[addressFormat.selectedIndex].value;
			var dataFormat = document.getElementById("dataFormat");
			var dataFormatIndex = dataFormat.options[dataFormat.selectedIndex].value;
			 
			if (typeof address === 'undefined') 
			{ 
				var addressStr = document.getElementById('addressInput').value;
				var dataStr = document.getElementById('dataInput').value;
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
			if (DIVINDEX%2) innerClass = "class=\"innerClass2\"";
			DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=writeData&Address="+addressStr
					+"&addressFormat="+addressFormatIndex+"&dataFormat="+dataFormatIndex+"&Data="+dataStr
					+"&supervisorIndex="+supervisorIndexArray+"&interfaceIndex="+interfaceIndexArray,"",writeHandlerFunction);
			var update = "<div " + innerClass + " id = \"" + DIVINDEX + "\"  title=\"" + "Entered: " 
					+ Date().toString() + "\nSelected interface: " + selectionStrArray + "\" onclick=\"callWrite(" 
					+ addressStr + "," + dataStr + ")\">Write " + dataStr + " into register " + addressStr + "</div>";
			contentEl.innerHTML += update;
			DIVINDEX++;
			updateScroll();
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
			var addressFormat = document.getElementById("addressFormat");
			var addressFormatIndex = addressFormat.options[addressFormat.selectedIndex].value;
			var dataFormat = document.getElementById("dataFormat");
			var dataFormatIndex = dataFormat.options[dataFormat.selectedIndex].value;
		
			if (typeof address === 'undefined') 
			{
				var addressStr = document.getElementById('addressInput').value;
				if(addressStr == "") 
				{
					reminderEl.innerHTML = "Please enter an address to read from";
					return;
				}
			}
			else
				var addressStr = address.toString();
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
			DesktopContent.XMLHttpRequest("MacroMakerRequest?RequestType=readData&Address="+addressStr+
					"&addressFormat="+addressFormatIndex+"&dataFormat="+dataFormatIndex+"&supervisorIndex="+
					supervisorIndexArray+"&interfaceIndex="+interfaceIndexArray,"",readHandlerFunction);
    	}
    }
    
    function writeHandlerFunction(req)
	{
		Debug.log("writeHandlerFunction() was called. Req: " + req.responseText);
    }
    
    function readHandlerFunction(req,address)
	{
    	var reminderEl = document.getElementById('reminder');
		Debug.log("readHandlerFunction() was called. Req: " + req.responseText);
		var dataOutput = DesktopContent.getXMLValue(req,"readData");
		console.log(dataOutput);
		if (typeof address === 'undefined') 
		    var addressStr = document.getElementById('addressInput').value;
		else
		    var addressStr = address.toString();
		var selectionStrArray = [];
		for (var i = 0; i < selected.length; i++) 
		{
			if (selected[i]!==0) selectionStrArray.push(FEELEMENTS[i].getAttribute("value"));
		}
		var innerClass = "class=\"innerClass1\"";
		if (DIVINDEX%2) innerClass = "class=\"innerClass2\"";
		var contentEl = document.getElementById('historyContent');
		var update = "<div " + innerClass + " id = \"" + DIVINDEX + "\" title=\"" + "Entered: " + Date().toString()
				+ "\nSelected interface: " + selectionStrArray + "\" onclick=\"callRead(" 
				+ addressStr + ")\">Read " + dataOutput + " from register " + addressStr + "</div>";
		contentEl.innerHTML += update;
		DIVINDEX++; 
		updateScroll();
		reminderEl.innerHTML = "Data read: " + dataOutput;
	}
    
    function updateScroll()
    {
        var element = document.getElementById("historyContent");
        element.scrollTop = element.scrollHeight;
    }
    
    function isArrayAllZero(arr){
        for(var j = 0; j < arr.length; j++){
          if (arr[j]!==0) return false;
        }
        return true;
    }

    
