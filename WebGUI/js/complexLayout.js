Ext.require(['*']);

//------------------ Execute when head has been fully loaded --------------------
Ext.onReady(
function() 
{
 var activeObjects_       = {}                                                                      ;
 var activeObjectsVec_    = []                                                                      ;
 var canvasTabs_          = []                                                                      ;
 var canvasPos_           = 0                                                                       ;
 var globalCanvas_        = 0                                                                       ;
 var ROOTControlsPanel_   = 0                                                                       ;
 var theInformationPanel_ = 0                                                                       ;
 var theNavigatorPanel_   = 0                                                                       ;
 var theViewPort_         = 0                                                                       ;
 var theStore_            = 0                                                                       ;
 var nxPlots_             = 1                                                                       ;
 var nyPlots_             = 1                                                                       ;
 var grid_                = ""                                                                      ;
 var dataModel_           = ""                                                                      ;
 var fSystemPath_         = ""                                                                      ;
 var fRootPath_           = ""                                                                      ;
 var fFoldersPath_        = ""                                                                      ;
 var fFileName_           = ""                                                                      ;
 var fHistName_           = ""                                                                      ;
 var fRFoldersPath_       = ""                                                                      ;
 var doReset_             = true                                                                    ;
 var currentCanvas_       = 'canvas1'                                                               ;
 var currentDiv_          = 'canvas1'                                                               ;
 var gridDivision_        = "grid1x1"                                                               ;
 var selectedItem_        = "getDirectories";                                                       ;
 var treeDisplayField_    = "fDisplayName"                                                          ;
 var buttonStyle_         = 'margin-left:5px;padding:5px;'                                          ;
 var _cookieCodeMailbox   = self.parent.document.getElementById("DesktopContent-cookieCodeMailbox") ;
 var _cookieCode          = _cookieCodeMailbox.innerHTML                                            ;
 var _theWindow           = self                                                                    ;
 var _requestURL          = self.parent.window.location.origin                                     +
                            "/urn:xdaq-application:lid="                                           +
                            getLocalURN(0,"urn")                                                   +
                            "/Request?"                                                             ;

 var theCanvasModel_ = {
                        canvases     : [
                                        {
                                         canvasName: 'canvas1',
                                         nDivX     : 1        ,
                                         nDivY     : 1
                                        },
                                        {
                                         canvasName: 'canvas2',
                                         nDivX     : 2        ,
                                         nDivY     : 2
                                        }
                                       ],
                        currentCanvas : 'canvas1',
                        currentDiv    : 1        ,
                        addCanvas     : function(newName)
                                        {
                                         var l = this.canvases.length ;
                                         this.canvases[l] = {
                                                             canvasName: newName,
                                                             nDivX     : 3      ,
                                                             nDivY     : 4
                                                            }
                                        },  
                        removeCanvas  : function(number)
                                        {
                                         var index = this.canvases.indexOf(number) ;
                                         this.canvases.splice(index,1) ;
                                        },  
                        changenDivX   : function(canvasNumber, newnDivX)
                                        {
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         this.canvases[canvasNumber].nDivX = newnDivX ;
                                        },
                        changenDivY   : function(canvasNumber, newnDivY)
                                        {
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         this.canvases[canvasNumber].nDivY = newnDivY ;
                                        }
                       } ;


 activeObjects_[currentCanvas_] = activeObjectsVec_ ;
  
 // Printout of navigation schema ----------------------------------------------------------------
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

 //-----------------------------------------------------------------------------------------------
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

 //-----------------------------------------------------------------------------------------------
 function makeROOTControlsPanel()
 {
  if( ROOTControlsPanel_ ) ROOTControlsPanel_.destroy() ;
  ROOTControlsPanel_ = Ext.create(
                                  'Ext.panel.Panel',
                                  {
                                   region      : 'east'              ,
                                   id          : 'east-panel'        ,
                                   title       : 'ROOT Controls'     ,
                                   split       : true                ,
                                   width       : 50                  ,
                                   minWidth    : 175                 ,
                                   maxWidth    : 400                 ,
                                   collapsible : true                ,
                                   collapsed   : true                ,
                                   animCollapse: true                ,
                                   //multi       : true                ,
                                   margins     : '0 0 0 5'           ,
                                   layout      : 'accordion'         ,
                                   items       : [
                                                  {
                                                   title     : 'Canvas commands'                                       ,
                                                   autoScroll: true                                                    ,
                                                   layout    : 'vbox'                                                  ,
                                                   items     : [
                                                                {
                                                                 xtype     : 'button'                                  ,
                                                                 text      : 'Add canvas'                              ,
                                                                 tooltip   : 'Add a new canvas'                        ,
                                                                 pressed   : true                                      ,
                                                                 border    : true                                      ,
                                                                 style     : buttonStyle_                              ,      
                                                                 handler   : function()
                                                                             {
                                                                              var addIndex = globalCanvas_.items.length ;
                                                                              STDLINE("addIndex: "+addIndex)            ;
                                                                              var newIndex = addIndex  + 1              ;
                                                                              STDLINE("newIndex: "+newIndex)            ;
                                                                              var tabId = 'canvas' + newIndex           ;
                                                                              generateDIVPlaceholderSize(tabId,350,440) ;          
                                                                              //createCanvasTab(tabs) ;
                                                                              //makeGlobalCanvas() ;
                                                                              //makeViewPort() ;
                                                                              currentCanvas_ = 'canvas' +newIndex ;
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
                                                                              activeObjectsVec_ = [] ;
                                                                              activeObjects_[currentCanvas_] = activeObjectsVec_ ;
                                                                              STDLINE("Adding new canvas tab")    ;
                                                                              //makeROOTControlsPanel() ;
                                                                             }
                                                                }, {
                                                                 xtype     : 'numberfield'       ,  
                                                                 name      : 'hzon'              ,
                                                                 labelWidth: 80                  ,
                                                                 flex      : 0                   ,
                                                                 width     : 140                 ,
                                                                 height    : 18                  ,
                                                                 fieldLabel: '# hor. plots'      ,  
                                                                 value     : 1                   ,  
                                                                 minValue  : 1                   ,  
                                                                 maxValue  : 20                  ,
                                                                 style     : buttonStyle_        ,      
                                                                 listeners : {
                                                                              change: function( thisSpinner, newValue, oldValue, eOpts )
                                                                                      {
                                                                                       nxPlots_      = newValue ;
                                                                                       gridDivision_ = 'grid' + nxPlots_ + 'x' + nyPlots_ ;
                                                                                       JSROOT.cleanup(currentCanvas_);
                                                                                       activeObjectsVec_ = [] ;
                                                                                       activeObjects_[currentCanvas_] = activeObjectsVec_ ;
                                                                                      }
                                                                             }
                                                                }, {
                                                                 xtype     : 'numberfield'       ,  
                                                                 name      : 'vzon'              ,
                                                                 labelWidth: 80                  ,
                                                                 flex      : 0                   ,
                                                                 width     : 140                 ,
                                                                 fieldLabel: '# ver. plots'      ,  
                                                                 value     : 1                   ,  
                                                                 minValue  : 1                   ,  
                                                                 maxValue  : 20                  ,
                                                                 style     : buttonStyle_        ,      
                                                                 listeners : {
                                                                              change: function( thisSpinner, newValue, oldValue, eOpts )
                                                                                      {
                                                                                       nyPlots_      = newValue ;
                                                                                       gridDivision_ = 'grid' + nxPlots_ + 'x' + nyPlots_ ;
                                                                                       JSROOT.cleanup(currentCanvas_);
                                                                                       activeObjectsVec_ = [] ;
                                                                                       activeObjects_[currentCanvas_] = activeObjectsVec_;
                                                                                      }
                                                                             }
                                                                }, {
                                                                 xtype     : 'button'                                  ,
                                                                 text      : 'Clear canvas'                            ,
                                                                 pressed   : true                                      ,
                                                                 tooltip   : 'Clear the current canvas content'        ,
                                                                 border    : true                                      ,
                                                                 style     : buttonStyle_                              ,      
                                                                 handler   : function()  
                                                                             {
                                                                              JSROOT.cleanup(currentCanvas_);
                                                                              mdi_ = new JSROOT.GridDisplay(currentCanvas_, gridDivision_); 
                                                                              activeObjectsVec_ = [] ;
                                                                              activeObjects_[currentCanvas_] = activeObjectsVec_;
                                                                              var len = activeObjects_[currentCanvas_].length ;
                                                                              STDLINE("activeObjects_ cleared for "+currentCanvas_+" len: "+len) ;
                                                                             }
                                                                }                             
                                                               ]
                                                  }, {
                                                   title     : 'Timing and Refresh Controls'                           ,
                                                   html      : '<p>Controls to drive the filesystem drill down.</p>'   ,
                                                   autoScroll: true                                                    ,
                                                   padding   : '5 5 5 5'                                               ,
                                                   iconCls   : 'info'                                                  ,
                                                   items     : [
                                                                {
                                                                 xtype     : 'button'                                  ,
                                                                 text      : 'Stop'                                    ,
                                                                 pressed   : true                                      ,
                                                                 style     : buttonStyle_                              ,      
                                                                 tooltip   : 'Stop periodical refreshing of histograms',
                                                                 border    : true
                                                                }
                                                               ]
                                                  }
                                                 ]
                                  }
                                 );
//                                  );               

 } ; 
 
 makeROOTControlsPanel() ;
 
 //-----------------------------------------------------------------------------------------------
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
                                               tabchange : function( thisPanel, newCard, oldCard, eOpts ) 
                                                           {
                                                            var regex      = /\d+/g               ;
                                                            var str        = newCard.title        ;
                                                            var matches    = str.match(regex)     ;
                                                            currentCanvas_ = 'canvas' + matches[0];
                                                            STDLINE("Changed tab to "+ currentCanvas_) ;
                                                           },
                                               resize    : function(thisPanel, width, height, oldWidth, oldHeight, eOpt)
                                                           {
                                                            STDLINE("Resizing "+currentCanvas_) ;
                                                            changeHistogramPanelSize(thisPanel, width, height, currentCanvas_, "resized"    ) ;
                                                           },
                                               collapse  : function(thisPanel, eOpt)
                                                           {
                                                            STDLINE("Collapsed "+currentCanvas_) ;
                                                            changeHistogramPanelSize(thisPanel, width, height, currentCanvas_, "collapsed"  ) ;
                                                           },
                                               expand    : function(thisPanel, eOpt)
                                                           {
                                                            STDLINE("Expanded "+currentCanvas_) ;
                                                            changeHistogramPanelSize(thisPanel, width, height, currentCanvas_, "expanded"   ) ;
                                                           }
                                              }
                             }
                            )
 }
 
 makeGlobalCanvas() ;

 //-----------------------------------------------------------------------------------------------
 function makeNavigatorPanel()
 {
  if( theNavigatorPanel_ ) theNavigatorPanel_.destroy() ;
  theNavigatorPanel_ = Ext.create(
                                  'Ext.panel.Panel',
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
                                   multi       : true              ,
                                   margins     : '0 0 0 5'         ,
                                   layout      : 'accordion'       ,
                                   items       : [
                                                  {
                                                   title     : 'FileSystem navigation'                                    ,
                                                   id        : 'navigatorDiv'                                             ,
                                                   autoScroll: true                                                       ,
                                                   tools     : [
                                                                {
                                                                 type   : 'left'                                          ,
                                                                 tooltip: 'Go back to list of folders and files'          ,
                                                                 handler: function()
                                                                          {
                                                                           if( currentTree_ = 'fileContent' )
                                                                           {
                                                                            selectedItem_ = "getDirectories"               ;
                                                                            makeStore(fRootPath_, 'RequestType=getMeDirs') ; 
                                                                            makeGrid (fRootPath_, 'Directories and files') ;
                                                                           }
                                                                          }
                                                                },
                                                                {
                                                                 type   : 'prev'                                          ,
                                                                 tooltip: 'Maximize canvas size'                          ,
                                                                 handler: function()
                                                                          {
                                                                           STDLINE("Collapsing") ;
                                                                           theNavigatorPanel_  .collapse() ;
                                                                           ROOTControlsPanel_  .collapse() ;
                                                                           theInformationPanel_.collapse() ;
                                                                          }
                                                                }
                                                               ]
                                                  }, {
                                                   title     : 'ROOT file navigation'                                     ,
                                                   html      : '<p>ROOT files with subfolders, plots and canvases.</p>'   ,
                                                   autoScroll: true                                                       ,
                                                   iconCls   : 'settings'
                                                  }, {
                                                   title     : 'Navigation Controls'                                      ,
                                                   html      : '<p>Controls to drive the filesystem drill down.</p>'      ,
                                                   autoScroll: true                                                       ,
                                                   iconCls   : 'info'
                                                  }
                                                 ],
                                   listeners   : {
                                                  collapse   : function() 
                                                               {
                                                                STDLINE("Expand!!!");
                                                               }
                                                 }
                                  },
                                  
                                 ) ;
 }
 
 makeNavigatorPanel() ;
 
 //-----------------------------------------------------------------------------------------------
 function makeInformationPanel()
 {
  if( theInformationPanel_ ) theInformationPanel_.destroy() ;
  theInformationPanel_ = Ext.create(
                                    'Ext.panel.Panel',
                                    {
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
                                   ) ;
 }
 
 makeInformationPanel() ;
 
 //-----------------------------------------------------------------------------------------------
 function makeViewPort() 
 {
  if( theViewPort_ ) theViewPort_.destroy() ;
  var fName = arguments.callee.toString().match(/function ([^\(]+)/)[1];
  STDLINE("Creating viewport for "+fName) ;
  theViewPort_ = Ext.create(
                            'Ext.Viewport'                            , 
                            {
                             id    : 'border-example'                 ,
                             layout: 'border'                         ,
                             items : [
                                      Ext.create(
                                                 'Ext.Component'      , 
                                                 {
                                                  region: 'north'     ,
                                                  height: 32          ,
                                                  id    : 'sourcesDiv'
                                                 }
                                                ), 
                                      theInformationPanel_            ,
                                      ROOTControlsPanel_              ,
                                      theNavigatorPanel_              ,
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
                                                          selectedItem_ = "getDirectories" ;
                                                         },
                                             focusleave: function (thisCombo) 
                                                         {
                                                          STDLINE('remove  selection listener') ;
                                                          //thisCombo.suspendEvent('select')      ;
                                                          theSourcesCB_.suspendEvent('select')      ;
                                                          STDLINE('removed selection listener') ;
                                                         },
                                             focusenter: function (thisCombo) 
                                                         {
                                                          STDLINE('reinstate  selection listener') ;
                                                          thisCombo.resumeEvent('select')          ;
                                                          theSourcesCB_.resumeEvent('select')          ;
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
                                                  if( typeof fFileName_     === "undefined" ) fFileName_     = ""   ;
                                                  if( typeof fHistName_     === "undefined" ) fHistName_     = ""   ;
                                                  xmlKeysPrintout("Clicked on a tree item")
                                                 }  
                                                 STDLINE("Selected     : "+selection.length+" items")               ;
                                                 STDLINE("selectedItem_: "+selectedItem_            )               ;
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
                               STDLINE("headOfSearch") ;
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
                               var rootName  = getXMLValue (response,"path"    ) ;                                      
                               var rootJSON  = getXMLValue (response,"rootJSON") ;                                  
                               var object    = JSROOT.parse(rootJSON           ) ; 
                               STDLINE("Launching displayPlot")                  ;
                               activeObjectsVec_.push(object)                    ;
                               activeObjects_[currentCanvas_] = activeObjectsVec_;
                               displayPlot_(currentCanvas_)                      ;
                                //                                JSROOT.RegisterForResize(theFrame);
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
 var pp_ = 1 ;                                                                                                                                     
 //-----------------------------------------------------------------------------
 displayPlot_ = function(currentCanvas_)
                {
                 STDLINE("gridDivision_: "+gridDivision_) ;
                 JSROOT.cleanup(currentCanvas_);
                 try
                 {
                  mdi_ = new JSROOT.GridDisplay(currentCanvas_, gridDivision_); // gridi2x2
                 }
                 catch(error)
                 {
                  STDLINE("ERROR: "+error) ;
                 }
                 STDLINE("Serching objects for "+currentCanvas_) ;
                 var activeObjectsList = activeObjects_[currentCanvas_] ;
                 STDLINE("Examining "+activeObjectsList.length+" objects") ;
                 for(var i=0; i<activeObjectsList.length; i++)
                 {
                  var index = canvasPos_ % mdi_.NumGridFrames() ;
                  STDLINE("index: "+index) ;
                  if( index > mdi_.NumGridFrames()) {index = 0}
//                  var pos = "item" + index ; 
                  var pos = pp_ ; pp_++ ; if( pp_ > mdi_.NumGridFrames() ) {pp_=1;}
                  STDLINE("pos: "+pos+" canvasPos_: "+canvasPos_+" mdi_:"+mdi_.NumGridFrames()) ;
                  if (mdi_!=null) theFrame = mdi_.FindFrame(pos, true);
                  var rootTitle = activeObjectsList[i].fTitle     ; 
                  if( doReset_ )
                  {
                   STDLINE("-------> Resetting " + rootTitle);
                   JSROOT.redraw (
                                  theFrame         ,
                                  activeObjectsList[i],
                                  ""
                                 );
                   doReset_ = false ;                                                                                
                  }
                  else
                  {
                   STDLINE("-------> Updating " + rootTitle) ;
                   JSROOT.redraw (
                                  theFrame         ,
                                  activeObjectsList[i],
                                  ""
                                 );                                                                              
                  }
                 }
                }
 //-----------------------------------------------------------------------------
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
 
 STDLINE("theCanvasModel_.currentCanvas: "  +
          theCanvasModel_.currentCanvas     +
         " has "                            +
         theCanvasModel_.canvases.length    +
         " canvases") ;
 theCanvasModel_.addCanvas('canvas4') ;
 STDLINE("-------- Before ------------") ;
 for(var i=0; i<theCanvasModel_.canvases.length; i++)
 {
  STDLINE("canvas"                               +
          i                                      +
          "] name: "                             +
          theCanvasModel_.canvases[i].canvasName +
          " nDivX: "                             +
          theCanvasModel_.canvases[i].nDivX      +
          " nDivY: "                             +
          theCanvasModel_.canvases[i].nDivY      ) ;
 } 
 STDLINE("-------- After  ------------") ;
 theCanvasModel_.removeCanvas(1) ;
 theCanvasModel_.changenDivX(0,16) ;
 theCanvasModel_.changenDivY(0,4 ) ;
 for(var i=0; i<theCanvasModel_.canvases.length; i++)
 {
  STDLINE("canvas"                               +
          i                                      +
          "] name: "                             +
          theCanvasModel_.canvases[i].canvasName +
          " nDivX: "                             +
          theCanvasModel_.canvases[i].nDivX      +
          " nDivY: "                             +
          theCanvasModel_.canvases[i].nDivY      ) ;
 } 
});
