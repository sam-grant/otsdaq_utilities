#!/bin/bash
#
# This script is expected to be in otsdaq utilities repository in a specific directory
# but it can be executed from any path (do not source it, execute with ./ )
#
# ./path/to/script/OnlineDocPushUpdate.sh <do NOT do mrb z> <only transfer main page>
#
echo 
echo
echo -e "OnlineDoc [${LINENO}]  \t =================="
echo -e "OnlineDoc [${LINENO}]  \t Starting online doc push..."

if ! [ -e setup_ots.sh ]; then
	echo -e "OnlineDoc [${LINENO}]  \t You must run this script from an OTSDAQ installation directory!"
  exit 1
fi

CURRENT_AWESOME_BASE=$PWD
CHECKIN_LOG_PATH=$CURRENT_AWESOME_BASE/.checkinAll.log
UPDATE_LOG_PATH=$CURRENT_AWESOME_BASE/.updateAll.log

echo 
echo
echo -e "OnlineDoc [${LINENO}]  \t Note: Your shell must be bash. If you received 'Expression Syntax' errors, please type 'bash' to switch."
echo -e "OnlineDoc [${LINENO}]  \t You are using $0"
echo
echo


SCRIPT_DIR="$( 
  cd "$(dirname "$(readlink "$0" || printf %s "$0")")"
  pwd -P 
)"
		
echo -e "OnlineDoc [${LINENO}]  \t Script directory found as: $SCRIPT_DIR"


#######################################################################################################################
# regenerate documentation

echo
echo -e "OnlineDoc [${LINENO}]  \t =================="
DO_MRBZ=0
if [ "x$1" == "x" ]; then    
	DO_MRBZ=1
else
	echo -e "OnlineDoc [${LINENO}]  \t Skipping mrb z and regeneration of documentation."
fi
ONLY_MAIN=1
if [ "x$2" == "x" ]; then    
	ONLY_MAIN=0
else
	echo -e "OnlineDoc [${LINENO}]  \t Only regenerating and updating main.html"
fi

if [ $DO_MRBZ == 1 ]; then
	echo -e "OnlineDoc [${LINENO}]  \t Cleaning all so that doxygen will run... mrb z..."
	source setup_ots.sh
	mrb z
	source mrbSetEnv
	mrb b
fi

#exit #for debugging

#######################################################################################################################
# transfer documentation

echo
echo -e "OnlineDoc [${LINENO}]  \t =================="

echo -e "OnlineDoc [${LINENO}]  \t Deleting current web documentation..."

if [ $ONLY_MAIN == 0 ]; then
	ssh web-otsdaq@otsdaq.fnal.gov /web/sites/otsdaq.fnal.gov/data/deleteCodeNav.sh
fi


echo -e "OnlineDoc [${LINENO}]  \t Finding paths and transferring doxygen html..."


REPO_DIR="$(find $SCRIPT_DIR/../../../build_slf6.x86_64 -maxdepth 1 -iname 'otsdaq*')"


for p in ${REPO_DIR[@]}; do
	if [ -d $p ]; then    
	if [ -d $p/doc/html ]; then
		echo -e "OnlineDoc [${LINENO}]  \t Doc directory found as: $(basename $p)"
	fi
	fi
done
				
for p in ${REPO_DIR[@]}; do
	if [ -d $p ]; then    
	if [ -d $p/doc/html ]; then
		echo -e "OnlineDoc [${LINENO}]  \t Handling directory: $(basename $p)"

		echo -e "OnlineDoc [${LINENO}]  \t Injecting main html..."

		if [ $DO_MRBZ == 1 ]; then #only backup when generated
			cp $p/doc/html/main.html $p/doc/html/main.html.bk
		fi
		doxygen_main_editor $p/doc/html/main.html $SCRIPT_DIR/../../../srcs/otsdaq_utilities/onlineDoc/inject_$(basename $p).html $SCRIPT_DIR/../../../srcs/otsdaq_utilities/onlineDoc/inject_$(basename $p)_head.html

		if [ $ONLY_MAIN == 1 ]; then
			scp -r $p/doc/html/main.html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs/docs/code/$(basename $p)/			
			continue
		fi

		echo
		echo -e "OnlineDoc [${LINENO}]  \t =================="
		echo -e "OnlineDoc [${LINENO}]  \t Transferring content..."
		echo
		echo
		
		echo -e "OnlineDoc [${LINENO}]  \t scp -r $p/doc/html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs/docs/code/$(basename $p)"
		scp -r $p/doc/html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs/docs/code/$(basename $p)

		echo
		echo -e "OnlineDoc [${LINENO}]  \t =================="
		echo -e "OnlineDoc [${LINENO}]  \t Refining content..."
		echo
		echo
		
		echo -e "OnlineDoc [${LINENO}]  \t scp -r $SCRIPT_DIR/../../../srcs/otsdaq_utilities/onlineDoc/doxygen_index.html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs/docs/code/$(basename $p)/index.html"
		scp -r $SCRIPT_DIR/../../../srcs/otsdaq_utilities/onlineDoc/doxygen_index.html web-otsdaq@otsdaq.fnal.gov:/web/sites/otsdaq.fnal.gov/htdocs/docs/code/$(basename $p)/index.html

	fi
    fi	   
done




echo
echo -e "OnlineDoc [${LINENO}]  \t =================="
echo
echo -e "OnlineDoc [${LINENO}]  \t =================="
echo -e "OnlineDoc [${LINENO}]  \t Online documentation update done"
echo -e "OnlineDoc [${LINENO}]  \t *******************************"
echo -e "OnlineDoc [${LINENO}]  \t *******************************"





















