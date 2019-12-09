Ext.require(['*']);

//------------------ Execute when head has been fully loaded --------------------
Ext.onReady(
function() 
{
 var activeObjects_     = []                                                                      ;
 var canvasTabs_        = []                                                                      ;
 var canvasPos_         = 0                                                                       ;
 var globalCanvas_      = 0                                                                       ;
 var ROOTControlsPanel_ = 0                                                                       ;
 var theViewPort_       = 0                                                                       ;
 var theStore_          = 0                                                                       ;
 var grid_              = ""                                                                      ;
 var dataModel_         = ""                                                                      ;
 var fSystemPath_       = ""                                                                      ;
 var fRootPath_         = ""                                                                      ;
 var fFoldersPath_      = ""                                                                      ;
 var fFileName_         = ""                                                                      ;
 var fHistName_         = ""                                                                      ;
 var fRFoldersPath_     = ""                                                                      ;
 var doReset_           = true                                                                    ;
 var currentCanvas_     = 'canvas1'                                                               ;
 var gridDivision_      = "grid1x1"                                                               ;
 var selectedItem_      = "getDirectories";                                                       ;
 var treeDisplayField_  = "fDisplayName"                                                          ;

 var _cookieCodeMailbox = self.parent.document.getElementById("DesktopContent-cookieCodeMailbox") ;
 var _cookieCode        = _cookieCodeMailbox.innerHTML                                            ;
 var _theWindow         = self                                                                    ;
 var _requestURL        = self.parent.window.location.origin                                     +
                          "/urn:xdaq-application:lid="                                           +
                          getLocalURN(0,"urn")                                                   +
                          "/Request?"                                                             ;
 
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

 Ext.QuickTips.init();

 Ext.state.Manager.setProvider(Ext.create('Ext.state.CookieProvider'));

 generateDIVPlaceholderSize('canvas1',350,440) ;	   
 generateDIVPlaceholderSize('canvas2',350,440) ;	   

 function createCanvasTab(tabNumber)
 {
  STDLINE("Creating canvas tab number "+tabNumber) ;
  var closable = false ;
  if(tabNumber>1) closable = true ;
  canvasTabs_.push(Ext.create(
                              'Ext.panel.Panel',
                              {
                               contentEl : 'canvas' +tabNumber,
                               title     : 'Canvas '+tabNumber,
                               closable  : closable           ,
                               border    : true               ,
                               autoScroll: true
                              }
                             ) 
                  );
  STDLINE("New tab created") ;
 }

 createCanvasTab(1) ;
 createCanvasTab(2) ;

 function makeROOTControlsPanel()
 {
  if( ROOTControlsPanel_ ) ROOTControlsPanel_.destroy() ;
  ROOTControlsPanel_ = Ext.create(
                                  'Ext.tab.Panel',
                                  {
                                   xtype       : 'tabpanel'           ,
                                   region      : 'east'               ,
                                   title       : 'ROOT controls'      ,
                                   id          : 'east-panel'         , // see Ext.getCmp() below
                                   collapsed   : true                 ,
                                   dockedItems : [
                                                  {
                                                   dock : 'top',
                                                   xtype: 'toolbar',
                                                   items: [ 
                                                           '->', 
                                                           {
                                                            xtype  : 'button'                                  ,
                                                            text   : 'Add canvas'                              ,
                                                            tooltip: 'Add a new canvas'                        ,
                                                            border : true                                      ,
                                                            handler: function()
                                                                     {
                                                                      var addIndex = globalCanvas_.items.length ;
                                                                      STDLINE("addIndex: "+addIndex) ;
                                                                      var newIndex = addIndex  + 1 ;
                                                                      STDLINE("newIndex: "+newIndex) ;
                                                                      var tabId = 'canvas' + newIndex ;
                                                                      generateDIVPlaceholderSize(tabId,350,440) ;	   
                                                                      //createCanvasTab(tabs) ;
                                                                      //makeGlobalCanvas() ;
                                                                      //makeViewPort() ;
                                                                      globalCanvas_.insert(
                                                                                           addIndex,
                                                                                           {
                                                                                            contentEl : 'canvas' +newIndex,
                                                                                            title     : 'Canvas '+newIndex,
                                                                                            closable  : true              ,
                                                                                            border    : true              ,
                                                                                            autoScroll: true
                                                                                           }
                                                                                          );
                                                                      globalCanvas_.setActiveTab(addIndex);
                                                                      STDLINE("Adding new canvas tab") ;
                                                                      //makeROOTControlsPanel() ;
                                                                     }
                                                           },
                                                           {
                                                            xtype  : 'button'                                  ,
                                                            text   : 'Stop'                                    ,
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
                                  }
                                 ) ;
 } ; 
 
 makeROOTControlsPanel() ;
 
 function makeGlobalCanvas()
 {
  STDLINE("Creating central panel (with canvas tabs)" ) ;
  if( globalCanvas_ ) globalCanvas_.destroy() ;
  globalCanvas_ = Ext.create(
                             'Ext.tab.Panel', 
                             {
                              id            : 'globalCanvas',
                              region        : 'center'      , // a center region is ALWAYS required for border layout
                              deferredRender: false         ,
                              activeTab     : 0             , // first tab initially active
                              items         : canvasTabs_   ,
                              listeners     : {
                                               tabchange : function( tabPanel, newCard, oldCard, eOpts ) 
                                                           {
                                                            if( newCard.title == "Canvas 1" ) currentCanvas_ = 'canvas1' ;
                                                            if( newCard.title == "Canvas 2" ) currentCanvas_ = 'canvas2' ;
                                                            if( newCard.title == "Canvas 3" ) currentCanvas_ = 'canvas3' ;
                                                            STDLINE("Changed tab to "+ currentCanvas_) ;
                                                           },
                                               resize    : function(thisPanel, width, height, oldWidth, oldHeight, eOpt)
                                                           {
                                                             changeHistogramPanelSize(thisPanel, width, height, oldWidth, oldHeight, eOpt, "resized") ;
                                                           },
                                               collapse  : function(thisPanel, eOpt)
                                                           {
                                                             changeHistogramPanelSize(thisPanel, width, height, oldWidth, oldHeight, eOpt, "collapsed") ;
                                                           },
                                               expand    : function(thisPanel, eOpt)
                                                           {
                                                             changeHistogramPanelSize(thisPanel, width, height, oldWidth, oldHeight, eOpt, "expanded") ;
                                                           }
                                              }
                             }
                            )
 }
 
 makeGlobalCanvas() ;

 function makeViewPort() 
 {
  if( theViewPort_ ) theViewPort_.destroy() ;
  var fName = arguments.callee.toString().match(/function ([^\(]+)/)[1];
  STDLINE("Creating viewport for "+fName) ;
  theViewPort_ = Ext.create(
                            'Ext.Viewport', 
                            {
                             id    : 'border-example',
                             layout: 'border',
                             items : [
                                      // create instance immediately
                                      Ext.create(
                                                 'Ext.Component', 
                                                 {
                                                  region: 'north'     ,
                                                  height: 32          ,
                                                  id    : 'sourcesDiv'
                                                 }
                                                ), 
                                      {
                                       // lazily created panel (xtype:'panel' is default)
                                       region      : 'south'              ,
                                       contentEl   : 'south'              ,
                                       split       : true                 ,
                                       height      : 100                  ,
                                       minSize     : 100                  ,
                                       maxSize     : 200                  ,
                                       collapsible : true                 ,
                                       collapsed   : true                 ,
                                       title       : 'General information',
                                       margins     : '0 0 0 0'
                                      },
                                      ROOTControlsPanel_,
                                      {
                                       region      : 'west'            ,
                                       stateId     : 'navigation-panel',
                                       id          : 'west-panel'      , // see Ext.getCmp() below
                                       title       : 'The navigator'   ,
                                       split       : true              ,
                                       width       : 200               ,
                                       minWidth    : 175               ,
                                       maxWidth    : 400               ,
                                       collapsible : true              ,
                                       animCollapse: true              ,
                                       margins     : '0 0 0 5'         ,
                                       layout      : 'accordion'       ,
                                       items       : [
                                                      {
                                                       title    : 'FileSystem navigation'                                                  ,
                                                       id       : 'navigatorDiv'                                                           ,
                                                       autoScroll: true
                                                      }, {
                                                       title    : 'ROOT file navigation'                                                   ,
                                                       html     : '<p>Here the tree of ROOT files with subfolders, plots and canvases.</p>',
                                                       iconCls  : 'settings'
                                                      }, {
                                                       title    : 'Navigation Controls'                                                    ,
                                                       html     : '<p>Here the controls to drive the filesystem drill down.</p>'           ,
                                                       iconCls  : 'info'
                                                      }
                                                     ],
                                      },
                                      globalCanvas_ 
                                     ]
                            }
                           );
  STDLINE("Viewport created") ;
 } ;
 
 //-----------------------------------------------------------------------------
 function createSources(dirs)
 {
  STDLINE("Creating sources") ;
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
  STDLINE("Sources created...") ;
  theSourcesCB_.setRawValue(dirs[0].dir) ; // Set default value
  STDLINE("Sources populated") ;
 }
 //-----------------------------------------------------------------------------
 function makeGrid(where,what)
 { 
  STDLINE("makeGrid("+where+","+what+")") ;
  if( grid_ ) grid_.destroy()     ;
  theStore_.sort(treeDisplayField_, 'ASC');

  //mdi_ = new JSROOT.GridDisplay('canvas1', gridDivision_); // gridi2x2
 
  STDLINE("Creating grid") ;
  grid_ = Ext.create(
                     'Ext.tree.Panel', 
                     {
                      title      : what                  ,
                      header     : false                 ,
                      id         : 'navigator'           ,
                      store      : theStore_             ,
                      draggable  : true                  ,
                      resizable  : true                  ,
                      border     : true                  ,
                      renderTo   : "navigatorDiv-innerCt",
                      rootVisible: false                 ,
                      useArrows  : true                  ,
                      scrollable : true                  ,
                      selModel   : {
                                    mode : 'MULTI' // SIMPLE or MULTI
                                   },
                      plugins    : [
                                    {
                                     ptype: 'bufferedrenderer'
                                    }
                                   ],
                      buttons    : [
                                    //backButton
                                    {
                                      xtype    : 'button'             ,
                                      text     : '<<'                 ,
                                      margin   : 2                    ,
                                      style    : {
                                                  borderColor: 'blue' ,
                                                  borderStyle: 'solid'
                                                 }                    ,
                                      minWidth : 10                   ,
                                      height   : 25                   ,
                                      width    : 30                   ,
                                      listeners: {
                                                  click: function()
                                                         {
                                                          if( currentTree_ = 'fileContent' )
                                                          {
                                                           selectedItem_ = "getDirectories"               ;
                                                           makeStore(fRootPath_, 'RequestType=getMeDirs') ; 
                                                           makeGrid (fRootPath_, 'Directories and files') ;
                                                          }
                                                         }
                                                 }
                                    }
                                   ],
                      columns    : [
                                    {
                                     xtype    : 'treecolumn'    ,
                                     id       : 'provenance'    ,
                                     text     : where           ,
                                     flex     : 1               ,
                                     dataIndex: 'fDisplayName' 
                                    }, 
                                    { 
                                     xtype    : 'treecolumn'    ,
                                     hidden   : false           ,
                                     text     : 'type'          ,
                                     width    : 1               ,
                                     dataIndex: 'leaf'                 
                                    }, 
                                    { 
                                     xtype    : 'treecolumn'    ,
                                     hidden   : false           ,
                                     text     : 'fSystemPath'   ,
                                     width    : 1               ,
                                     dataIndex: 'fSystemPath'                 
                                    }, 
                                    { 
                                     xtype    : 'treecolumn'    ,
                                     hidden   : false           ,
                                     text     : 'fRootPath'     ,
                                     width    : 1               ,
                                     dataIndex: 'fRootPath'                
                                    }, 
                                    { 
                                     xtype    : 'treecolumn'    ,
                                     hidden   : false           ,
                                     text     : 'fFoldersPath'  ,
                                     width    : 1               ,
                                     dataIndex: 'fFoldersPath'                
                                    }, 
                                    { 
                                      xtype    : 'treecolumn'   ,
                                      hidden   : false          ,
                                      text     : 'fFileName'    ,
                                      width    : 1              ,
                                      dataIndex: 'fFileName'                
                                     }, 
                                     { 
                                      xtype    : 'treecolumn'   ,
                                      hidden   : false          ,
                                      text     : 'fRFoldersPath',
                                      width    : 1              ,
                                      dataIndex: 'fRFoldersPath'                
                                     }, 
                                    { 
                                      xtype    : 'treecolumn'  ,
                                      hidden   : false         ,
                                      text     : 'fHistName'   ,
                                      width    : 1             ,
                                      dataIndex: 'fHistName'                
                                     }
                                   ],
                      listeners  : {
                                    expand    : function(expandedItem, options)  // for some reason doesn't trigegr
                                                {
                                                 STDLINE("expanded") ;
                                                },                                   
                                    itemclick : function(thisItem, record, item, index, e, eOpts)
                                                {
                                                 var selection = this.getSelection()                                ;
                                                 STDLINE("Selected "+selection.length+" items")                     ;
                                                 for(var i=0; i<selection.length; i++)  
                                                 {  
                                                  fSystemPath_   = selection[i].data.fSystemPath                    ;
                                                  fRootPath_     = selection[i].data.fRootPath                      ;
                                                  fFoldersPath_  = selection[i].data.fFoldersPath                   ;
                                                  fFileName_     = selection[i].data.fFileName                      ;
                                                  fRFoldersPath_ = selection[i].data.fRFoldersPath                  ;
                                                  fHistName_     = selection[i].data.fHistName                      ;
                                                  if( typeof fFoldersPath_  === "undefined" ) fFoldersPath_  = ""   ;
                                                  if( typeof fRFoldersPath_ === "undefined" ) fRFoldersPath_ = ""   ;
                                                  xmlKeysPrintout("Clicked on a tree item")
                                                 }  
                                                 STDLINE("Selected "+selection.length+" items")                     ;
                                                 //clearInterval(periodicPlotID_)                                   ;
                                                 var itemSplit     = item.innerText.split("\n\t\n")                 ;
                                                 var isLeaf        = itemSplit[1].replace("\n","").replace("\t","") ;
                                                 if( isLeaf == "true" ) 
                                                 {
                                                  if( selectedItem_ == "getDirectories" )
                                                  {
                                                   treeDisplayField_  = 'fDisplayName'                              ;
                                                   selectedItem_      = "getRootObject"                             ;
                                                   currentTree_       = 'fileContent'                               ;
                                                   currentDirectory_ = fSystemPath_                                +
                                                                       '/'                                         +
                                                                       fRootPath_                                  +
                                                                       "/"                                         +
                                                                       fFoldersPath_                               +
                                                                       "/"                                         +
                                                                       fFileName_                                   ;
                                                   STDLINE('RequestType      : getMeRootFile'     )                 ;
                                                   xmlKeysPrintout("Getting directories in particular")
                                                   STDLINE('currentDirectory_: '+currentDirectory_)                 ;
                                                   makeStore(currentDirectory_,'RequestType=getMeRootFile')         ;
                                                   makeGrid (currentDirectory_,'ROOT file content'        )         ;
                                                  }
                                                  else if( selectedItem_ == "getRootObject" )
                                                  { 
                                                   xmlKeysPrintout("Getting object (getRootObject)")
                                                   currentRootObject_  = "/"                                       +
                                                                         fRootPath_                                +
                                                                         "/"                                       +
                                                                         fFoldersPath_                             +
                                                                         fFileName_                                +
                                                                         "/"                                       +
                                                                         fRFoldersPath_                            +
                                                                         "/"                                       +
                                                                         fHistName_  ;
                                                   STDLINE('RequestType       : getRootObject'      )               ;
                                                   STDLINE('currentRootObject_: '+currentRootObject_)               ;
                                                   theAjaxRequest(
                                                                  _requestURL+"RequestType=getRoot",
                                                                  {                                                           
                                                                   CookieCode: _cookieCode,                                  
                                                                   RootPath  : currentRootObject_                                     
                                                                  }, 
                                                                  ""
                                                                 )
                                                // var tOut = Math.round(timeoutInterval_.getValue() * 1000);
                                                //    periodicPlotID_ = setInterval(
                                                //                                  function()
                                                //                                  {
                                                //                                   STDLINE("Launching Ajax Request with refresh time: "+tOut) ;
                                                //                                   theAjaxRequest(
                                                //                                                  _requestURL+"RequestType=getRoot",
                                                //                                                  {                                                           
                                                //                                                   CookieCode: _cookieCode,                                  
                                                //                                                   RootPath  : currentRootObject_                                     
                                                //                                                  }, 
                                                //                                                  ""
                                                //                                                 )
                                                //                                  },
                                                //                                  tOut 
                                                //                                 )                                                          
                                                  }
                                                 }
                                                },
                                    headerclick: function(ct, column, e, t, eOpts)
                                                 {
                                                  var a = column ;
                                                  STDLINE("header clicked") ;
                                                 }
                                   }
                     }
                    ).setPosition(0,0);
  STDLINE("Grid created") ;                   
  var objectProvenance = Ext.create(
                                    'Ext.tip.ToolTip', 
                                    {
                                     target: 'provenance',
                                     html  : 'Object provenance: ' + where
                                    }
                                   );
 }
 //-----------------------------------------------------------------------------
 dataModel_ = Ext.define(
                         'DirectoriesDataModel',
                         {
                          extend: 'Ext.data.Model',
                          fields: [
                                   {name: 'nChilds'      , type: 'int'   , convert: null},
                                   {name: 'fSystemPath'  , type: 'string', convert: null},
                                   {name: 'fRootPath'    , type: 'string', convert: null},
                                   {name: 'fFoldersPath' , type: 'string', convert: null},
                                   {name: 'fFileName'    , type: 'string', convert: null},
                                   {name: 'fHistName'    , type: 'string', convert: null},
                                   {name: 'fRFoldersPath', type: 'string', convert: null},
                                   {name: 'fDisplayName' , type: 'string', convert: null}
                                  ]
                         }
                        );
 //-----------------------------------------------------------------------------
 function makeStore(path, reqType)
 { 
  xmlKeysPrintout("Sending parameters block to server")
  STDLINE("path       : " + path       ) ;
  STDLINE("reqType    : " + reqType    ) ;
  STDLINE("_requestURL: " + _requestURL) ;
  theStore_ = Ext.create(
                         'Ext.data.TreeStore', 
                         {
                          model    : 'DirectoriesDataModel',
                          id       : 'theStore',
                          autoLoad : false,
                          root     : {
                                      expanded     : true
                                     },
                          proxy    : {
                                      type         : 'ajax',
                                      actionMethods: {
                                                      read          : 'POST'
                                                     }, 
                                      extraParams  : { 
                                                      "CookieCode"  : _cookieCode   ,
                                                      "Path"        : path          , // used by Ryan's part
                                                      "fRootPath"   : fRootPath_    ,
                                                      "fFoldersPath": fFoldersPath_ ,
                                                      "fHistName"   : fHistName_    ,
                                                      "fFileName"   : fFileName_
                                                     },
                                      url          : _requestURL + reqType,
                                      reader       : {
                                                      type          : 'xml'         ,
                                                      root          : 'nodes'       ,
                                                      record        : '> node'
                                                     },
                                     },
                          listeners: {
                                      beforeload   : function(thisStore, operation, eOpts) 
                                                     {
                                                      STDLINE("Request: "+_requestURL + reqType) ;
                                                     },
                                      load         : function( thisStore, records, successful, operation, node, eOpts )
                                                     {
                                                      STDLINE("Load was succesful? "+successful) ;
                                                     }
                                     }
                         }
                        );
  STDLINE("Going to load...") ;
  theStore_.load() ;
  STDLINE("...loaded!") ;
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

                               STDLINE("Create sources") ;
                               createSources(dirs) ;
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
  STDLINE("Ajax request formed") ;                                                                                                
 } ;                                                                                                                                      
 //-----------------------------------------------------------------------------
 displayPlot_ = function()
                {
                 STDLINE("gridDivision_: "+gridDivision_) ;
                 if( gridDivision_ == "grid1x1")
                 { 
                  JSROOT.cleanup(currentCanvas_);
                  mdi_ = new JSROOT.GridDisplay(currentCanvas_, gridDivision_); // gridi2x2
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

 makeStore("where","what") ;
// makeGrid("where","what") ;
 STDLINE("Calling makeViewPort") ;
 makeViewPort() ;
});
