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
									"4" :"thermometer.html",
									"5" :"siren_alarm.html",
									"6" :"label.html",
									"7" :"led.html",
									"8" :"picture.html",
									"9" :"root_visualizer.html",
									"10" :"2d_stop-light.html"
								}

widgetLibrary.typeToImgSrc =	{
									"0" :"txt-icon.png",
		   							"1" :"table-icon.png",
									"2" :"gauge-icon.png",
		   							"3" :"basic-chart-icon.png",
		   					 		"4" :"thermometer-icon.png" ,
		   					 		"5" :"red_siren.png",
		   					 		"6" :"label-icon.png",
		   					 		"7" :"led-icon.png",
		   					 		"8" :"picture-icon.png",
		   					 		"9" :"root-icon.png",
		   					 		"10" :"2d_stop-light.png"
								}

widgetLibrary.typeToName =		{
									"0" : "Plain Text",
									"1" : "Basic PVs Table",
									"2" : "Basic Gauge Style Display",
									"3" : "Basic Line Chart",
									"4" : "Basic Thermometer",
									"5" : "Basic Siren Alarm",
									"6" : "Basic Label for widgets",
									"7" : "Basic Led",
									"8" : "Basic picture",
									"9" : "Basic Root file viewer",
									"10" : "Basic 2D stop-light widget"
								}

widgetLibrary.typeToPurpose =	{
									"0" : "Plain Text",
									"1" : "PVs Table",
									"2" : "Gauge",
									"3" : "Chart",
									"4" : "Thermometer",
									"5" : "Siren Alarm",
									"6" : "Label widgets",
									"7" : "Led",
									"8" : "Picture",
									"9" : "Root file",
									"10" : "2D stop-light"
								}

widgetLibrary.dimensions =		{
									"3": {
										"X": "300",
										"Y": "200",
										"Scale": "True"
									},
									"8": {
										"X": "300",
										"Y": "300",
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
