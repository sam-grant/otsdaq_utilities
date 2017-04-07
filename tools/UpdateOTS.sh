# This script is expected to be in otsdaq utilities repository in a specific directory
# but it can be executed from any path (do not source it, execute with ./ )
#
# ./path/to/script/checkinAll.sh "comment for git commit"
#
# If no comment is given, the script will only pull updates - it will not checkin.
#

#/bin/bash

CURRENT_AWESOME_BASE=$PWD

echo 
echo
echo "Note: Your shell must be bash. If you received 'Expression Syntax' errors, please type 'bash' to switch."
echo "You are using $0"
echo
echo


if [ "x$1" == "x" ]; then
    echo "Usage Error: parameter 1 is the comment for git commit"
	echo "Note: to use ! at the end of your message put a space between the ! and the closing \""
    echo "Note: git status will be logged here: $CURRENT_AWESOME_BASE/checkinAll.log"
    echo "WARNING: without comment, script will only do git pull and git status"
fi

echo
echo
echo "Finding paths..."

SCRIPT_DIR="$( 
  cd "$(dirname "$(readlink "$0" || printf %s "$0")")"
  pwd -P 
)"
		
echo "Script directory found as: $SCRIPT_DIR"

#REPO_DIR="$(find $SCRIPT_DIR/../../ -maxdepth 1 -iname 'otsdaq*')" #old way before moving script to tools, and allowing compiling with CMake 
REPO_DIR="$(find $SCRIPT_DIR/../../../srcs -maxdepth 1 -iname 'otsdaq*')"

for p in ${REPO_DIR[@]}; do
    if [ -d $p ]; then
	echo "Repo directory found as: $(basename $p)"
    fi	   
done


#handle manual updates that should take place ONLY if it is UPDATING not committing
if [ "x$1" == "x" ]; then

	echo
	echo "#######################################################################################################################" 
	echo "#######################################################################################################################" 
	echo "Updating USER_DATA path $USER_DATA,"
	echo "based on the list in $USER_DATA/ServiceData/CoreTableInfoNames.dat."
	echo "If CoreTableInfoNames.dat doesn't exists the whole directory $OTSDAQ_DIR/data-core/ConfigurationInfo/ will be copied!"
	echo "#######################################################################################################################"
	echo "#######################################################################################################################"
	echo
	
	if [ -e "$USER_DATA/ServiceData/CoreTableInfoNames.dat" ]; then
		echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ConfigurationInfo.xsd $USER_DATA/ConfigurationInfo/"
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ConfigurationInfo.xsd $USER_DATA/ConfigurationInfo/
		while read line; do    
			echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/${line}Info.xml $USER_DATA/ConfigurationInfo/"
			cp $OTSDAQ_DIR/data-core/ConfigurationInfo/${line}Info.xml $USER_DATA/ConfigurationInfo/
		done < $USER_DATA/ServiceData/CoreTableInfoNames.dat
	else
		echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/* $USER_DATA/ConfigurationInfo/"
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/* $USER_DATA/ConfigurationInfo/
	fi
	
	echo "cp $OTSDAQ_DIR/data-core/XDAQConfigurations/otsConfigurationNoRU_Wizard_CMake.xml $USER_DATA/XDAQConfigurations/"
	cp $OTSDAQ_DIR/data-core/XDAQConfigurations/otsConfigurationNoRU_Wizard_CMake.xml $USER_DATA/XDAQConfigurations/

fi

echo
echo "=================="

echo "Git comment '$1'"
echo "Status will be logged here: $CURRENT_AWESOME_BASE/checkinAll.log"


echo
echo "=================="

echo "log start:" > $CURRENT_AWESOME_BASE/checkinAll.log
for p in ${REPO_DIR[@]}; do
    if [ -d $p ]; then
	echo "Checking in $p"
	cd $p
	git pull
	echo "==================" >> $CURRENT_AWESOME_BASE/checkinAll.log
	pwd >> $CURRENT_AWESOME_BASE/checkinAll.log
	git status &>> $CURRENT_AWESOME_BASE/checkinAll.log
	
	if [ "x$1" != "x" ]; then
		#add space for user
	    git commit -m "$1 " .  &>> $CURRENT_AWESOME_BASE/checkinAll.log
	    git push   
	fi

	cd $CURRENT_AWESOME_BASE
	echo
	echo "=================="

    fi	   
done



echo
echo "=================="

echo "Git comment '$1'"
echo "Status was logged here: $CURRENT_AWESOME_BASE/checkinAll.log"
echo
echo "log dump in 2 seconds... #######################################################"
sleep 2s
echo
cat $CURRENT_AWESOME_BASE/checkinAll.log
echo "end log dump... #######################################################"
echo "Status was logged here: $CURRENT_AWESOME_BASE/checkinAll.log"

echo
echo "=================="
echo
echo "=================="
echo "Awesome script done"
echo "*******************************"
echo "*******************************"