  //////////////////////////////////////////////////////////////////////////
 //////// Functions and variables to be included by other pages ///////////        
//////////////////////////////////////////////////////////////////////////
//	
// To make a Multi-Select Box
//  	create a div element and call in JavaScript...
//
//	MultiSelectBox.createSelectBox(el,name,title,vals)
//
//	This function is called by user to actually create the multi select box
// 	These parameters are optional and can be omitted or set to 0: 
//		keys, types, handler, noMultiSelect 
// 	Note: handler is the string name of the function (put in quotes).
//
// 	Can use MultiSelectBox.initMySelectBoxes after manually setting the mySelects_ array
//
//
//
// Example selection handler:
//
//        function exampleSelectionHandler(el)
//        {
//            var splits = el.id.split('_');
//            var i = splits[splits.length-1] | 0;
//            MultiSelectBox.dbg("Chosen element index:",i,
//            		" key:",el.getAttribute("key-value"),
//            		" type:",el.getAttribute("type-value"));
//            for(var s in MultiSelectBox.mySelects_[el.parentElement.id])
//                MultiSelectBox.dbg("selected: ",MultiSelectBox.mySelects_[el.parentElement.id][s]);
//            	
//        }
//
// Example usage: /WebPath/html/MultiSelectBoxTest.html
//
//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

var selected = [];
var MultiSelectBox = MultiSelectBox || {}; //define MultiSelectBox namespace

if(window.console && console && console.log)
	MultiSelectBox.dbg = console.log.bind(window.console);
//var is_chrome = navigator.userAgent.toLowerCase().indexOf('chrome') > -1;

function $(id) {return document.getElementById(id);}

/////////////////////////////////////////////////////////////////////////
//global variables

MultiSelectBox.mySelects_ = {};
MultiSelectBox.omnis_ = {}; 
MultiSelectBox.isSingleSelect_ = {}; 
MultiSelectBox.lastOptSelect_ = {};  //maintain last opt clicked

MultiSelectBox.selInitBoxHeight_ = 0; //init with first showing of search box in showSearch()
MultiSelectBox.SEL_INIT_PADDING = 5; 
/////////////////////////////////////////////////////////////////////////
//function definitions

MultiSelectBox.hasClass = function(ele,cls) 
{
    return !!ele.className.match(new RegExp('(\\s|^)'+cls+'(\\s|$)'));
}

MultiSelectBox.addClass = function(ele,cls) 
{
    if (!MultiSelectBox.hasClass(ele,cls)) ele.className += " "+cls;
}

MultiSelectBox.removeClass = function(ele,cls) 
{
    if (MultiSelectBox.hasClass(ele,cls)) 
    {
    	var reg = new RegExp('(\\s|^)'+cls+'(\\s|$)');
    	ele.className=ele.className.replace(reg,'');
    }
}

MultiSelectBox.toggleClass = function(ele,cls) 
{
	//returns true if the element had the class
	if (MultiSelectBox.hasClass(ele,cls)) 
	{
		var reg = new RegExp('(\\s|^)'+cls+'(\\s|$)');
    	ele.className=ele.className.replace(reg,'');
    	return true;
    }
    else 
    {
    	 ele.className += " "+cls;
		return false;
	}
}

MultiSelectBox.getSelectedIndex = function(el)
{
    var splits = el.id.split('_');
    return splits[splits.length-1] | 0;    
}

MultiSelectBox.getSelectionArray = function(el)
{    
	console.log(el.id);
	if(el.parentElement.id.indexOf("selbox-") == 0)
		return MultiSelectBox.mySelects_[el.parentElement.id];
	else
		return MultiSelectBox.mySelects_[el.getElementsByClassName("mySelect")[0].id];		
}

MultiSelectBox.getSelectionElementByIndex = function(el,i)
{    
	return document.getElementById(el.getElementsByClassName("mySelect")[0].id + 
			"-option_" + i);
}

MultiSelectBox.setSelectionElementByIndex = function(el,i,selected)
{    
	var name = el.getElementsByClassName("mySelect")[0].id;
	if(MultiSelectBox.isSingleSelect_[name] && 
			selected) //if true, only allow one select at a time, so deselect others
	{
		var size = MultiSelectBox.mySelects_[name].length;
		for (var opt=0; opt<size; opt++)
			MultiSelectBox.mySelects_[name][opt] = 0;
	}
	MultiSelectBox.mySelects_[name][i] = selected?1:0;
}

//for multiple selects to behave like checkboxes
MultiSelectBox.myOptionSelect = function(option, index, isSingleSelect, event)
{
	var select = option.parentElement;
	var id = select.getAttribute("id");
	var selectList = MultiSelectBox.mySelects_[id];
	var size = select.childNodes.length;
	
	if(event)
		MultiSelectBox.dbg("Shift click = " + event.shiftKey);

	//if shift.. then select or deselect 
	//	(based on value at MultiSelectBox.lastOptSelect_[id]) from
	//	MultiSelectBox.lastOptSelect_[id]
	//	to this click
	
	//MultiSelectBox.dbg(selectList);
	if (!selectList || selectList.length!=size)
	{ //first time, populate select list
		MultiSelectBox.mySelects_[id] = []; 
		MultiSelectBox.lastOptSelect_[id] = -1;
		selectList=MultiSelectBox.mySelects_[id];	
		for (var opt=0; opt<size; opt++)
			selectList.push(0);
	}

	//toggle highlighted style and global array
	MultiSelectBox.toggleClass(option,"optionhighlighted");
	selectList[index] ^= 1;
	
	if(isSingleSelect) //if true, only allow one select at a time, so deselect others
		for (var opt=0; opt<size; opt++)
        {
            //fixed, now works for any order option IDs. Goes by index only.
            var cindex = select.childNodes[opt].id.split("_");
            cindex = cindex[cindex.length-1];
        
			if(cindex == index) continue;
			else if(selectList[cindex] == 1)
			{
				MultiSelectBox.toggleClass(select.childNodes[opt],"optionhighlighted");
				selectList[cindex] = 0;
			}
        }
	else if(event.shiftKey && 
			MultiSelectBox.lastOptSelect_[id] != -1)
	{
		//if shift.. then select or deselect 
		//	(based on value at MultiSelectBox.lastOptSelect_[id]) from
		//	MultiSelectBox.lastOptSelect_[id]
		//	to this click
		
		var lo = MultiSelectBox.lastOptSelect_[id] < index? 
				MultiSelectBox.lastOptSelect_[id]:index;
		var hi = MultiSelectBox.lastOptSelect_[id] < index? 
				index:MultiSelectBox.lastOptSelect_[id];

		MultiSelectBox.dbg("lo ",lo," hi ",hi);
		//handle multi shift click
		for (var opt=lo; opt<=hi; opt++)
		{
			MultiSelectBox.dbg(selectList[opt]," vs ",
					selectList[MultiSelectBox.lastOptSelect_[id]]);
			if(selectList[opt] != 
					selectList[MultiSelectBox.lastOptSelect_[id]]) //if not matching selected value
			{
				MultiSelectBox.dbg("flip");
				//toggle highlighted style and global array
				MultiSelectBox.toggleClass(select.childNodes[opt],"optionhighlighted");
				selectList[opt] ^= 1;
			}
		}
	}

	MultiSelectBox.dbg(selectList);
	selected = selectList;
	MultiSelectBox.lastOptSelect_[id] = index; //save selection
}

//This function is called by user to actually create the multi select box
// These parameters are optional and can be omitted or set to 0: 
//		keys, types, handler, noMultiSelect 
// Note: handler is the string name of the function
MultiSelectBox.createSelectBox = function(el,name,title,vals,keys,types,handler,noMultiSelect)
{
	if(!el) 
	{ MultiSelectBox.dbg("Invalid Element given to MultiSelectBox: " + el);
		throw new Error("Invalid Element given to MultiSelectBox: " + el); return; } 
	
	el.innerHTML = ""; //delete current contents

	name = "selbox-" + name;
	MultiSelectBox.addClass(el,"multiselectbox"); //add multiselectbox class to div  
	
	MultiSelectBox.omnis_[name] = el; 
	MultiSelectBox.isSingleSelect_[name] = noMultiSelect;
	MultiSelectBox.lastOptSelect_[name] = -1; //default to nothing selected

	//searchglass=28x28, margin=5, vscroll=16, border=1
	var msW = el.offsetWidth - 28 - 5 - 16 - 2; 
	var msH = el.offsetHeight - 40 - 2; 
	
	
	el = document.createElement("div"); //create element within element
	MultiSelectBox.omnis_[name].appendChild(el);

	var str = "";
	if(title)
	{
		str += "<div id='" + name + "header' " +
				"style='margin-top:20px;width:100%'><b>"
		str += title;
		str += "</b></div>";
	}
	
	if(!keys) keys = vals;
	if(!types) types = vals;
	
	//make selbox
	str += "<div class='mySelect' unselectable='on' id='" + 
			name + "' style='float:left;" + 
			"width: " + (msW) + "px;" + 
			"height: " + (msH) + "px;" + 
			"' name='" + name + "' " +
			">";

	for (var i = 0; i < keys.length;++i)//cactus length
	{
		str += "<div  class='myOption' " +
			"id='" + name + "-option_" + i + "' " +
			"onmousedown = 'MultiSelectBox.myOptionSelect(this, " + i + "," +
			noMultiSelect + ", event); ";
		if(handler && (typeof handler) == "string") //if handler supplied as string
			str += handler + "(this);"; //user selection handler
		else if(handler) //assume it is a function
			str += handler.name + "(this);"; //user selection handler
		str += "' ";
		str += "key-value='" + keys[i] + "' type-value='" +
			types[i] + "'>";  //index, key, ids available as attributes
		str += vals[i];
		str += "</div>";
	}       	
	str += "</div>"; 
	//close selbox
	
	//append search bar
	str += MultiSelectBox.makeSearchBar(name);
	
    el.innerHTML = str;
}

//for initializing the highlights if selects are made "manually" (without clicking)
MultiSelectBox.initMySelectBoxes = function(clearPreviousSelections)
{
	var divs=document.getElementsByClassName('mySelect');
	for (var el=0; el<divs.length; el++){
		var select = divs[el];
		
		var id = select.getAttribute("id");
		var options = select.childNodes;
		MultiSelectBox.lastOptSelect_[id] = -1;
		if (!MultiSelectBox.mySelects_[id] ||
				MultiSelectBox.mySelects_[id].length > options.length)
		{//if first time drawing select box OR size was reduced
			MultiSelectBox.mySelects_[id]=[];
			for (var opt=0; opt<options.length; opt++)
			{
				MultiSelectBox.mySelects_[id].push(0);
				options[opt].setAttribute("unselectable","on");//make not selectable for ie<10
			}
		}
		else
		{ 	//if repaint: set highlighted options
			MultiSelectBox.dbg("repaint");
			
			//if more elements were added, expand the selects array
			for (var opt=MultiSelectBox.mySelects_[id].length; opt<options.length; opt++)
			{
				MultiSelectBox.mySelects_[id].push(0);
				options[opt].setAttribute("unselectable","on");//make not selectable for ie<10
			}
			
			//highlight properly according to mySelects_ array
			for (var opt=0; opt < options.length; opt++)
			{
				if (clearPreviousSelections)
					MultiSelectBox.mySelects_[id][opt] = 0; //clear
				
				if (MultiSelectBox.mySelects_[id][opt])
				{
					//MultiSelectBox.dbg(opt);
					MultiSelectBox.addClass(options[opt],"optionhighlighted");
				}
				else
					MultiSelectBox.removeClass(options[opt],"optionhighlighted");
			}
		}		
	}
}

//for searching selectboxes (works for standard "selects" and "mySelects")
MultiSelectBox.showSearch = function(boxid)
{
	var textinput=$(boxid+"search");
	
	if(!MultiSelectBox.selInitBoxHeight_)	//init if not yet defined
	{
		MultiSelectBox.selInitBoxHeight_ = $(boxid).clientHeight; //as soon as hidden is toggled H changes
	}
		 
	//RAR decided on 2/2/2017 to not show er
	//MultiSelectBox.toggleClass($(boxid+"searchErr"),"hidden");
	$(boxid+"searchErr").innerHTML = "";
	
	if (MultiSelectBox.toggleClass(textinput,"hidden")){
		$(boxid).style.height = (MultiSelectBox.selInitBoxHeight_-47) + "px";
		$(boxid).style.paddingTop = "42px";
		//$(boxid).childNodes[0].style.marginTop="42px";
		//textinput.style.left = ($(boxid).offsetLeft-8) + "px"
		textinput.focus();
		MultiSelectBox.searchSelect(boxid,textinput);
	}
	else{
		MultiSelectBox.searchSelect(boxid,null, '');
		$(boxid).style.paddingTop = MultiSelectBox.SEL_INIT_PADDING + "px";
		$(boxid).style.height = (MultiSelectBox.selInitBoxHeight_-10) + "px";
		//$(boxid).childNodes[0].style.marginTop="initial";
	}
}

MultiSelectBox.searchTimeout_ = null;

MultiSelectBox.searchSelect = function(id,el,altstr)
{
	//wait 100ms so that it does not keep constantly searching as user types only when they are done typing
	if (MultiSelectBox.searchTimeout_){
		clearTimeout(MultiSelectBox.searchTimeout_);
	}
	MultiSelectBox.searchTimeout_ = setTimeout(function(){ MultiSelectBox.performSearchSelect(id,el,altstr); }, 100);
}

MultiSelectBox.performSearchSelect = function(id,el,altstr)
{	
	var searchstr;
	if (altstr !== undefined){
		searchstr = altstr;
	}
	else{
	 	searchstr = el.value;
	}
	MultiSelectBox.searchTimeout_ = null;
	var select = $(id).childNodes;
	
	MultiSelectBox.dbg("END OF TIMEOUT FOR   : "+searchstr);

	var re; //regular expression
	$(id+"searchErr").innerHTML = "";
	try {
		re = new RegExp(searchstr,'i');
	}
	catch(err) {		//invalid regular expression
		$(id+"searchErr").innerHTML = err.message + 
				" <a href='https://regex101.com/' target='_blank'>(help)</a>";
		re = "";
	}
	
	for (var opt=0; opt < select.length; opt++)
	{
		var option = select[opt];
		
		//MultiSelectBox.dbg("opt: " + opt);
		
		if (option.tagName == 'INPUT') { continue; }
		var text = option.innerHTML;
		
		//MultiSelectBox.dbg("tagName: " + option.tagName);
		
		//first set the hidden to unhidden and unbold the bolded
		if (MultiSelectBox.hasClass(option,"hidden"))
			MultiSelectBox.removeClass(option,"hidden");
		else
			option.innerHTML = text = text.replace("<b><u>","").replace("</u></b>","");
	
		
		var index=text.search(re);
		var len=(text.match(re) || [[]])[0].length; //returns the matched string within an array or null (when null take [[]]), so we want length of element 0
		//MultiSelectBox.dbg(text+' '+index);
		
		if(index == -1)		//if searchstr not in option innerHTML
			MultiSelectBox.addClass(option,"hidden");		
		else if(len)		//make searched string bold
			option.innerHTML = text.slice(0,index) + "<b><u>" + 
				text.slice(index,index+len) + 
				"</u></b>" + text.slice(index+len);
	}
}

MultiSelectBox.makeSearchBar = function(id)
{
	var searchBox=document.createElement("input");
	var onchange='MultiSelectBox.searchSelect("' + id + '",this)';
	
	searchBox.setAttribute( "class","hidden");
	searchBox.setAttribute( 'type','text');
	searchBox.setAttribute( 'id',id + "search");
	searchBox.setAttribute( "onpaste"   , onchange);
	searchBox.setAttribute( "oncut"     , onchange);
	searchBox.setAttribute( "onkeydown" , onchange);
	searchBox.setAttribute( "onkeyup"   , onchange);
	searchBox.setAttribute( "onkeypress", onchange);
	searchBox.setAttribute( "onselect"  , onchange);
	

	var searchErrBox=document.createElement("div");
	
	searchErrBox.setAttribute( "class","hidden");
	searchErrBox.setAttribute( 'id',id + "searchErr");
	searchErrBox.style.color="red";
	searchErrBox.style.overflow="hidden";
	searchErrBox.style.height="23px";
	
	var interval;
	function addSearchBox()
	{
		var select=$(id);
		if (select)
		{
			if(!MultiSelectBox.mySelects_[id])
				MultiSelectBox.mySelects_[id] = []; //initialize to empty the selected items
			
			var selRect = select.getBoundingClientRect(),
				omniRect = MultiSelectBox.omnis_[id].getBoundingClientRect();
				//select.offsetParent.getBoundingClientRect();
			
			var offsetx = selRect.left - omniRect.left,
				offsety = selRect.top - omniRect.top;
			var margin = 5;
			
			searchBox.style.position="absolute";
			searchBox.style.top=(offsety)+"px";
			searchBox.style.left=(offsetx)+"px";
			searchBox.style.width=(selRect.width-margin*2-30)+"px";
			
			searchErrBox.style.position="absolute"; //place above title
			searchErrBox.style.top=(offsety - 37)+"px";
			searchErrBox.style.left=(offsetx + 0)+"px";
			
			MultiSelectBox.omnis_[id].appendChild(searchBox);
			
			MultiSelectBox.omnis_[id].appendChild(searchErrBox);
			
			clearInterval(interval);
		}
	}
	
	interval = setInterval( addSearchBox, 50);
	
	imgstr = "<img src='/WebPath/images/windowContentImages/multiselectbox-magnifying-glass.jpg' " +
				" style='float:left' height='28' width='28' alt='&#128269;' ";
	imgstr += "onclick = 'MultiSelectBox.showSearch(\"" + id + "\");' title='Search' class='searchimg'>";
	return imgstr;  
}



