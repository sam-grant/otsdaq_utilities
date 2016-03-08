// Created by swu at fnal dot gov
//  February 2016
//
//



	//Function List:
		//init
	    //redrawWindow
		//callWrite
		//callRead



	//Handling window resizing
	function init() 
	{			
			Debug.log("init() was called");
			DesktopContent.XMLHttpRequest("MacroMaker?RequestType=FEWlist","",FEClistHandlerFunction);
			block1El = document.getElementById('fecList');//red
			block2El = document.getElementById('macroLib');//yellow
			block3El = document.getElementById('main');//blue
			block4El = document.getElementById('instruction');//green
			block5El = document.getElementById('history');//green
			historybox = document.getElementById('historyContent');

			window.onresize = redrawWindow;
			redrawWindow(); //redraw window for first time
			
			//
	}
	
	function redrawWindow() 
	{
		Debug.log("Chat redrawChat to " + window.innerWidth + " - " + window.innerHeight);
						
		var w = window.innerWidth;
		var h = window.innerHeight;
		
		
		//square [x,y] [w,h]
		var _MARGIN = 10;
		
		var b1 = [_MARGIN, _MARGIN, w/3-_MARGIN, h/2-_MARGIN]; //left column red
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
			 
	function FEClistHandlerFunction(req) 
	{
		Debug.log("FEClistHandlerFunction() was called. Req: " + req.responseText);
		var FECElements = req.responseXML.getElementsByTagName("FEW");
		
		//Make search box for the list
		var noMultiSelect = false; 									
				
	    var keys = [];
	    var vals = [];
	    var types = [];

	    for(var i=0;i<FECElements.length;++i)
		{
			keys[i] = "one";
			vals[i] = FECElements[i].getAttribute("value");
			types[i] = "number";
			
			Debug.log(vals[i]);
		}
	    var listoffecs = document.getElementById('list');  
		MultiSelectBox.createSelectBox(listoffecs,
				"box1",
				"List of Available FECs",
				vals,keys,types,"listSelectionHandler",noMultiSelect);
	    			            
	    //End of making box
	}

	function listSelectionHandler(listoffecs)
	{
	 	 var splits = listoffecs.id.split('_');
		 var i = splits[splits.length-1] | 0;
		 MultiSelectBox.dbg("Chosen element index:",i);
	}
	        
            
            
    function clearData(){
         	 document.getElementById('dataInput').value = "";
    }
    
    function clearAddress()
    {
           	 document.getElementById('addressInput').value = "";
    }

    function callWrite()
    {
		 var reminderEl = document.getElementById('reminder');

			//if(document.getElementById("FECcheck").checked||true)//CHANGE THIS
			//{
				 var addressFormat = document.getElementById("addressFormat");
			     var addressFormatIndex = addressFormat.options[addressFormat.selectedIndex].value;
			     var dataFormat = document.getElementById("dataFormat");
			     var dataFormatIndex = dataFormat.options[dataFormat.selectedIndex].value;
			     
				 var contentEl = document.getElementById('historyContent');
			     var addressStr = document.getElementById('addressInput').value;
				 var dataStr = document.getElementById('dataInput').value;
				 DesktopContent.XMLHttpRequest("MacroMaker?RequestType=writeData&Address="+addressStr+"&addressFormat="+addressFormatIndex+"&dataFormat="+dataFormatIndex+"&Data="+dataStr,"",writeHandlerFunction);
//var time = Date().toString();
				 var update = "Write ".concat(dataStr," into ",addressStr,"<br\>");
				 var newContent = contentEl.innerHTML.concat(update);
				 contentEl.innerHTML = newContent;
				 updateScroll()
				 reminderEl.innerHTML = "Data written."
			//}
			//else
			//	 reminderEl.innerHTML = "Please select a FEC from the list."
    }

    function callRead()
    {
		 var reminderEl = document.getElementById('reminder');

		//	if(document.getElementById("FECcheck").checked)
			//{
				var addressFormat = document.getElementById("addressFormat");
				var addressFormatIndex = addressFormat.options[addressFormat.selectedIndex].value;
				var dataFormat = document.getElementById("dataFormat");
				var dataFormatIndex = dataFormat.options[dataFormat.selectedIndex].value;
			
				var addressStr = document.getElementById('addressInput').value;
	            var dataStr = document.getElementById('dataInput').value;
	           
	            DesktopContent.XMLHttpRequest("MacroMaker?RequestType=readData&Address="+addressStr+"&addressFormat="+addressFormatIndex+"&dataFormat="+dataFormatIndex,"",readHandlerFunction);
	       	//}
			//else
				// reminderEl.innerHTML = "Please select a FEC from the list."
    }
    
    function writeHandlerFunction(req)
	{
		Debug.log("writeHandlerFunction() was called. Req: " + req.responseText);
    }
    
    function readHandlerFunction(req)
	{
		Debug.log("readHandlerFunction() was called. Req: " + req.responseText);
		var dataOutput = DesktopContent.getXMLValue(req,"readData");
		var reminderEl = document.getElementById('reminder');
		
		var contentEl = document.getElementById('historyContent');
	    var addressStr = document.getElementById('addressInput').value;
		//var time = Date().toString();
		var update = "Read ".concat(dataOutput," from ",addressStr,"<br\>");
		var newContent = contentEl.innerHTML.concat(update);
		contentEl.innerHTML = newContent;
		updateScroll()
		reminderEl.innerHTML = ("Data read from address ").concat(addressStr," is ",dataOutput);
	}
    
    function updateScroll(){
        var element = document.getElementById("historyContent");
        element.scrollTop = element.scrollHeight;
    }

//    function enterAddress(){
//    			 var text_box = document.getElementById('addressInput');
//    		     var results_box = document.getElementById('output1');
//    			 var text = text_box.value;
//    			 var message = "Address logged in";
//    			 results_box.innerHTML = message;
//    		}     
//        
//	function enterData(){
//			 var text_box = document.getElementById('dataInput');
//			 var results_box = document.getElementById('output2');
//			 var text = text_box.value;
//			 var message = " Data logged in";
//			 results_box.innerHTML = message;