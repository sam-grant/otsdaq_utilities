var selectedSecurityChoice_ = "";
var IDs_ = ['NoSecurity', 'DigestAccessAuthentication', 'ResetSecurityUserData'];//'Kerberos','ResetSecurityUserData'];
var selection_ = [];
INTERVAL_ = 10;
state_ = document.getElementById("state");
//////////////////////////
nonSelectedRed = 2;
nonSelectedGreen = 2;
nonSelectedBlue = 2;
//////////////////////////
var nonSelectedTargetRed;
var nonSelectedTargetGreen;
var nonSelectedtargetBlue;
//////////////////////////
selectedRed = 2;
selectedGreen = 2;
selectedBlue = 2;
//////////////////////////
var selectedTargetRed;
var selectedTargetGreen;
var selectedtargetBlue;
//////////////////////////










function init(){
	console.log("init()");


	for(var index = 0; index < IDs_.length; index++)
			selection_[index] = IDs_[index];
	//Added all of the IDs into the selection_ array and position the selection at the end
	
	colorFade('orange', "all");
	state_= document.getElementById("state");
	state_.innerHTML = "Loading...";

	var setSelection=setTimeout(makeServerRequest, 4000);
}


function colorFade(secondColor, which){


	//Added all of the IDs into the selection_ array and position the selection at the end
	
	if(which == "selected" || which == "all"){

		if(secondColor == 'orange'){
			selectedTargetRed = 255;
			selectedTargetGreen = 188;
			selectedTargetBlue = 18;				
		}else if(secondColor == 'red'){
			selectedTargetRed = 242;
			selectedTargetGreen = 5;
			selectedTargetBlue = 5;					
		}else if(secondColor == 'green'){
			selectedTargetRed = 56;
			selectedTargetGreen = 224;
			selectedTargetBlue = 56;					
		}else if(secondColor == 'current'){
			
		}
		
	}
	if(which == "notSelected" || which == "all"){
	
		if(secondColor == 'orange'){
			nonSelectedTargetRed = 255;
			nonSelectedTargetGreen = 188;
			nonSelectedTargetBlue = 18;				
		}else if(secondColor == 'red'){
			nonSelectedTargetRed = 242;
			nonSelectedTargetGreen = 5;
			nonSelectedTargetBlue = 5;					
		}else if(secondColor == 'green'){
			nonSelectedTargetRed = 56;
			nonSelectedTargetGreen = 224;
			nonSelectedTargetBlue = 56;					
		}else if(secondColor == 'current'){
			
		}
	}    
	
	console.log(selectedRed, selectedGreen, selectedBlue, nonSelectedRed, nonSelectedGreen, nonSelectedBlue);
	
	var colorChanger=setTimeout(decreaseColor, INTERVAL_);


}

function decreaseColor(){
	if(selectedRed > 0)
		--selectedRed;
	if(selectedGreen > 0)
		--selectedGreen;
	if(selectedBlue > 0)
		--selectedBlue;
	if(nonSelectedRed > 0)
		--nonSelectedRed;
	if(nonSelectedGreen > 0)
		--nonSelectedGreen;
	if(nonSelectedBlue > 0)
		--nonSelectedBlue;
	console.log("Called");
	setDivGlow();
	
	if((selectedRed + selectedGreen + selectedBlue + nonSelectedRed + nonSelectedGreen + nonSelectedBlue) > 0)
		colorChanger = setTimeout(decreaseColor, INTERVAL_);
	else{
		restructure();
		colorChanger = setTimeout(increaseColor, INTERVAL_);

	}
}

function increaseColor(){
	if(selectedRed < selectedTargetRed)
		++selectedRed;
	if(selectedGreen < selectedTargetGreen)
		++selectedGreen;
	if(selectedBlue < selectedTargetBlue)
		++selectedBlue;
	if(nonSelectedRed < nonSelectedTargetRed)
		++nonSelectedRed;
	if(nonSelectedGreen < nonSelectedTargetGreen)
		++nonSelectedGreen;
	if(nonSelectedBlue < nonSelectedTargetBlue)
		++nonSelectedBlue;	
	setDivGlow();
	
	if((selectedRed + selectedGreen + selectedBlue + nonSelectedRed + nonSelectedGreen + nonSelectedBlue) < (selectedTargetRed + selectedTargetGreen + selectedTargetBlue + nonSelectedTargetRed + nonSelectedTargetGreen + nonSelectedTargetBlue))
		colorChanger = setTimeout(increaseColor, INTERVAL_);
	
}

function setDivGlow(){
	
	
	for(var index = 0; index < (selection_.length); index++)
	{
		var div = document.getElementById(selection_[index]);
		if(index < (selection_.length-1)){
			div.style.boxShadow = "0px 0px 64px 20px rgba(" + nonSelectedRed + "," + nonSelectedGreen + "," + nonSelectedBlue + ",1)";
			div.style.webkitBoxShadow = "0px 0px 64px 20px rgba(" + nonSelectedRed + "," + nonSelectedGreen + "," + nonSelectedBlue + ",1)";
			div.style.mozBoxShadow = "0px 0px 64px 20px rgba(" + nonSelectedRed + "," + nonSelectedGreen + "," + nonSelectedBlue + ",1)";
		}else{
			div.style.boxShadow = "0px 0px 64px 20px rgba(" + selectedRed + "," + selectedGreen + "," + selectedBlue + ",1)";
			div.style.webkitBoxShadow = "0px 0px 64px 20px rgba(" + selectedRed + "," + selectedGreen + "," + selectedBlue + ",1)";
			div.style.mozBoxShadow = "0px 0px 64px 20px rgba(" + selectedRed + "," + selectedGreen + "," + selectedBlue + ",1)";	
		}
	}
	
}


function showVisualSelection(){

	
	state_.innerHTML = "&nbsp";

	//set selected to green
	colorFade('green', 'selected');
	//set selected to green
	colorFade('red', 'notSelected');

}

function confirm(){
	if(prompt("Are you sure? This will delete all use data. Type 15091420 to continue")==15091420)
		if(confirm("I know exactly what I'm doing.")){
			setSecurity('ResetSecurityUserData');
		}
		else
			alert("Mission aborted!");

	else
		alert("Mission aborted!");
}

function setSecurity(id){
	
	if(id == 'NoSecurity'){
		console.log("No Security selected.")
		selectedSecurityChoice_ = 'NoSecurity';
	}else if(id == 'DigestAccessAuthentication'){
		console.log("Digest Access Authentication selcted.")
		selectedSecurityChoice_= 'DigestAccessAuthentication';
	}else if(id == 'Kerberos'){
		console.log("Kerberos selected.")
		selectedSecurityChoice_= 'Kerberos';
	}else if(id == 'ResetSecurityUserData'){
		console.log("Reset Data selected.")
		selectedSecurityChoice_= 'ResetSecurityUserData';
	}else{
		console.log("Selection not recognized!");
		return;
	}
	
	
	var serverRequest = "selection=" + selectedSecurityChoice_;
	console.log(serverRequest);
	makeServerRequest(serverRequest);
}

function makeServerRequest(data){
	
	DesktopContent.XMLHttpRequest("editSecurity", data, editSecurityHandler, undefined, undefined, DesktopWizardContent.getSequence());
}

function restructure(){

	for(var index = 0; index < IDs_.length; index++)
	{
		if(selectedSecurityChoice_ != "")
		{
			if(selectedSecurityChoice_ != IDs_[index])
				selection_[index] = IDs_[index];	
		}	
	}
	if(selectedSecurityChoice_ != "")
		selection_[selection_.length] = selectedSecurityChoice_;
	
}

var editSecurityHandler = function(req){
	
	
	
	selectedSecurityChoice_ = req.responseText; 
	console.log(selectedSecurityChoice_);
	showVisualSelection();
	return;
}