Ext.require(['*']);

//------------------ Execute when head has been fully loaded --------------------
Ext.onReady(
function() 
{
 var _cookieCodeMailbox = self.parent.document.getElementById("DesktopContent-cookieCodeMailbox") ;
 var _cookieCode        = _cookieCodeMailbox.innerHTML                                            ;
 var _theWindow         = self                                                                    ;
 var _requestURL        = self.parent.window.location.origin                                     +
                          "/urn:xdaq-application:lid="                                           +
                          getLocalURN(0,"urn")                                                   +
                          "/Request?"                                                             ;
 
 Ext.QuickTips.init();

 Ext.state.Manager.setProvider(Ext.create('Ext.state.CookieProvider'));

 var viewport = Ext.create(
                           'Ext.Viewport', 
                           {
                            id    : 'border-example',
                            layout: 'border',
                            items : [
                                     // create instance immediately
                                     Ext.create(
                                                'Ext.Component', 
                                                {
                                                 region: 'north',
                                                 height: 32, // give north and south regions a height
                                                 id    : 'sourcesDiv',
                                                 autoEl: {
                                                          tag : 'div',
                                                         }
                                                }
                                               ), 
                                     {
                                      // lazily created panel (xtype:'panel' is default)
                                      region      : 'south',
                                      contentEl   : 'south',
                                      split       : true,
                                      height      : 100,
                                      minSize     : 100,
                                      maxSize     : 200,
                                      collapsible : true,
                                      collapsed   : true,
                                      title       : 'General information',
                                      margins     : '0 0 0 0'
                                     }, 
                                     {
                                      xtype       : 'tabpanel',
                                      region      : 'east',
                                      title       : 'ROOT controls',
                                      id          : 'east-panel', // see Ext.getCmp() below
                                      dockedItems : [
                                                     {
                                                      dock : 'top',
                                                      xtype: 'toolbar',
                                                      items: [ 
                                                              '->', 
                                                              {
                                                               xtype  : 'button'     ,
                                                               text   : 'Stop'      ,
                                                               tooltip: 'Stop periodical refreshing of histograms',
                                                               border : true
                                                              }
                                                             ]
                                                     }
                                                    ],
                                      animCollapse: true,
                                      collapsible : true,
                                      split       : true,
                                      width       : 225, // give east and west regions a width
                                      minSize     : 175,
                                      maxSize     : 400,
                                      margins     : '0 5 0 0',
                                      activeTab   : 1,
                                      tabPosition : 'bottom',
                                      items       : [
                                                     {
                                                      html      : '<p>Here controls of periodic refresh of histograms</p>',
                                                      title     : 'Cycling'                                               ,
                                                      tooltip   : 'Tab to control refreshing of histograms'               ,
                                                      autoScroll: true
                                                     }, 
                                                     Ext.create(
                                                                'Ext.grid.PropertyGrid', 
                                                                {
                                                                 title   : 'Operations on histograms',
                                                                 closable: true,
                                                                 source  : {
                                                                            "(name)"           : "Properties Grid",
                                                                            "grouping"         : false,
                                                                            "autoFitColumns"   : true,
                                                                            "productionQuality": false,
                                                                            "created"          : Ext.Date.parse('10/15/2006', 'm/d/Y'),
                                                                            "tested"           : false,
                                                                            "version"          : 0.01,
                                                                            "borderWidth"      : 1
                                                                           }
                                                                }
                                                               )
                                                    ]
                                     }, 
                                     {
                                      region      : 'west',
                                      stateId     : 'navigation-panel',
                                      id          : 'west-panel', // see Ext.getCmp() below
                                      title       : 'Filesystem navigation',
                                      split       : true,
                                      width       : 200,
                                      minWidth    : 175,
                                      maxWidth    : 400,
                                      collapsible : true,
                                      animCollapse: true,
                                      margins     : '0 0 0 5',
                                      layout      : 'accordion',
                                      items       : [
                                                     {
                                                      contentEl: 'west',
                                                      title    : 'Files navigation',
                                                      iconCls  : 'nav' // see the HEAD section for style used
                                                     }, {
                                                      title    : 'ROOT file navigation',
                                                      html     : '<p>Here the tree of ROOT files with subfolders, plots and canvases.</p>',
                                                      iconCls  : 'settings'
                                                     }, {
                                                      title    : 'Navigation Controls',
                                                      html     : '<p>Here the controls to drive the filesystem drill down.</p>',
                                                      iconCls  : 'info'
                                                     }
                                                    ]
                                     },
                                     // in this instance the TabPanel is not wrapped by another panel
                                     // since no title is needed, this Panel is added directly
                                     // as a Container
                                     Ext.create(
                                                'Ext.tab.Panel', 
                                                {
                                                 region        : 'center', // a center region is ALWAYS required for border layout
                                                 deferredRender: false,
                                                 activeTab     : 0,        // first tab initially active
                                                 items         : [
                                                                  {
                                                                   contentEl : 'center1' ,
                                                                   title     : 'Canvas 1',
                                                                   closable  : false     ,
                                                                   autoScroll: true
                                                                  }, 
                                                                  {
                                                                   contentEl : 'center2' ,      
                                                                   title     : 'Canvas 2', 
                                                                   closable  : true      ,
                                                                   autoScroll: true
                                                                  }
                                                                 ]
                                                }
                                               )
                                    ]
 });
 
 //-----------------------------------------------------------------------------
 function createSources(dirs)
 {
  theSources_ = Ext.create  (
                             'Ext.data.Store', 
                             {
                              fields: ['abbr', 'dir'],
                              data  : dirs
                             }
                            );
  theSourcesCB_ = Ext.create(
                             'Ext.form.ComboBox', 
                             {
                              id          : 'source'   ,    
                              fieldLabel  : 'Source:'  ,
                              labelWidth  : 45         ,
                              height      : 25         ,
                              width       : 200        ,
                              store       : theSources_,
                              queryMode   : 'local'    ,
                              displayField: 'dir'      ,
                              valueField  : 'abbr'     ,
                              renderTo    : 'sourcesDiv',
                              listeners   : {
                                             select    : function(thisCombo, record, eOpts)
                                                         {
                                                          fRootPath_        = record.data.dir            ;
                                                          STDLINE("fRootPath_: "+fRootPath_)             ;
                                                          makeStore(fRootPath_, 'RequestType=getMeDirs') ;
                                                          makeGrid (fRootPath_, 'Directories and files') ;
                                                         },
                                             focusleave: function (thisCombo) 
                                                         {
                                                          STDLINE('remove  selection listener') ;
                                                          thisCombo.suspendEvent('select')      ;
                                                          STDLINE('removed selection listener') ;
                                                         },
                                             focusenter: function (thisCombo) 
                                                         {
                                                          STDLINE('reinstate  selection listener') ;
                                                          thisCombo.resumeEvent('select')          ;
                                                          STDLINE('reinstated selection listener') ;
                                                         }
                                            }
                             }
                            );
  theSourcesCB_.setRawValue(dirs[0].dir) ; // Set default value
 }
 //-----------------------------------------------------------------------------
 function theAjaxRequest(theRequestURL,theParams,theRawData)                                                                   
 { 
  var today = new Date();
  var time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
  STDLINE("Ajax request issued to "+theRequestURL+ " at " + time) ;                                                                                                                                      
  Ext.Ajax.request(                                                                                                                       
                   {                                                                                                                      
                    url    : theRequestURL,                                                                                               
                    method : 'POST'       ,                                                                                                      
                    headers: {                                                                                                            
                              'Content-Type': 'text/plain;charset=UTF-8'                                                                  
                             }            ,                                                                                                           
                    params : theParams    ,                                                                                                          
                    rawData: theRawData   ,                                                                                                  
                    timeout: 50000        ,                                                                                                       
                    success: function(response, request)                                                                                  
                             { 
                              STDLINE("Successful") ;
                              if(getXMLValue(response,"headOfSearch") == 'located') // Returns the list of available fRooPaths                                                                     
                              { // Get list of head-points
                                var dirs     = [] ;
                               var theNodes = getXMLNodes(response,'dir') ;
                               for(var i=0; i<theNodes.length; ++i)
                               {
                                var theDir = theNodes[i].getAttribute("value")
                                STDLINE("Adding "+theDir+" to the list of enty points") ;
                                dirs.push({"abbr":  theDir, "dir": theDir}) ;
                               }

                               createSources(dirs) ;
                               var a = 0 ;                                                    
                              }                                                                                                         
                              else if(!(typeof getXMLValue(response,"rootType") == 'undefined')) // Returns the plot to display                                                                     
                              { // get specific ROOT Object and display
                               canvasPos_++ ;
                        //        if( periodicPlotID_ != "" ) 
                        //        {
                        //         clearInterval(periodicPlotID_) ;
                        //         periodicPlotID_ = "" ;
                        //         doReset_ = true ;
                        //        }
                               var rootName  = getXMLValue (response,"path"    );                                       
                               var rootJSON  = getXMLValue (response,"rootJSON");                                   
                               var object    = JSROOT.parse(rootJSON           );  
                               STDLINE("Launchin displayPlot")                  ;
                               activeObjects_.push(object) ;
                               displayPlot_() ; // This is to get an immediate response
                                //                              JSROOT.RegisterForResize(theFrame);
                        //        if( object._typename != "TCanvas") 
                        //        {
                        //         periodicPlotID_ = setInterval(
                        //                                       function()
                        //                                       {
                        //                                        displayPlot_(object) ; // This is delayed
                        //                                       }, 
                        //                                       2000
                        //                                      ) ;
                        //         }
                              }
                             },                                                                                                           
                    failure: function(response, options)                                                                                  
                             {                                                                                                            
                              var a = response ;                                                                                          
                              Ext.MessageBox.alert(                                                                                       
                                                   'Something went wrong:',                                                               
                                                   'Response: ' + response.responseText                                                   
                                                  );                                                                                      
                             }                                                                                                            
                   }                                                                                                                      
           );                                                                                                
 } ;                                                                                                                                      
 //-----------------------------------------------------------------------------
 displayPlot_ = function()
                {
                 STDLINE("gridDivision_: "+gridDivision_) ;
                 if( gridDivision_ == "grid1x1")
                 { 
                  JSROOT.cleanup('histogram1');
                  mdi_ = new JSROOT.GridDisplay('histogram1', gridDivision_); // gridi2x2
                  STDLINE("cleared...") ;
                }

                 for(var i=0; i<activeObjects_.length; i++)
                 {
                  var index = canvasPos_ % mdi_.NumGridFrames() ;
                  if( index > mdi_.NumGridFrames()) {index = 0}
                  var pos = "item" + index ; 

                  if (mdi_!=null) theFrame = mdi_.FindFrame(pos, true);
                  var rootTitle = activeObjects_[i].fTitle     ; 
                  if( doReset_ )
                  {
                   STDLINE("-------> Resetting " + rootTitle);
                   JSROOT.redraw (
                                  theFrame         ,
                                  activeObjects_[i],
                                  ""
                                 );
                   doReset_ = false ;                                                                                
                  }
                  else
                  {
                   STDLINE("-------> Updating " + rootTitle) ;
                   JSROOT.redraw (
                                theFrame         ,
                                activeObjects_[i],
                                ""
                               );                                                                                
                  }
                 }
                }
 //-----------------------------------------------------------------------------
 // get a reference to the HTML element with id "hideit" and add a click listener to it
 Ext.get("hideit").on(
                      'click', 
                      function()
                      {
                       // get a reference to the Panel that was created with id = 'west-panel'
                       var w = Ext.getCmp('west-panel');
                       // expand or collapse that Panel based on its collapsed property state
                       w.collapsed ? w.expand() : w.collapse();
                       var e = Ext.getCmp('east-panel');
                       // expand or collapse that Panel based on its collapsed property state
                       e.collapsed ? e.expand() : e.collapse();
                      }
                     );
 theAjaxRequest(
                _requestURL+"RequestType=getDirectoryContents",
                {                                                            
                 CookieCode: _cookieCode,                                   
                 Path      : "/"                                 
                }, 
                ""
               ) ;                                                          

});
