//=====================================================================================
//
//	Created Aug, 2016
//	by Daniel Parilla ((parilla at fnal.gov))
//
//	OtsConfigurationWizard.js
//
//
//
//
//
//
//=====================================================================================
var widgetLibrary = widgetLibrary || {};
widgetLibrary.typeToSrc = {			   "0" :"widget_text.html",
						   "1" :"widget_basicDisplay.html",
						   "2" :"widget_text_with_title.html",
						   "3" :"guage_widget.html",
						   "4" :"simple_bar_chart.html",
						   "5" :"thermometer.html"/*,
						   "3" :"widgets",
						   "4" :"widgets",						   
						   "5" :"widgets/thermometer.html",						   
						   "6" :"widgets",						   
						   "7" :"widgets",						   
						   "8" :"widgets",						   
						   "9" :"widgets",						   
						   "10":"widgets",						   
						   "11":"widgets",						   
						   "12":"widgets"	*/					   
						  }
widgetLibrary.typeToImgSrc =  {
							   "0" :"label-icon.png",
							   "1" :"digital-clock-icon.png",
		   					   "2" :"label-icon.png",
		   					   "3" :"guage-icon.png",
		   					   "4" :"basic-chart-icon.png",
		   					   "5" :"thermometer-icon.png" /*,
							   "3" :"Alarm-clock-icon.png",
							   "4" :"digital-clock-icon.png",						   
							   "5" :"thermometer-icon.png"		*/		   
							  }
widgetLibrary.typeToName = {
                                                                "0" : "Plain Text",
                                                                "2" : "Basic Display with name and value displays",
                                                                "3" : "Basic Guage Style Display",
                                                                "4" : "Basic Line Chart",
                                                                "5" : "Basic Thermometer"/*,
                                                                "2" : "Hakeem's Widet",
                                                                "3" : "Clock",
                                                                "4" : "Clock",
                                                                "5" : "Temperature"*/
                                                                }
widgetLibrary.typeToPurpose = {
								"0" : "Plain Text",
								"2" : "Plain Text",
								"3" : "Guage",
								"4" : "Chart",
								"5" : "Thermometer"/*,
								"2" : "Hakeem's Widet",
								"3" : "Clock",
								"4" : "Clock",
								"5" : "Temperature"*/
								}

widgetLibrary.dimensions = {
								"0": {
									"X": "100",
									"Y": "300",
									"Scale": "False"
								},
								"4": {
									"X": "300",
									"Y": "200",
									"Scale": "True"
								},
								"7": {
									"X": "300",
									"Y": "300",
									"Scale": "True"
								}
}
								
								
widgetLibrary.init = function()
{
	
}
