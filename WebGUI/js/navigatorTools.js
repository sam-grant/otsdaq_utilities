 //------------------------- General utility function ---------------------------
 // Retrieves the local URN number
 var getLocalURN = function(index,name) 
 {      
  var params = (window.location.search.substr(1)).split('&');
  var splitted, vs;
  if(name)
  {
   for(index=0;index<params.length;++index)
   {
    splitted = params[index].indexOf('=');
    if(splitted < 0) continue; 
    vs = [params[index].substr(0,splitted),params[index].substr(splitted+1)];
    if(decodeURIComponent(vs[0]) == name) return decodeURIComponent(vs[1]);
   }
   return; 
  }

  if(index >= params.length) return; 

  splitted = params[index].indexOf('=');
  if(splitted < 0) return;     
  vs = [params[index].substr(0,splitted),params[index].substr(splitted+1)];
  return decodeURIComponent(vs[1]);
 }

 // Printout of navigation schema
 function xmlKeysPrintout(fromWhere)
 {
      const e = new Error();
      const a = e.stack.split("\n")[1] ;
      const w = a.split("/") ;
      const s = w.length -1 ;
      const l = w[s].split(":")[1] ;
      STDLINE("----------- Line: " + l + "-----------")  ;
      STDLINE("From: '"+fromWhere+"'"                 )  ;
      STDLINE("   --> fSystemPath_  : "+fSystemPath_  )  ;
      STDLINE("   --> fRootPath_    : "+fRootPath_    )  ;
      STDLINE("   --> fFoldersPath_ : "+fFoldersPath_ )  ;
      STDLINE("   --> fFileName_    : "+fFileName_    )  ;
      STDLINE("   --> fRFoldersPath_: "+fRFoldersPath_)  ;
      STDLINE("   --> fHistName_    : "+fHistName_    )  ;
      STDLINE("--------------------------------------")  ;
 }

 //-----------------------------------------------------------------------------
 getXMLValue          = function(req, name) 
                        {
                         if(!name) return req.getAttribute("value");       
                         return getXMLAttributeValue(req,name,"value");
                        }

 //-----------------------------------------------------------------------------
 getXMLAttributeValue = function(req, name, attribute) 
                        {
                         var el;
                         if(el = getXMLNode(req,name)) return el.getAttribute(attribute);
                         else if((name == "Error" )&& (!req || !req.responseXML)) 
                          return "Unknown error occured "               + 
                                 "(XML response may have been illegal)!";
                         else
                          return undefined;
                        }

 //-----------------------------------------------------------------------------
 getXMLNode           = function(req, name) 
                        {
                         var els;
                         if(req && req.responseXML) 
                                 req = req.responseXML;
                         if(req)
                         {
                          var i;
                          els = req.getElementsByTagName(name);
                          if(els.length) return els[0];
                         }

                         return undefined;       
                        }
              
 //-----------------------------------------------------------------------------
 getXMLNodes          = function(req, name) 
                        {
                         var els;
                         if(req && req.responseXML) 
                                 req = req.responseXML;
                         if(req)
                         {
                          return req.getElementsByTagName(name);
                         }

                         return undefined;       
                        }
              
