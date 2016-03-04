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
// Note: handler is the string name of the function
//
// Example usage: WebGUI/html/MultiSelectBox.html
//
//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


var MultiSelectBox = MultiSelectBox || {}; //define MultiSelectBox namespace

if(window.console && console && console.log)
	MultiSelectBox.dbg = console.log.bind(window.console);
//var is_chrome = navigator.userAgent.toLowerCase().indexOf('chrome') > -1;

function $(id) {return document.getElementById(id);}

/////////////////////////////////////////////////////////////////////////
//global variables

MultiSelectBox.mySelects_ = {};
MultiSelectBox.omnis_ = {}; 

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


//for multiple selects to behave like checkboxes
MultiSelectBox.myOptionSelect = function(option, index, isSingleSelect)
{
	var select = option.parentElement;
	var id = select.getAttribute("id");
	var selectList = MultiSelectBox.mySelects_[id];
	var size = select.childNodes.length;
	
	//MultiSelectBox.dbg(selectList);
	if (!selectList || selectList.length!=size)
	{ //first time, populate select list
		MultiSelectBox.mySelects_[id]=[]; 
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

	MultiSelectBox.dbg(selectList);
}

//This function is called by user to actually create the multi select box
// These parameters are optional and can be omitted or set to 0: 
//		keys, types, handler, noMultiSelect 
// Note: handler is the string name of the function
MultiSelectBox.createSelectBox = function(el,name,title,vals,keys,types,handler,noMultiSelect)
{
	if(!el) return;

	name = "selbox-" + name;
	MultiSelectBox.addClass(el,"multiselectbox"); //add multiselectbox class to div  
	
	MultiSelectBox.omnis_[name] = el; 
	el = document.createElement("div"); //create element within element
	MultiSelectBox.omnis_[name].appendChild(el);

	var str = "";
	if(title)
	{
		str += "<div style='margin-top:20px;width:100%'><b>"
		str += title;
		str += "</b></div>";
	}
	
	if(!keys) keys = vals;
	if(!types) types = vals;
	
	//make selbox
	str += "<div class='mySelect' unselectable='on' id='" + 
			name + "' style='float:left' name='" + name + "' >";

	for (var i = 0; i < keys.length;++i)//cactus length
	{
		str += "<div  class='myOption' " +
			"id='" + name + "-option_" + i + "' " +
			"onmousedown = 'MultiSelectBox.myOptionSelect(this, " + i + "," +
			noMultiSelect + "); ";
		if(handler)
			str += handler + "(this);"; //user selection handler
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

MultiSelectBox.initMySelectBoxes = function()
{
	var divs=document.getElementsByTagName('div');
	for (var el=0; el<divs.length; el++){
		var select = divs[el];
		if (MultiSelectBox.hasClass(select,'mySelect'))
		{
			var id = select.getAttribute("id");
			var options = select.childNodes;
			if (!MultiSelectBox.mySelects_[id] || 
					MultiSelectBox.mySelects_[id].length!=options.length)
			{//if first time drawing select box
				MultiSelectBox.mySelects_[id]=[];
				for (var opt=0; opt<options.length; opt++)
				{
					MultiSelectBox.mySelects_[id].push(0);
					options[opt].setAttribute("unselectable","on");//make not selectable for ie<10
				}
			}
			else
			{ //if repaint: set highlighted options
				MultiSelectBox.dbg("repaint")
				for (var opt=0; opt < options.length; opt++)
				{
					if (MultiSelectBox.mySelects_[id][opt])
					{
						//MultiSelectBox.dbg(opt);
						MultiSelectBox.addClass(options[opt],"optionhighlighted");
					}
				}
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
	
	MultiSelectBox.toggleClass($(boxid+"searchErr"),"hidden")
	if (MultiSelectBox.toggleClass(textinput,"hidden")){
		$(boxid).style.height = (MultiSelectBox.selInitBoxHeight_-47) + "px";
		$(boxid).style.paddingTop = "42px";
		//$(boxid).childNodes[0].style.marginTop="42px";
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
			
			var offsetx = selRect.left - omniRect.left,
				offsety = selRect.top - omniRect.top;
			var margin = 5;
			
			searchBox.style.position="absolute";
			searchBox.style.top=(offsety)+"px";
			searchBox.style.left=(offsetx)+"px";
			searchBox.style.width=(selRect.width-margin*2-20)+"px";
			
			searchErrBox.style.position="absolute";
			searchErrBox.style.top=(offsety-50)+"px";
			searchErrBox.style.left=(offsetx+0)+"px";
			
			MultiSelectBox.omnis_[id].appendChild(searchBox);
			MultiSelectBox.omnis_[id].appendChild(searchErrBox);
			
			clearInterval(interval);
		}
	}
	
	interval = setInterval( addSearchBox, 50);
	
	imgstr = "<img src='../images/windowContentImages/multiselectbox-magnifying-glass.jpg' " +
				" style='float:left' height='28' width='28' alt='&#128269;' ";
	imgstr += "onclick = 'MultiSelectBox.showSearch(\"" + id + "\");' title='Search' class='searchimg'>";
	return imgstr;  
}



