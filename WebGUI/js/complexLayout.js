/*---------------------------------------------------------------------------------------------------
 Author : D. Menasce
 Purpose: Code to instantiate and manipulate the histogram navigator component
-------------------------------------------------------------------------------------------------- */

Ext.require(['*']);
Ext.QuickTips.init();

Ext.state.Manager.setProvider(Ext.create('Ext.state.CookieProvider'));


//---------------------------- Execute noly once head has been fully loaded -------------------------
Ext.onReady(
function() 
{
 var enableDebug_         = false                                                                   ;
 var canvasTabs_          = []                                                                      ;
 var currentPad_          = 0                                                                       ;
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
 var mdi_                 = ""                                                                      ;
 var doReset_             = true                                                                    ;
 var clearCanvas_         = true                                                                    ;     
 var superimposeFlag_     = false                                                                   ;
 var currentCanvas_       = 1                                                                       ;
 var currentDiv_          = 'canvas1'                                                               ;
 var gridDivision_        = "grid1x1"                                                               ;
 var selectedItem_        = "getDirectories";                                                       ;
 var treeDisplayField_    = "fDisplayName"                                                          ;
 var buttonStyle_         = 'margin-left  : 2px;' +
                            'margin-right : 2px;' + 
                            'margin-top   : 2px;' +
                            'margin-bottom: 2px;' +
                            'padding      : 2px;'                                                   ;
 var _cookieCodeMailbox   = self.parent.document.getElementById("DesktopContent-cookieCodeMailbox") ;
 var _cookieCode          = _cookieCodeMailbox.innerHTML                                            ;
 var _theWindow           = self                                                                    ;
 var _requestURL          = self.parent.window.location.origin                                     +
                            "/urn:xdaq-application:lid="                                           +
                            getLocalURN(0,"urn")                                                   +
                            "/Request?"                                                             ;


 enableSTDLINE(enableDebug_) ;

 generateDIVPlaceholderSize('canvas1',350,440) ;	   
 generateDIVPlaceholderSize('canvas2',350,440) ;	   
 //--------------------------------------------------------------------------------------------------
 getCanvasDiv_ = function(number)
                 {
                  return 'canvas' + number ;
                 }
 
 //--------------------------------------------------------------------------------------------------
 var theProvenance_ = {
                       fSystemPath_   : "",                                                 
                       fRootPath_     : "",                                                 
                       fFoldersPath_  : "",                                                 
                       fFileName_     : "",                                                 
                       fHistName_     : "",                                                 
                       fRFoldersPath_ : "",                                                 
                       setSystemPath  : function(SystemPath)                                
                                        { 
                                         if( typeof SystemPath   === "undefined" ) this.fSystemsPath_  = "" 
                                         else                                      this.fSystemPath_   = SystemPath  ;                     
                                        },                                                   
                       setRootPath    : function(RootPath)                                   
                                        {                                                    
                                         if( typeof RootPath     === "undefined" ) this.fRootPath_     = "" 
                                         else                                      this.fRootPath_     = RootPath    ;                     
                                        },                                                   
                       setFoldersPath : function(FoldersPath)                                
                                        {                                                    
                                         if( typeof FoldersPath  === "undefined" ) this.fFoldersPath_  = "" 
                                         else                                      this.fFoldersPath_  = FoldersPath ;                     
                                        },                                                   
                       setFileName    : function(FileName)                                   
                                        {                                                    
                                         if( typeof FileName     === "undefined" ) this.fFileName_     = "" 
                                         else                                      this.fFileName_     = FileName    ;                  
                                        },                                                   
                       setHistName    : function(HistName)                                   
                                        {                                                    
                                         if( typeof HistName     === "undefined" ) this.fHistName_     = "" 
                                         else                                      this.fHistName_     = HistName    ;                  
                                        },                                                   
                       setRFoldersPath: function(RFoldersPath)                              
                                        {                                                   
                                         if( typeof RFoldersPath === "undefined" ) this.fRFoldersPath_ = "" 
                                         else                                      this.fRFoldersPath_ = RFoldersPath;                   
                                        },                                                  
                       getSystemPath  : function(SystemPath)                                
                                        {                                                   
                                         return this.fSystemPath_   ;                      
                                        },                                            
                       getRootPath    : function(RootPath)                            
                                        {                                             
                                         return this.fRootPath_     ;                      
                                        },                                            
                       getFoldersPath : function(FoldersPath)                         
                                        {                                             
                                         return this.fFoldersPath_  ;                      
                                        },                                            
                       getFileName    : function(FileName)                            
                                        {                                             
                                         return this.fFileName_     ;                      
                                        },                                            
                       getHistName    : function(HistName)                            
                                        {                                             
                                         return this.fHistName_     ;                      
                                        },                                            
                       getRFoldersPath: function(RFoldersPath)                        
                                        {                                             
                                         return this.fRFoldersPath_ ;                      
                                        },                                                  
                       dump           : function(fromWhere)                                 
                                        {                                                   
                                         const e = new Error()                                 ;
                                         const a = e.stack.split("\n")[1]                      ;
                                         const w = a.split("/")                                ;
                                         const s = w.length -1                                 ;
                                         const l = w[s].split(":")[1]                          ;
                                         STDLINE("----------- Line: " + l + "-----------"     ); 
                                         STDLINE("From: '"+fromWhere+"'"                      ); 
                                         STDLINE("   --> fSystemPath_  : "+this.fSystemPath_  ); 
                                         STDLINE("   --> fRootPath_    : "+this.fRootPath_    ); 
                                         STDLINE("   --> fFoldersPath_ : "+this.fFoldersPath_ ); 
                                         STDLINE("   --> fFileName_    : "+this.fFileName_    ); 
                                         STDLINE("   --> fRFoldersPath_: "+this.fRFoldersPath_); 
                                         STDLINE("   --> fHistName_    : "+this.fHistName_    ); 
                                         STDLINE("--------------------------------------"     ); 
                                        }                                                   
                      } ;                                                                   

 //--------------------------------------------------------------------------------------------------
 var theCanvasModel_ = {
                        canvases      : [
                                         {
                                          canvasName: 'canvas1',
                                          nDivX     : 1        ,
                                          nDivY     : 1        ,
                                          divPos    : 1        ,
                                          objects   : []       
                                         },
                                         {
                                          canvasName: 'canvas2',
                                          nDivX     : 1        ,
                                          nDivY     : 1        ,
                                          divPos    : 1        ,
                                          objects   : []       
                                         }
                                        ],
                        currentCanvas : 0  ,
                        currentDiv    : 1  ,
                        currentWidth  : 350,
                        currentHeight : 440,
                        addCanvas     : function()
                                        {
                                         var l = this.canvases.length ;
                                         this.canvases[l] = {
                                                             canvasName: 'canvas' + l,
                                                             nDivX     : 1           ,
                                                             nDivY     : 1           ,
                                                             divPos    : 1           ,
                                                             objects   : []          
                                                            }
                                        },  
                        addROOTObject : function(canvasNumber, object)
                                        {
                                         canvasNumber--                                     ; // Canvas array starts from zero!
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         var n = this.canvases[canvasNumber].objects.length ;
                                         for(var i=0; i<n; i++)
                                         { 
                                          var o = this.canvases[canvasNumber].objects[i]    ;
                                          if(o.fName == object.fName ) 
                                          {
                                           STDLINE("Object "+object.fName+" already there") ;
                                           return ;
                                          }
                                         }
                                         this.canvases[canvasNumber].objects.push(object)   ;
                                        },
                        getROOTObjects: function(canvasNumber)
                                        {
                                         canvasNumber--                                     ;
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         return this.canvases[canvasNumber].objects         ;
                                        },
                        clearCanvas   : function(canvasNumber)
                                        {
                                         canvasNumber--                                     ;
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         currentPad_ = 0                                    ;
                                         this.canvases[canvasNumber].objects.length = 0     ;
                                         this.canvases[canvasNumber].objects        = []    ;
                                        },  
                        removeCanvas  : function(canvasNumber)
                                        {
                                         canvasNumber--                                     ;
                                         var index = this.canvases.indexOf(canvasNumber)    ;
                                         this.canvases.splice(index,1)                      ;
                                        },  
                        changenDivX   : function(canvasNumber, newnDivX)
                                        {
                                         canvasNumber--                                     ; 
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         this.canvases[canvasNumber].nDivX = newnDivX       ;
                                        },
                        changenDivY   : function(canvasNumber, newnDivY)
                                        {
                                         canvasNumber--                                     ; 
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         this.canvases[canvasNumber].nDivY = newnDivY       ;
                                        },
                        setnDivX      : function(canvasNumber, newValue)
                                        {
                                         canvasNumber--                                     ; 
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         STDLINE("setting divX: "+newValue)                 ;
                                         this.canvases[canvasNumber].nDivX = newValue       ;
                                        },                
                        getnDivX      : function(canvasNumber)
                                        {
                                         canvasNumber--                                     ; 
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         STDLINE("divX: "+this.canvases[canvasNumber].nDivX);
                                         return this.canvases[canvasNumber].nDivX           ;
                                        },                
                        setnDivY      : function(canvasNumber, newValue)
                                        {
                                         canvasNumber--                                     ; 
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         STDLINE("setting divY: "+newValue)                 ;
                                         this.canvases[canvasNumber].nDivY = newValue       ;
                                        },                
                        getnDivY      : function(canvasNumber)
                                        {
                                         canvasNumber--                                     ; 
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         STDLINE("divY: "+this.canvases[canvasNumber].nDivY);
                                         return this.canvases[canvasNumber].nDivY           ;
                                        },                
                        setDivPosition: function(canvasNumber, posX, posY)
                                        {
                                         canvasNumber--                                     ; 
                                         if( canvasNumber > this.canvases.length-1 ) return ;
                                         var nx   = this.canvases[canvasNumber].nDivX       ;
                                         var ny   = this.canvases[canvasNumber].nDivY       ;
                                         if( posX > nx || posY > ny ) return                ;
                                         var modY = posY%nx                                 ;
                                         var pos  = modY * nx + posX                        ;
                                         this.canvases[canvasNumber].divPos = pos           ;
                                        },
                        dumpContent   : function()
                                        {
                                         STDLINE("=============== theCanvasModel_ dump =====================") ;
                                         STDLINE("Size: " + this.currentWidth + "x" + this.currentHeight     ) ;
                                         STDLINE("Number of canvases: "+this.canvases.length                 ) ;
                                         for(var i=0; i<this.canvases.length; i++)
                                         {
                                          var theC = this.canvases[i] ;
                                          STDLINE(" name   : "+theC.canvasName                               ) ;
                                          STDLINE(" divs   : "+theC.nDivX+"x"+theC.nDivY                     ) ;
                                          STDLINE(" objects: "+theC.objects.length                           ) ;
                                          for( var j=0; j<theC.objects.length; j++)
                                          {
                                           STDLINE("   object " + j                      +
                                                   " title: "   + theC.objects[j].fTitle +
                                                   " name : "   + theC.objects[j].fName                      ) ;
                                          }
                                         }
                                         STDLINE("==========================================================") ;
                                        }
                       } ;

 //-----------------------------------------------------------------------------
 // Resize the div signed by id to width/height sizes
 function changeHistogramPanelSize(thisPanel, width, height, from)      
 {
  var ROOTObjects = theCanvasModel_.getROOTObjects(currentCanvas_) ;
  STDLINE("New canvas size: "       +
          width                     +
          "x"                       +
          height                    +
          " for "+ROOTObjects.length+
          " objects"                                             );
  var div = document.getElementById(getCanvasDiv_(currentCanvas_));
  div.style.width  = width  - 20                                  ;
  div.style.height = height - 30                                  ;
  for(var i=0; i<ROOTObjects.length; i++)
  {
   displayPlot_(currentCanvas_,ROOTObjects[i])                    ;
  }
 } 
             
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
 var nDivXCB = Ext.create(
                          'Ext.form.field.Number',
                          {
                           xtype     : 'numberfield'       ,  
                           name      : 'hzon'              ,
                           labelWidth: 50                  ,
                           flex      : 0                   ,
                           width     : 100                 ,
                           height    : 18                  ,
                           fieldLabel: 'PlotsX'            ,  
                           value     : 1                   ,  
                           minValue  : 1                   ,  
                           maxValue  : 20                  ,
                           style     : buttonStyle_        ,
                           listeners : {
                                        change: function( thisSpinner, newValue, oldValue, eOpts )
                                                {
                                                 theCanvasModel_.setnDivX(currentCanvas_, newValue);
                                                 currentPad_ = 0 ;
                                                },
                                        spin  : function( thisSpinner, newValue, oldValue, eOpts )
                                                {
                                                 JSROOT.cleanup(getCanvasDiv_(currentCanvas_))     ;
                                                 theCanvasModel_.setnDivX(currentCanvas_, newValue);
                                                 currentPad_ = 0 ;
                                                }
                                       }
                          }
                         ) ;
 //-----------------------------------------------------------------------------------------------
 var nDivYCB = Ext.create(
                          'Ext.form.field.Number',
                          {
                           xtype     : 'numberfield'       ,  
                           name      : 'vzon'              ,
                           labelWidth: 50                  ,
                           flex      : 0                   ,
                           width     : 100                 ,
                           height    : 18                  ,
                           fieldLabel: 'PlotsY'            ,  
                           value     : 1                   ,  
                           minValue  : 1                   ,  
                           maxValue  : 20                  ,
                           style     : buttonStyle_        ,      
                           listeners : {
                                        change: function( thisSpinner, newValue, oldValue, eOpts )
                                                {
                                                 theCanvasModel_.setnDivY(currentCanvas_, newValue);
                                                 currentPad_ = 0 ;
                                                },
                                        spin  : function( thisSpinner, newValue, oldValue, eOpts )
                                                {
                                                 JSROOT.cleanup(getCanvasDiv_(currentCanvas_))     ;
                                                 theCanvasModel_.setnDivY(currentCanvas_, newValue);
                                                 currentPad_ = 0 ;
                                                }
                                       }
                          }
                         ) ;
                          
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
                                   width       : 110                 ,
                                   minWidth    : 100                 ,
                                   maxWidth    : 400                 ,
                                   collapsible : true                ,
                                   collapsed   : true                ,
                                   animCollapse: true                ,
                                   margins     : '0 0 0 5'           ,
                                   layout      : 'accordion'         ,
                                   items       : [
                                                  {
                                                   title     : 'Canvas'                                      ,
                                                   autoScroll: true                                          ,
                                                   layout    : 'vbox'                                        ,
                                                   tooltip   : 'Canvas controls'                             ,
                                                   tools     : [
                                                                {
                                                                 type      : 'next'                          ,  
                                                                 tooltip   : 'Maximize canvas size'          ,
                                                                 handler   : function()
                                                                             {
                                                                              STDLINE("Collapsing") ;
                                                                              theNavigatorPanel_  .collapse() ; 
                                                                              ROOTControlsPanel_  .collapse() ; 
                                                                              theInformationPanel_.collapse() ; 
                                                                             }
                                                                }
                                                               ],
                                                   items     : [
                                                                {
                                                                 xtype     : 'button'                        ,
                                                                 text      : 'Add canvas'                    ,
                                                                 tooltip   : 'Add a new canvas'              ,
                                                                 width     : 100                             ,
                                                                 height    : 20                              ,
                                                                 pressed   : true                            ,
                                                                 border    : true                            ,
                                                                 style     : buttonStyle_                    , 
                                                                 handler   : function()
                                                                             {
                                                                              var addIndex   = globalCanvas_.items.length                      ;
                                                                              currentCanvas_ = addIndex  + 1                                   ;
                                                                              theCanvasModel_.addCanvas()                                      ;
                                                                              generateDIVPlaceholderSize(getCanvasDiv_(currentCanvas_),350,440);          
                                                                              changeHistogramPanelSize(
                                                                                                       'canvas'+currentCanvas_      , 
                                                                                                       theCanvasModel_.currentWidth , 
                                                                                                       theCanvasModel_.currentHeight, 
                                                                                                       "resized"
                                                                                                      ) ;
                                                                              globalCanvas_.insert(
                                                                                                   addIndex,
                                                                                                   {
                                                                                                    contentEl : getCanvasDiv_(currentCanvas_),
                                                                                                    title     : 'Canvas ' +  currentCanvas_  ,
                                                                                                    closable  : true                         ,
                                                                                                    border    : true                         ,
                                                                                                    autoScroll: true
                                                                                                   }
                                                                                                  );
                                                                              globalCanvas_.setActiveTab(addIndex)                            ;
                                                                              STDLINE("Adding new canvas tab")                                ;
                                                                             }
                                                                }, {
                                                                 xtype     : 'button'                                     ,
                                                                 text      : 'Clear canvas'                               ,
                                                                 pressed   : true                                         ,
                                                                 width     : 100                                          ,
                                                                 height    : 20                                           ,
                                                                 tooltip   : 'Clear the current canvas content and '      +
                                                                             'reset the list of displayed plots'          ,
                                                                 border    : true                                         ,
                                                                 style     : buttonStyle_                                 ,   
                                                                 handler   : function()  
                                                                             {
                                                                              JSROOT.cleanup(getCanvasDiv_(currentCanvas_));
                                                                              theCanvasModel_.clearCanvas (currentCanvas_) ;
                                                                              theCanvasModel_.dumpContent (currentCanvas_) ;
                                                                             }
                                                                }, {
                                                                 xtype     : 'button'                                     ,
                                                                 text      : 'Reset canvas'                               ,
                                                                 pressed   : true                                         ,
                                                                 width     : 100                                          ,
                                                                 height    : 20                                           ,
                                                                 tooltip   : 'Clear the canvas container in memory but '  +
                                                                             'not the canvas display'                     ,
                                                                 border    : true                                         ,
                                                                 style     : buttonStyle_                                 ,   
                                                                 handler   : function()  
                                                                             {
                                                                              JSROOT.cleanup(getCanvasDiv_(currentCanvas_));
                                                                              theCanvasModel_.clearCanvas (currentCanvas_) ;
                                                                              theCanvasModel_.dumpContent (currentCanvas_) ;
                                                                             }
                                                                }, {
                                                                 xtype     : 'button'                                     ,
                                                                 text      : 'Dump canvas'                                ,
                                                                 pressed   : true                                         ,
                                                                 width     : 100                                          ,
                                                                 height    : 20                                           ,
                                                                 tooltip   : 'Clear the current canvas content'           ,
                                                                 border    : true                                         ,
                                                                 style     : buttonStyle_                                 ,   
                                                                 handler   : function()  
                                                                             {
                                                                              theCanvasModel_.dumpContent (currentCanvas_) ;
                                                                             }
                                                                },                             
                                                                nDivXCB,
                                                                nDivYCB,
                                                                {
                                                                 defaultType: 'checkbox'                                   ,
                                                                 border     : false                                        ,
                                                                 style      : buttonStyle_                                 ,   
                                                                 items      : [
                                                                               {
                                                                                boxLabel  : 'Superimpose'                  ,
                                                                                name      : 'superimpose'                  ,
                                                                                inputValue: superimposeFlag_               ,
                                                                                id        : 'superimpose'                  ,
                                                                                checked   : superimposeFlag_               ,
                                                                                handler   : function(thisCheckbox,status)
                                                                                            {
                                                                                             superimposeFlag_ = status      ;
                                                                                             STDLINE("Superimpose: "+superimposeFlag_) ;
                                                                                            } 
                                                                               }
                                                                              ]
                                                                },{
                                                                 xtype     : 'button'                                     ,
                                                                 text      : 'Normalize'                                  ,
                                                                 pressed   : true                                         ,
                                                                 width     : 100                                          ,
                                                                 height    : 20                                           ,
                                                                 tooltip   : 'Set Y axis to highest number of entires '   +
                                                                             '(+10%) of selected histograms'              ,
                                                                 border    : true                                         ,
                                                                 style     : buttonStyle_                                 ,   
                                                                 handler   : function()  
                                                                             {
                                                                              STDLINE("Going to normalize")                ;
                                                                              theCanvasModel_.dumpContent (currentCanvas_) ;
//                                                                               theAjaxRequest(
//                                                                                              _requestURL+"RequestType=getNormalized",
//                                                                                              {                                          
//                                                                                               CookieCode: _cookieCode,                  
//                                                                                               RootPath  : currentRootObject_            
//                                                                                              }, 
//                                                                                              ""
//                                                                                             )                                                                              
                                                                             }
                                                                }
                                                               ]
                                                  }, {
                                                   title     : 'Timing'                                                   ,
                                                   html      : '<p>Controls to drive the filesystem drill down.</p>'      ,
                                                   autoScroll: true                                                       ,
                                                   padding   : '5 5 5 5'                                                  ,
                                                   iconCls   : 'info'                                                     ,
                                                   items     : [
                                                                {
                                                                 xtype     : 'button'                                     ,
                                                                 text      : 'Stop'                                       ,
                                                                 pressed   : true                                         ,
                                                                 style     : buttonStyle_                                 ,   
                                                                 tooltip   : 'Stop periodical refreshing of histograms'   ,
                                                                 border    : true
                                                                }
                                                               ]
                                                  }, {
                                                   title     : 'Navigator'                                                ,
                                                   autoScroll: true                                                       ,
                                                   padding   : '5 5 5 5'                                                  ,
                                                   iconCls   : 'info'                                                     ,
                                                   items     : [
                                                                {
                                                                 defaultType: 'checkbox'                                  ,
                                                                 border     : false                                       ,
                                                                 style      : buttonStyle_                                ,   
                                                                 items      : [
                                                                               {
                                                                                boxLabel  : 'Debugger'                    ,
                                                                                name      : 'debugger'                    ,
                                                                                inputValue: enableDebug_                  ,
                                                                                id        : 'debugger'                    ,
                                                                                checked   : enableDebug_                  ,
                                                                                handler   : function(thisCheckbox,status)
                                                                                            {
                                                                                             enableDebug_ = status         ;
                                                                                             enableSTDLINE(enableDebug_)   ;
                                                                                            } 
                                                                               }
                                                                              ]
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
                              region        : 'center'      ,
                              deferredRender: false         ,
                              activeTab     : 0             ,
                              items         : canvasTabs_   ,
                              listeners     : {
                                               tabchange : function( thisPanel, newCard, oldCard, eOpts ) 
                                                           {
                                                            clearCanvas_   = false                                     ;
                                                            var regex      = /\d+/g                                    ;
                                                            var str        = newCard.title                             ;
                                                            var matches    = str.match(regex)                          ;
                                                            currentCanvas_ = matches[0]                                ;
                                                            STDLINE("Changed tab to " + getCanvasDiv_(currentCanvas_)) ;
                                                            nDivXCB.setValue(theCanvasModel_.getnDivX(currentCanvas_)) ;
                                                            nDivYCB.setValue(theCanvasModel_.getnDivY(currentCanvas_)) ;
                                                            width  = theCanvasModel_.currentWidth                      ;
                                                            height = theCanvasModel_.currentHeight                     ;
                                                            changeHistogramPanelSize(
                                                                                     thisPanel                        ,
                                                                                     width                            ,
                                                                                     height                           ,
                                                                                     "resized"
                                                                                    ) ;
                                                           },
                                               resize    : function(thisPanel, width, height, oldWidth, oldHeight, eOpt)
                                                           {                                                           ;
                                                            STDLINE("Resizing "+getCanvasDiv_(currentCanvas_))         ;
                                                            theCanvasModel_.currentWidth  = width                      ;
                                                            theCanvasModel_.currentHeight = height                     ;
                                                            currentPad_ = 0                                            ;             
                                                            changeHistogramPanelSize(
                                                                                     thisPanel                        ,
                                                                                     width                            ,
                                                                                     height                           ,
                                                                                     "resized"
                                                                                    ) ;
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
                                   id          : 'west-panel'      ,
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
                                                   title     : 'FileSystem navigation'                            ,
                                                   id        : 'navigatorDiv'                                     ,
                                                   autoScroll: true                                               ,
                                                   tools     : [
                                                                {
                                                                 type   : 'left'                                  ,
                                                                 tooltip: 'Go back to list of folders and files'  ,
                                                                 handler: function()
                                                                          {
                                                                           if( currentTree_ = 'fileContent' )
                                                                           {
                                                                            selectedItem_ = "getDirectories"       ;
                                                                            makeStore(theProvenance_.getRootPath(), 
                                                                                      'RequestType=getMeDirs'    ) ;
                                                                            makeGrid (theProvenance_.getRootPath(), 
                                                                                      'Directories and files'    ) ;
                                                                           }
                                                                          }
                                                                },
                                                                {
                                                                 type   : 'prev'                                  ,
                                                                 tooltip: 'Maximize canvas size'                  ,
                                                                 handler: function()
                                                                          {
                                                                           STDLINE("Collapsing")                   ;
                                                                           theNavigatorPanel_  .collapse()         ;
                                                                           ROOTControlsPanel_  .collapse()         ;
                                                                           theInformationPanel_.collapse()         ;
                                                                          }
                                                                }
                                                               ]
                                                  }, {
                                                   title     : 'ROOT file navigation'                                  ,
                                                   html      : '<p>ROOT files with subfolders, plots and canvases.</p>',
                                                   autoScroll: true                                                    ,
                                                   iconCls   : 'settings'
                                                  }, {
                                                   title     : 'Navigation Controls'                                   ,
                                                   html      : '<p>Controls to drive the filesystem drill down.</p>'   ,
                                                   autoScroll: true                                                    ,
                                                   iconCls   : 'info'
                                                  }
                                                 ],
                                   listeners   : {
                                                  collapse   : function() 
                                                               {
                                                                STDLINE("Collapse!!!");
                                                               },
                                                  expand     : function() 
                                                               {
                                                                STDLINE("Expand!!!"  );
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
                                                          theProvenance_.setRootPath(record.data.dir)    ;
                                                          STDLINE("fRootPath_: "+theProvenance_.getRootPath());
                                                          selectedItem_ = "getDirectories"               ;
                                                          makeStore(theProvenance_.getRootPath(), 
                                                                    'RequestType=getMeDirs'    ) ; 
                                                          makeGrid (theProvenance_.getRootPath(), 
                                                                    'Directories and files'    ) ;
                                                         },
                                             focusleave: function (thisCombo) 
                                                         {
                                                          STDLINE('remove  selection listener')          ;
                                                          theSourcesCB_.suspendEvent('select')           ;
                                                          STDLINE('removed selection listener')          ;
                                                         },
                                             focusenter: function (thisCombo) 
                                                         {
                                                          STDLINE('reinstate  selection listener')       ;
                                                          thisCombo.resumeEvent('select')                ;
                                                          theSourcesCB_.resumeEvent('select')            ;
                                                          STDLINE('reinstated selection listener')       ;
                                                         }
                                            }
                             }
                            );
  STDLINE("Sources created...") ;
  theSourcesCB_.setRawValue(dirs[0].dir) ; // Set default value
  STDLINE("Sources populated" ) ;
 }
 //-----------------------------------------------------------------------------
 function makeGrid(where,what)
 { 
  STDLINE("makeGrid("+where+","+what+")") ;
  if( grid_ ) grid_.destroy()     ;
  theStore_.sort(treeDisplayField_, 'ASC');

  STDLINE("Creating grid") ;
  grid_ = Ext.create(
                     'Ext.tree.Panel', 
                     {
                      title      : what                  ,
                      header     : false                 ,
                      id         : 'navigator'           ,
                      store      : theStore_             ,
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
                                    expand    : function(expandedItem, options) 
                                                {
                                                 STDLINE("expanded") ;
                                                },                                   
                                    itemclick : function(thisItem, record, item, index, e, eOpts)
                                                {
                                                 var selection = this.getSelection()                             ;
                                                 STDLINE("Selected "+selection.length+" items")                  ;
                                                 for(var i=0; i<selection.length; i++)  
                                                 {  
                                                  theProvenance_.setSystemPath  (selection[i].data.fSystemPath  );
                                                  theProvenance_.setRootPath    (selection[i].data.fRootPath    );
                                                  theProvenance_.setFoldersPath (selection[i].data.fFoldersPath );
                                                  theProvenance_.setFileName    (selection[i].data.fFileName    );
                                                  theProvenance_.setRFoldersPath(selection[i].data.fRFoldersPath);
                                                  theProvenance_.setHistName    (selection[i].data.fHistName    );
                                                  theProvenance_.dump()                                          ;
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
                                                   currentDirectory_ = theProvenance_.getSystemPath()              +
                                                                       '/'                                         +
                                                                       theProvenance_.getRootPath()                +
                                                                       "/"                                         +
                                                                       theProvenance_.getFoldersPath()             +
                                                                       "/"                                         +
                                                                       theProvenance_.getFileName()                 ;
                                                   STDLINE('RequestType      : getMeRootFile'     )                 ;
                                                   theProvenance_.dump()                                            ;
                                                   STDLINE('currentDirectory_: '+currentDirectory_)                 ;
                                                   makeStore(currentDirectory_,'RequestType=getMeRootFile')         ;
                                                   makeGrid (currentDirectory_,'ROOT file content'        )         ;
                                                  }
                                                  else if( selectedItem_ == "getRootObject" )
                                                  { 
                                                   theProvenance_.dump()                                            ;
                                                   currentRootObject_  = "/"                                       +
                                                                         theProvenance_.getRootPath()              +
                                                                         "/"                                       +
                                                                         theProvenance_.getFoldersPath()           +
                                                                         theProvenance_.getFileName()              +
                                                                         "/"                                       +
                                                                         theProvenance_.getRFoldersPath()          +
                                                                         "/"                                       +
                                                                         theProvenance_.getHistName()               ;
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
  theProvenance_.dump()                  ;
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
                                                      "CookieCode"  : _cookieCode                    ,
                                                      "Path"        : path                           ,
                                                      "fRootPath"   : theProvenance_.getRootPath()   ,
                                                      "fFoldersPath": theProvenance_.getFoldersPath(),
                                                      "fHistName"   : theProvenance_.getHistName()   ,
                                                      "fFileName"   : theProvenance_.getFileName()
                                                     },
                                      url          : _requestURL + reqType,
                                      reader       : {
                                                      type          : 'xml'                          ,
                                                      root          : 'nodes'                        ,
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
                               var rootName  = getXMLValue (response,"path"    )                  ;                     
                               var rootJSON  = getXMLValue (response,"rootJSON")                  ;                 
                               var object    = JSROOT.parse(rootJSON           )                  ;
                               STDLINE("Launching displayPlot on currentCanvas_ "+currentCanvas_ );
                               theCanvasModel_.addROOTObject(currentCanvas_,object)               ;
                               displayPlot_(currentCanvas_,object)                                ;
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
 displayPlot_ = function(currentCanvas_,object)
                {
                 if( ! object ) return ;
                 var nx        = theCanvasModel_.getnDivX   (currentCanvas_)  ;
                 var ny        = theCanvasModel_.getnDivY   (currentCanvas_)  ;
                 gridDivision_ = "gridi" + nx + "x" + ny ;
                 mdi_ = new JSROOT.GridDisplay(getCanvasDiv_(currentCanvas_) , 
                                                             gridDivision_ )  ;
                 if (mdi_!=null) theFrame = mdi_.FindFrame(  currentPad_     , 
                                                             false           );
                 if( nx == 1 & ny == 1 ) 
                 {
                  theFrame = 'canvas' + currentCanvas_                        ;
                 } else {
                  theFrame = 'canvas' + currentCanvas_ + '_' + currentPad_    ;
                 }
                 STDLINE("Plotting "+object._typename+": "+object.fTitle)     ;
                 if( superimposeFlag_ )
                 {
                  STDLINE("Superimpose on "+theFrame+"...") ;
                  JSROOT.draw  (
                                theFrame,
                                object  ,
                                ""
                               ); 
                 } else {
                  STDLINE("Do NOT superimpose on "+theFrame+"...") ;
                  JSROOT.redraw(
                                theFrame,
                                object  ,
                                ""
                               ); 
                  currentPad_++ ; 
                  if( currentPad_ > nx * ny - 1) {currentPad_=0;}
                 }                                                                          
                }
 //-----------------------------------------------------------------------------
 //
 //    H e r e   b e g i n s  t h e  a c t i o n
 //
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

 STDLINE("Calling makeViewPort") ;
 makeViewPort() ;
});
