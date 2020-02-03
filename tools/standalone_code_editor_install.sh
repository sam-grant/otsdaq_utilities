#!/bin/bash
#
# This file can be downloaded here:
#
# 	wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/tools/standalone_code_editor_install.sh
#
#
# This script is expected to be sourced. It will
# create the folder structure and download
# the necessary files for the otsdaq stand-alone Code Editor.
#
# Copyright January 2020,  rrivera at fnal dot gov
#
# source standalone_code_editor_install.sh



echo -e "standalone_code_editor_install.sh [${LINENO}]  "
echo -e "standalone_code_editor_install.sh [${LINENO}]  \t ~~ Code Editor stand-alone installation ~~ "
echo -e "standalone_code_editor_install.sh [${LINENO}]  "
echo -e "standalone_code_editor_install.sh [${LINENO}]  "

mkdir otsCodeEditor
mkdir otsCodeEditor/html
mkdir otsCodeEditor/css
mkdir otsCodeEditor/js

wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/html/CodeEditor_standalone.html -P otsCodeEditor/html/ --no-check-certificate || curl -o  otsCodeEditor/html/CodeEditor_standalone.html https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/html/CodeEditor_standalone.html  	

wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/css/ots.css -P otsCodeEditor/css/ --no-check-certificate	 || curl -o  otsCodeEditor/css/ots.css https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/css/ots.css 
wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/css/MultiSelectBox.css -P otsCodeEditor/css/ --no-check-certificate || curl -o  otsCodeEditor/css/MultiSelectBox.css https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/css/MultiSelectBox.css	
wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/css/CodeEditor.css -P otsCodeEditor/css/ --no-check-certificate	|| curl -o  otsCodeEditor/css/CodeEditor.css https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/css/CodeEditor.css

wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/js/Globals.js -P otsCodeEditor/js/ --no-check-certificate	|| curl -o  otsCodeEditor/js/Globals.js https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/js/Globals.js
wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/js/Debug.js -P otsCodeEditor/js/ --no-check-certificate		|| curl -o  otsCodeEditor/js/Debug.js https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/js/Debug.js
wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/js/DesktopWindowContentCode.js -P otsCodeEditor/js/ --no-check-certificate		|| curl -o  otsCodeEditor/js/DesktopWindowContentCode.js https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/js/DesktopWindowContentCode.js
wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/js/CodeEditor.js -P otsCodeEditor/js/ --no-check-certificate	|| curl -o  otsCodeEditor/js/CodeEditor.js https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/js/CodeEditor.js
wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/js/js_lib/MultiSelectBox.js -P otsCodeEditor/js/ --no-check-certificate		|| curl -o  otsCodeEditor/js/MultiSelectBox.js https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/WebGUI/js/MultiSelectBox.js


echo -e "standalone_code_editor_install.sh [${LINENO}]  \t =================="
echo -e "standalone_code_editor_install.sh [${LINENO}]  \t Code Editor stand-alone installation done"
echo -e "standalone_code_editor_install.sh [${LINENO}]  \t "
echo -e "standalone_code_editor_install.sh [${LINENO}]  \t Now open this file in your browser: otsCodeEditor/html/CodeEditor_standalone.html"
echo -e "standalone_code_editor_install.sh [${LINENO}]  \t "
echo -e "standalone_code_editor_install.sh [${LINENO}]  \t *******************************"
echo -e "standalone_code_editor_install.sh [${LINENO}]  \t *******************************"

		
		



