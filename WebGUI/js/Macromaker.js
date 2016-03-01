// Created by swu at fnal dot gov
//  February 2016
//
//



	//Function List:
		//enterAddress
		//enterData
		//callWrite



	//Handling window resizing
	function init() {			
			 Debug.log("init() was called");
			 DesktopContent.XMLHttpRequest("MacroMaker?RequestType=FEClist","",handlerFunction);
			block1El = document.getElementById('fecList');//red
			block2El = document.getElementById('macroLib');//yellow
			block3El = document.getElementById('main');//blue
			block4El = document.getElementById('instruction');//green
			block5El = document.getElementById('history');//green
			historybox = document.getElementById('historyContent');

			window.onresize = redrawWindow;
			redrawWindow(); //redraw window for first time
	}
	
	function redrawWindow() {
				Debug.log("Chat redrawChat to " + window.innerWidth + " - " + window.innerHeight);
								
				var w = window.innerWidth;
				var h = window.innerHeight;
				
				
				//square [x,y] [w,h]
				var _MARGIN = 10;
				
				var b1 = [_MARGIN, _MARGIN, w/4, h/2-_MARGIN]; //left column red
				var b2 = [_MARGIN, h/2, w/4, h/2-_MARGIN]; //left column red
				var b3 = [_MARGIN+w/4, _MARGIN, w/2-2*_MARGIN, h/2-_MARGIN]; //top middle yellow
				var b4 = [_MARGIN+w/4, h/2, w/2-2*_MARGIN, h/2-_MARGIN]; //bottom middle blue
				var b5 = [w-_MARGIN-w/4,_MARGIN,w/4, h-2*_MARGIN]; //right column green
				
				
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
			 
	 function handlerFunction(req) {
			 Debug.log("handlerFunction() was called. Req: " + req.responseText);

			 var child1data = DesktopContent.getXMLValue(req,"child");
		 	 var child2data = DesktopContent.getXMLValue(req,"child2");
			
		   	 Debug.log("--child1data:",child1data," --child2data:",child2data);
		   	 var results_box = document.getElementById('output2');
		     results_box.innerHTML = child1data;
			
		}
			 
	function enterAddress(){
			 var text_box = document.getElementById('addressInput');
		     var results_box = document.getElementById('output1');
			 var text = text_box.value;
			 var message = "Address logged in";
			 results_box.innerHTML = message;
		}     
    //test
    function enterData(){
             var text_box = document.getElementById('dataInput');
             var results_box = document.getElementById('output2');
             var text = text_box.value;
             var message = " Data logged in";
             results_box.innerHTML = message;
            }      

    function callWrite(){
        //call write() in .cc, 
       var contentEl = document.getElementById('historyContent');
       var addressStr = document.getElementById('addressInput').value;
       var dataStr = document.getElementById('dataInput').value;
        var now = Date();
        var time = now.toString();
        var str = " was written into address ";
        var update = time.concat(" Data ",dataStr,str,addressStr,"<br\>");
        var newContent = contentEl.innerHTML.concat(update);
        contentEl.innerHTML = newContent;
    }

    function callRead(){
        //call read() in .cc, 
        var contentEl = document.getElementById('historyContent');
        var addressStr = document.getElementById('addressInput').value;
        var dataStr = "data";//will be data from read()
        var now = Date();
        var time = now.toString();
        var update = time.concat(" Data ",dataStr," was read from address ",addressStr,"<br\>");
        var newContent = contentEl.innerHTML.concat(update);
        contentEl.innerHTML = newContent;
    }