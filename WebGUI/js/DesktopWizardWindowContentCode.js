//=====================================================================================
//
//	Created Jan, 2013
//	by Dan parilla ((parilla at fnal.gov))
//
//	DesktopWizardWindowContentCode.js
//
//  Requirements in <head>: 
//   1. paste the following: 
//				
//				<script type="text/JavaScript" src="/WebPath/js/DesktopWizardWindowContentCode.js"></script>	
//
//
//=====================================================================================

var DesktopWizardContent = DesktopWizardContent || {}; //define DesktopWizardContent namespace

if (typeof Debug == 'undefined') 
	alert('ERROR: Debug is undefined! Must include Debug.js before DesktopWizardWindowContentCode.js');
if (typeof Globals == 'undefined') 
	alert('ERROR: Globals is undefined! Must include Globals.js before DesktopWizardWindowContentCode.js');
	
//=====================================================================================
// Gets the unique sequence from the url
DesktopWizardContent.getSequence = function() {

	return (window.parent.parent.window.location.pathname.split("/")[2]);
}


	
	