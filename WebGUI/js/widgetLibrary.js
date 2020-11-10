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
									"1" :"table.html",
									"2" :"gauge_widget.html",
							   		"3" :"plotly_bar_chart.html",
							   		"4" :"plotly_xy_chart.html",
									"5" :"thermometer.html",
									"6" :"siren_alarm.html",
									"7" :"label.html",
									"8" :"led.html",
									"9" :"picture.html",
									"10" :"2d_stop-light.html",
									"11" :"root_visualizer.html"
								}

widgetLibrary.typeToImgSrc =	{
									"0" :"txt-icon.png",
		   							"1" :"table-icon.png",
									"2" :"gauge-icon.png",
		   							"3" :"basic-chart-icon.png",
		   							"4" :"basic-xy-chart-icon.png",
		   					 		"5" :"thermometer-icon.png" ,
		   					 		"6" :"red_siren.png",
		   					 		"7" :"label-icon.png",
		   					 		"8" :"led-icon.png",
		   					 		"9" :"picture-icon.png",
		   					 		"10" :"2d_stop-light.png",
		   					 		"11" :"root-icon.png"
								}

widgetLibrary.typeToName =		{
									"0" : "Plain Text",
									"1" : "Basic PVs Table",
									"2" : "Basic Gauge Style Display",
									"3" : "Basic Time Line Chart",
									"4" : "Basic XY Line Chart",
									"5" : "Basic Thermometer",
									"6" : "Basic Siren Alarm",
									"7" : "Basic Label for widgets",
									"8" : "Basic Led",
									"9" : "Basic picture",
									"10" : "Basic 2D stop-light viewer",
									"11" : "Basic Root file widget"
								}

widgetLibrary.typeToPurpose =	{
									"0" : "Plain Text",
									"1" : "PVs Table",
									"2" : "Gauge",
									"3" : "Time Chart",
									"4" : "XY Chart",
									"5" : "Thermometer",
									"6" : "Siren Alarm",
									"7" : "Label widgets",
									"8" : "Led",
									"9" : "Picture",
									"10" : "2D stop-light",
									"11" : "Root file"
								}

widgetLibrary.dimensions =		{
									"3": {
										"X": "350",
										"Y": "200",
										"Scale": "True"
									},
									"4": {
										"X": "350",
										"Y": "200",
										"Scale": "True"
									},
									"1": {
										"X": "400",
										"Y": "200",
										"Scale": "True"
									},
									"10": {
										"X": "420",
										"Y": "350",
										"Scale": "True"
									}
								}

widgetLibrary.init = function()
{
	
}
