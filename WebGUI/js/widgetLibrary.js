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
widgetLibrary.typeToSrc =		{
									"0" :"widget_text.html",
									//"1" :"widget_basicDisplay.html",
									//"1" :"timeseries_widget.html",
									"1" :"widget_text_with_title.html",
							   		"2" :"gauge_widget.html",
							   		//"3" :"simple_bar_chart.html",
							   		"3" :"plotly_bar_chart.html",
									"4" :"thermometer.html",
									"5" :"siren_alarm.html",
									"6" :"label.html"
								}

widgetLibrary.typeToImgSrc =	{
									"0" :"txt-icon.png",
							 		//"1" :"digital-clock-icon.png",
		   							"1" :"txt-icon.png",
									"2" :"gauge-icon.png",
		   							"3" :"basic-chart-icon.png",
		   					 		"4" :"thermometer-icon.png" ,
		   					 		"5" :"red_siren.png",
		   					 		"6" :"label-icon.png"		   
								}

widgetLibrary.typeToName =		{
									"0" : "Plain Text",
									//"1" : "Basic Display as a digital clock",
									//"1" : "Basic Timeseries Display",
									"1" : "Basic Display with name & value displays",
									"2" : "Basic Gauge Style Display",
									"3" : "Basic Line Chart",
									"4" : "Basic Thermometer",
									"5" : "Basic Siren Alarm",
									"6" : "Basic Label for widgets"
								}

widgetLibrary.typeToPurpose =	{
									"0" : "Plain Text",
									//"1" : "Timeseries",
									"1" : "Plain Text",
									"2" : "Gauge",
									"3" : "Chart",
									"4" : "Thermometer",
									"5" : "Siren Alarm",
									"6" : "Label widgets"
								}

widgetLibrary.dimensions =		{
									"0": {
										"X": "100",
										"Y": "300",
										"Scale": "False"
									},
									"3": {
										"X": "300",
										"Y": "200",
										"Scale": "True"
									},
									"8": {
										"X": "300",
										"Y": "300",
										"Scale": "True"
									}
								}

widgetLibrary.init = function()
{
	
}
