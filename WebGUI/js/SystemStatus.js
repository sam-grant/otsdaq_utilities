
///////////-----------------
var _allAppsArray;// leave undefined to indicate first time in getAppsArray()
var _allContextNames = new Array();
var _allClassNames = new Array();
var _arrayOnDisplayTable = new Array(); // has the array values currently displayed on the table
var intersectionArray = new Array();
				
//functions:			
    // init()
    // ========= server calls ====================
    // getContextNames()
    // getAppsArray()
    // updateAppsArray()
    // ========== Display functions ==============
    // displayTable() -> Inside this function, you can change the color of the cells depending on the state of the app
    // ========= filtering functions =============
    // createFilterList()
    // collapsibleList()
    // selectAll()
    // filterUsingCheckBox()
    // filter()
    // getFilteredArray()
    // isEquivalent()


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

//=====================================================================================
//init called once body has loaded
function init() 
{					
    Debug.log("Calibrations init");

    collapsibleList();

    // Use promises to make code execute in an intutive manner
    // 1. Get context names into an array...then
    // 2. Get app names into an array ....then
    // 3. display the table of array names and call filtering functions
    // 4. repeatedly update the _allAppsArray values
    getContextNames().
        then(getAppsArray).
        then(function(result){
            // display table of apps
            displayTable(result);
                
            // populate filterDiv
            createFilterList();

            // if user clicks on list item instead, tick the checkbox and call filter function
            filterByClickingOnItem();

            // update the _allAppsArray variable with repeated calls to server
            // setTimeout(updateAppsArray, 4000);
            setTimeout(function(){
                updateAppsArray();
            }, 4000);

            
        });

    
    return;
} // end of init()

//=====================================================================================
// The function below gets the available context names from the server
function getContextNames()
{

    return new Promise(function(resolve, reject){
        //get context
        DesktopContent.XMLHttpRequest("Request?RequestType=getContextMemberNames", "", 
            function (req) 
            {	
                var memberNames = req.responseXML.getElementsByTagName("ContextMember");
                _allContextNames = new Array(); //reset

                for(var i=0;i<memberNames.length;++i)
                {
                    _allContextNames.push(memberNames[i].getAttribute("value"));
                }
                // Debug.log(_allContextNames);
                if(_allContextNames.length == 0)
                {
                    Debug.log("Empty context member list found!",Debug.HIGH_PRIORITY);
                    reject("Empty context member list found!");
                }
                
                
                // return _allContextNames
                resolve(_allContextNames);
            }); //end request handler

    }); // end of Promise

}// end of getContextNames()

//=====================================================================================
// This function makes a call to the server and returns an array of objects
// each object contains the details of an application such as the id, name, status etc.
function getAppsArray()
{

    return new Promise(function(resolve, reject){
        DesktopContent.XMLHttpRequest("Request?RequestType=getAppStatus", "", 
        function (req) 
        {
            var appNames, appUrls, appIds, appStatus, appTime, appClasses, appProgress, appContexts;
    


            appNames = req.responseXML.getElementsByTagName("name");
            appIds = req.responseXML.getElementsByTagName("id");
            appStatus = req.responseXML.getElementsByTagName("status");
            appTime = req.responseXML.getElementsByTagName("time");
            appProgress = req.responseXML.getElementsByTagName("progress");
            appClasses = req.responseXML.getElementsByTagName("class");
            appUrls = req.responseXML.getElementsByTagName("url");
            appContexts = req.responseXML.getElementsByTagName("context");

            var i;
            
            if(_allAppsArray === undefined && appTime.length > 1)
            {
            	//first time, check for app status monitoring enabled
            	//	time of 0, indicates app status not updating
            	
            	if(appTime[appTime.length-1].getAttribute("value") == "0")
            	{
            		Debug.log("It appears that active application status monitoring is currently OFF! " +
            				"\n\n\n" +
							"If you want to turn it on, there is a Gateway Supervisor parameter that " +
							"controls it. Set this field to YES in your Context Group Configuration Tree: \n\n" +
							"<b>GatewaySupervisor (record in XDAQApplicationTable) --> \nLinkToSuperivorTable --> \nEnableApplicationStatusMonitoring</b>",
							Debug.HIGH_PRIORITY);
            	}            	
            }
            
            _allAppsArray = new Array();
            _allClassNames = new Array();
            
            for (i = 0; i< appNames.length; i++) 
            {
                _allAppsArray.push({ 
                    "name"      :   appNames[i].getAttribute("value"),
                    "id"        :   appIds[i].getAttribute("value"),
                    "status"    :   appStatus[i].getAttribute("value"),
                    "time"      :   appTime[i].getAttribute("value"),
                    "progress"  :   appProgress[i].getAttribute("value"),
                    "class"     :   appClasses[i].getAttribute("value"),
                    "url"       :   appUrls[i].getAttribute("value"),
                    "context"   :   appContexts[i].getAttribute("value")
                });
                
                // populate the array of classes
                _allClassNames.push(appClasses[i].getAttribute("value"));
            }

            if(_allAppsArray.length == 0)
            {
                Debug.log("Empty apps array!",Debug.HIGH_PRIORITY);
                reject("Empty Empty apps array!");
            }

            //return _allAppsArray;
            resolve(_allAppsArray);
        
        }, 0,0,0,true);// end of request handler
    });// end of Promise

}// end of getAppsArray()

//=====================================================================================
// this function updates the _allAppsArray by making repeated requests to the server 
// at specific time intervals. The function is recursive and makes use of setTimeout()
// because setInterval() can overload the server if a request fails.
function updateAppsArray() 
{
    {
        getAppsArray();
        // intersectionArray = setIntersection(_allAppsArray, _arrayOnDisplayTable); // should return updated array for display
        _arrayOnDisplayTable = setIntersection(_allAppsArray, _arrayOnDisplayTable); 
        displayTable(_arrayOnDisplayTable);
    }
    setTimeout(updateAppsArray, 4000);
}; // end of updateAppsArray()

//=====================================================================================
// this function displays a table with the app array passed into it
function displayTable(appsArray)
{

    // clear the statusDiv
    var statusDivElement = document.getElementById("statusDiv");
    statusDivElement.innerHTML = "";

    //Create a HTML Table element.
    var table = document.createElement("TABLE");
    table.border = "1";

    //Get the count of columns.
    var columnNames = ["Name", "App ID", "Status", "Time", "Progress", "Class", "Application Url", "Context"];
    var columnCount = columnNames.length;

    //Add the header row.
    var row = table.insertRow(-1);
    for (var i = 0; i < columnCount; i++) {
        var headerCell = document.createElement("TH");
        headerCell.innerHTML = columnNames[i];
        row.appendChild(headerCell);
    }

    //Add the data rows.
    for (var i = 0; i < appsArray.length; i++) {
        row = table.insertRow(-1);
        for (var key in appsArray[i]) {
            var cell = row.insertCell(-1);
            cell.innerHTML = appsArray[i][key];
            if (key == "status")
            {
                switch(appsArray[i][key]) {
                    case "Initial":
                        cell.style.backgroundColor = "#77D0FF"// rgb(119, 208, 255); -> colors obtained from state machine
                        break;
                    case "Halted":
                        cell.style.backgroundColor = "#4AB597"; // rgb(74, 181, 151);
                        break;
                    case "Failed":
                        cell.style.backgroundColor = "red"; 
                        break;
                    default:
                  } // end of switch

            }// end of if statement
        }
    }// done with adding data rows

    // add table to statusDiv	
    statusDivElement.appendChild(table);
    
    // keep record of current array on display. This variable is later used to redisplay table after user does filtering
    _arrayOnDisplayTable = appsArray; 

    return 1;

}// end of displayTable()

//=====================================================================================
// this function creates list elements and checkboxes to 
// be displayed in the filterDiv
function createFilterList() 
{
    var contextUl = document.getElementById('contextUl');
    var classUl = document.getElementById('classUl');

    renderFilterList(_allContextNames, contextUl, "contextName");
    renderFilterList(_allClassNames, classUl, "className");

    //========================
    function renderFilterList(elemArray, ulelem, cbName) 
    {

        for (var i = 0; i < elemArray.length; i++)
        {
            var li = document.createElement('li'); // create a list element
            var cb_input = document.createElement('input'); // create a checkbox
            cb_input.setAttribute("type", "checkbox");
            cb_input.setAttribute("class", cbName);
            cb_input.checked = false; // set checkboxes to false
            cb_input.setAttribute("value", elemArray[i]);


            li.setAttribute('class','item');
            li.appendChild(cb_input);
            var textnode;

            if (cbName == "className") 
            {
                textnode = document.createTextNode(elemArray[i].slice(5));// remove "ots::" in display text
            }
            else
            {
                textnode = document.createTextNode(elemArray[i]);
            }

            li.appendChild(textnode);
            ulelem.appendChild(li);
    
        }    
    }// end of renderFilterList()

}// end of createFilterList()

//=====================================================================================
// this function creates a collapsible menu in the filterDiv
// from the available context names, and class names 
function collapsibleList()
{

    var collapsible = document.getElementsByClassName("collapsible");
    
    for (var i = 0; i < collapsible.length; i++) 
    {
      collapsible[i].addEventListener("click", function() 
    		  {

        this.firstElementChild.style.visibility = "hidden";  // make help tooltip hidden

        this.classList.toggle("active");
        var content = this.nextElementSibling;
        if (content.style.display === "block") {
          content.style.display = "none";
        } else {
          content.style.display = "block";
        }
      });

    }
}// end of collapsibleList()

//=====================================================================================
function filterByClickingOnItem() 
{

    var listElements = document.getElementsByTagName("li");

    for (let i = 0; i < listElements.length; i++) 
    {
        listElements[i].addEventListener("click", function() 
        		{

            // tick the checkbox and call filter function
            var listChildren = listElements[i].childNodes;
            // console.log(listChildren);
            for (let j = 0; j < listChildren.length; j++) 
            {

                if(listChildren[j].className == "selectAll")
                {
                    if(listChildren[j].checked)
                    {
                        listChildren[j].checked = false;
                    }
                    else
                    {
                        listChildren[j].checked = true;
                    }

                    checkAllBoxes(listChildren[j].checked);

                    function checkAllBoxes(cbvalue){
                        var parentUl = listChildren[j].parentElement.parentElement;
                        var listSiblings = parentUl.childNodes;

                        for (let k = 0; k < listSiblings.length; k++) {
                            if(listSiblings[k].nodeName == "LI"){
                                listSiblings[k].firstChild.checked = cbvalue;
                            }            
                        }
                        filter();
                    }// end of checkAllBoxes()
                    return;
                }

                if(listChildren[j].nodeName == "INPUT")
                {
                    listChildren[j].checked = true;
                }
                
                
            } // end of for loop
            filter();
        });
        
    }
}// end of filterByClickingOnItem()

//=====================================================================================
function filter() 
{

    
    var filteredClass = getFilteredArray("className"); // filter by class
    var filteredContext = getFilteredArray("contextName"); // filter by context

    // if filterByClass and filterByContext return empty arrays, display the full table
    if (filteredClass.length == 0 && filteredContext.length == 0) 
    {
        displayTable(_allAppsArray);
        return;
    }
    
    // merge the two arrays

    // 1. get apps in filteredContext that are not in filteredClass
    var notInFilteredClass = new Array();
    var common;

    for (let i = 0; i < filteredContext.length; i++) 
    {
        common = false;
        
        for (let j = 0; j < filteredClass.length; j++) 
        {
            if (isEquivalent(filteredContext[i], filteredClass[j])) 
            {
                common = true;
                continue;
            }
            
        }
        if (common == false) 
        {
            notInFilteredClass.push(filteredContext[i]);
        }
        
    }

    // 2. concatenate the two arrays
    var result = filteredClass.concat(notInFilteredClass);

    // display the table
    displayTable(result);
} // end of filter()

//=====================================================================================
function getFilteredArray(className)
{

    var filterobjects = document.getElementsByClassName(className);
    var checkedItems = new Array();

    // loop through elements and find those that are checked
    for(var i = 0; i < filterobjects.length; i++)
    {
        if (filterobjects[i].checked == true)
        {
            var val = filterobjects[i].getAttribute("value");
            checkedItems.push(val);
        }
    }

    // loop through _allAppsArray and get apps that match checked values
    var filtered =  _allAppsArray.filter(function(app) {
        // returns apps that have class/context values in checkedItems array
        return checkedItems.includes(app.class) || checkedItems.includes(app.context); // if value is in checkedItems
    });

    return filtered;
} // end of getFilteredArray()

//=====================================================================================
// compares two objects by value to see if they are the same
function isEquivalent(a, b) 
{
    // Create arrays of property names
    var aProps = Object.getOwnPropertyNames(a);
    var bProps = Object.getOwnPropertyNames(b);

    // If number of properties is different,
    // objects are not equivalent
    if (aProps.length != bProps.length) 
    {
        return false;
    }

    for (var i = 0; i < aProps.length; i++) 
    {
        var propName = aProps[i];

        // If values of same property are not equal,
        // objects are not equivalent
        if (a[propName] !== b[propName]) 
        {
            return false;
        }
    }

    // If we made it this far, objects
    // are considered equivalent
    return true;
} // end of isEquivalent()

//=====================================================================================
// generic function that can be used to get union/intersection of two arrays of objects
function setIntersection(list1, list2) 
{

    result = [];
    for (let i = 0; i < list1.length; i++) 
    {

        for(let j = 0; j < list2.length; j++)
        {
            if (list1[i].id == list2[j].id)
            {
                result.push(list1[i]);
                break;
            }
        }// inner for loop
        
    }// outer for loop
    return result;
} // end of setIntersection()