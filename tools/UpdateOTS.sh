#!/bin/bash
#
# This script is expected to be in otsdaq utilities repository in a specific directory
# but it can be executed from any path (do not source it, execute with ./ )
#
# ./path/to/script/UpdateOTS.sh "comment for git commit"
#
# If no comment is given, the script will only pull updates - it will not checkin.
#
# Note: it will be compiled by mrb so that no path is required:
#
# UpdateOTS.sh "comment for git commit"
#



CURRENT_AWESOME_BASE=$PWD
CHECKIN_LOG_PATH=$CURRENT_AWESOME_BASE/.checkinAll.log
UPDATE_LOG_PATH=$CURRENT_AWESOME_BASE/.updateAll.log

echo 
echo
echo -e "UpdateOTS.sh [${LINENO}]  \t Note: Your shell must be bash. If you received 'Expression Syntax' errors, please type 'bash' to switch."
echo -e "UpdateOTS.sh [${LINENO}]  \t You are using $0"
echo
echo


if [ "x$1" == "x" ]; then
    echo -e "UpdateOTS.sh [${LINENO}]  \t Usage Error: parameter 1 is the comment for git commit"
	echo -e "UpdateOTS.sh [${LINENO}]  \t Note: to use ! at the end of your message put a space between the ! and the closing \""
    echo -e "UpdateOTS.sh [${LINENO}]  \t Note: git status will be logged here: $CHECKIN_LOG_PATH"
    echo -e "UpdateOTS.sh [${LINENO}]  \t WARNING: without comment, script will only do git pull and git status"
fi

echo
echo
echo -e "UpdateOTS.sh [${LINENO}]  \t Finding paths..."

SCRIPT_DIR="$( 
  cd "$(dirname "$(readlink "$0" || printf %s "$0")")"
  pwd -P 
)"
		
echo -e "UpdateOTS.sh [${LINENO}]  \t Script directory found as: $SCRIPT_DIR"

#REPO_DIR="$(find $SCRIPT_DIR/../../ -maxdepth 1 -iname 'otsdaq*')" #old way before moving script to tools, and allowing compiling with CMake 
REPO_DIR="$(find $SCRIPT_DIR/../../../srcs -maxdepth 1 -iname 'otsdaq*')"

for p in ${REPO_DIR[@]}; do
    if [ -d $p ]; then
    if [ -d $p/.git ]; then
		echo -e "UpdateOTS.sh [${LINENO}]  \t Repo directory found as: $(basename $p)"
	fi
    fi	   
done



 
#######################################################################################################################

echo
echo -e "UpdateOTS.sh [${LINENO}]  \t =================="

echo -e "UpdateOTS.sh [${LINENO}]  \t Git comment '$1'"
echo -e "UpdateOTS.sh [${LINENO}]  \t Status will be logged here: $CHECKIN_LOG_PATH"


echo
echo -e "UpdateOTS.sh [${LINENO}]  \t =================="

echo -e "UpdateOTS.sh [${LINENO}]  \t log start:" > $CHECKIN_LOG_PATH
for p in ${REPO_DIR[@]}; do
    if [ -d $p ]; then
    if [ -d $p/.git ]; then
	echo -e "UpdateOTS.sh [${LINENO}]  \t Pulling updates from $p"
	cd $p
	git pull
	echo -e "UpdateOTS.sh [${LINENO}]  \t ==================" >> $CHECKIN_LOG_PATH
	pwd >> $CHECKIN_LOG_PATH
	git status &>> $CHECKIN_LOG_PATH
	
	if [ "x$1" != "x" ]; then
		
		echo -e "UpdateOTS.sh [${LINENO}]  \t Checking in $p"
	    git commit -m "$1 " .  &>> $CHECKIN_LOG_PATH  #add space in comment for user
	    git push   
	fi

	cd $CURRENT_AWESOME_BASE
	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t =================="

    fi	   
    fi	   
done



echo
echo -e "UpdateOTS.sh [${LINENO}]  \t =================="


#######################################################################################################################
#handle manual updates that should take place ONLY if it is UPDATING not committing
if [ "x$1" == "x" ]; then

	echo -e "UpdateOTS.sh [${LINENO}]  \t Update status will be logged here: $UPDATE_LOG_PATH"
	echo -e "UpdateOTS.sh [${LINENO}]  \t Update log start:" > $UPDATE_LOG_PATH
	
	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################" 
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################" 
	echo -e "UpdateOTS.sh [${LINENO}]  \t Updating USER_DATA path $USER_DATA,"
	echo -e "UpdateOTS.sh [${LINENO}]  \t based on the list in $USER_DATA/ServiceData/CoreTableInfoNames.dat."
	echo -e "UpdateOTS.sh [${LINENO}]  \t If CoreTableInfoNames.dat doesn't exist the whole directory $OTSDAQ_DIR/data-core/ConfigurationInfo/ will be copied!"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo
	
	echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ConfigurationInfo.xsd $USER_DATA/ConfigurationInfo/"
	cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ConfigurationInfo.xsd $USER_DATA/ConfigurationInfo/
			
	if [ -e "$USER_DATA/ServiceData/CoreTableInfoNames.dat" ]; then
		echo -e "UpdateOTS.sh [${LINENO}]  \t $USER_DATA/ServiceData/CoreTableInfoNames.dat exists!"
		echo -e "UpdateOTS.sh [${LINENO}]  \t Loading updated info for core tables (relative paths and wildcards are allowed) from $OTSDAQ_DIR/data-core/ConfigurationInfo/ ..."
		echo
		cat $USER_DATA/ServiceData/CoreTableInfoNames.dat
		echo
		
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp -r $USER_DATA/ConfigurationInfo $USER_DATA/ConfigurationInfo.updateots.bk"
		rm -rf $USER_DATA/ConfigurationInfo.updateots.bk
		cp -r $USER_DATA/ConfigurationInfo $USER_DATA/ConfigurationInfo.updateots.bk		
		
		#NOTE: relative paths are allowed from otsdaq/data-core/ConfigurationInfo
		while read line; do
			#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
			cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/${line}Info.xml $USER_DATA/ConfigurationInfo/	&>/dev/null #hide output	
			#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
			cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/${line}Info.xml $USER_DATA/ConfigurationInfo/ &>/dev/null #hide output		
			#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/ConfigurationInfo/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
			cp $OTSDAQ_DIR/data-core/ConfigurationInfo/${line}Info.xml $USER_DATA/ConfigurationInfo/ &>/dev/null #hide output		
		done < $USER_DATA/ServiceData/CoreTableInfoNames.dat
		
		#do one more time after loop to make sure last line is read (even if user did not put new line) 
		#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/${line}Info.xml $USER_DATA/ConfigurationInfo/ &>/dev/null #hide output		
		#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/${line}Info.xml $USER_DATA/ConfigurationInfo/ &>/dev/null #hide output		
		#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/ConfigurationInfo/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/${line}Info.xml $USER_DATA/ConfigurationInfo/ &>/dev/null #hide output
	else
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp -r $USER_DATA/ConfigurationInfo $USER_DATA/ConfigurationInfo_update_bk"
		rm -rf $USER_DATA/ConfigurationInfo_update_bk
		cp -r $USER_DATA/ConfigurationInfo/ $USER_DATA/ConfigurationInfo_update_bk
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/ConfigurationInfo/*Info.xml $USER_DATA/ConfigurationInfo/"
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/*Info.xml $USER_DATA/ConfigurationInfo/
		# undo c++ style comment for Eclipse viewing*/
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/*Info.xml $USER_DATA/ConfigurationInfo/"
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/*Info.xml $USER_DATA/ConfigurationInfo/
		# undo c++ style comment for Eclipse viewing*/
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/*Info.xml $USER_DATA/ConfigurationInfo/"
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/*Info.xml $USER_DATA/ConfigurationInfo/
		# undo c++ style comment for Eclipse viewing*/
	fi
	
	echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/XDAQConfigurations/otsConfigurationNoRU_Wizard_CMake.xml $USER_DATA/XDAQConfigurations/"
	cp $OTSDAQ_DIR/data-core/XDAQConfigurations/otsConfigurationNoRU_Wizard_CMake.xml $USER_DATA/XDAQConfigurations/
	
	#make sure permissions are usable
	echo -e "UpdateOTS.sh [${LINENO}]  \t chmod 777 $USER_DATA/XDAQConfigurations/*.xml"
	chmod 777 $USER_DATA/XDAQConfigurations/*.xml #*/ just resetting comment coloring
	echo -e "UpdateOTS.sh [${LINENO}]  \t chmod 755 $USER_DATA/ConfigurationInfo/*Info.xml"
	chmod 755 $USER_DATA/ConfigurationInfo/*Info.xml #*/ just resetting comment coloring
	echo -e "UpdateOTS.sh [${LINENO}]  \t chmod 755 $USER_DATA/ConfigurationInfo/*Info.xsd"
	chmod 755 $USER_DATA/ConfigurationInfo/*Info.xsd #*/ just resetting comment coloring
	
	#copy tutorial launching scripts
	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t updating tutorial launch scripts..."
	rm $OTSDAQ_DIR/../../get_tutorial_data.sh &>/dev/null 2>&1 #hide output
	rm $OTSDAQ_DIR/../../get_tutorial_database.sh &>/dev/null 2>&1 #hide output
	rm $OTSDAQ_DIR/../../reset_ots_tutorial.sh &>/dev/null 2>&1 #hide output
	#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_tutorial.sh $OTSDAQ_DIR/../../reset_ots_tutorial.sh"
	#cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_tutorial.sh $OTSDAQ_DIR/../../reset_ots_tutorial.sh
	wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/demo/revisions/develop/raw/tools/reset_ots_tutorial.sh -P $OTSDAQ_DIR/../../	
	chmod 755 $OTSDAQ_DIR/../../reset_ots_tutorial.sh
	
	rm $OTSDAQ_DIR/../../reset_ots_artdaq_tutorial.sh
	#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_artdaq_tutorial.sh $OTSDAQ_DIR/../../reset_ots_artdaq_tutorial.sh"
	#cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_artdaq_tutorial.sh $OTSDAQ_DIR/../../reset_ots_artdaq_tutorial.sh
	#wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/demo/revisions/develop/raw/tools/reset_ots_artdaq_tutorial.sh -P $OTSDAQ_DIR/../../	
	#chmod 755 $OTSDAQ_DIR/../../reset_ots_artdaq_tutorial.sh
	
	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	#end copy tables
	
	
	

	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################" 
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################" 
	echo -e "UpdateOTS.sh [${LINENO}]  \t Updating installed Repositories,"
	echo -e "UpdateOTS.sh [${LINENO}]  \t based on the list in $USER_DATA/ServiceData/InstalledRepoNames.dat."
	echo -e "UpdateOTS.sh [${LINENO}]  \t If InstalledRepoNames.dat doesn't exist, then nothing happens"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo
	
	if [ -e "$USER_DATA/ServiceData/InstalledRepoNames.dat" ]; then
		echo -e "UpdateOTS.sh [${LINENO}]  \t $USER_DATA/ServiceData/InstalledRepoNames.dat exists!"
		echo -e "UpdateOTS.sh [${LINENO}]  \t Loading list of repos to update..."
		cat $USER_DATA/ServiceData/InstalledRepoNames.dat
		echo
			
	
		#NOTE: relative paths are allowed from otsdaq/../
		while read line; do
			echo
			echo -e "UpdateOTS.sh [${LINENO}]  \t updating ${line} repository...."
			echo -e "UpdateOTS.sh [${LINENO}]  \t running script $OTSDAQ_DIR/../${line}/tools/update_ots_repo.sh"	
			$OTSDAQ_DIR/../${line}/tools/update_ots_repo.sh			
		done < $USER_DATA/ServiceData/InstalledRepoNames.dat
	
		#do one more time after loop to make sure last line is read (even if user did not put new line)
		echo
		echo -e "UpdateOTS.sh [${LINENO}]  \t updating ${line} repository...."		
		echo -e "UpdateOTS.sh [${LINENO}]  \t running script $OTSDAQ_DIR/../${line}/tools/update_ots_repo.sh"	
		$OTSDAQ_DIR/../${line}/tools/update_ots_repo.sh			
			
	fi
	
	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
		

	
	
	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################" 
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################" 
	echo -e "UpdateOTS.sh [${LINENO}]  \t Upgrading database (if needed)..."
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo
	
	#TODO by lukhanin

	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
		

	
	
	
	
	
	
	
	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t Updating ups products based on .bz2 files in $MRB_SOURCE/otsdaq/tarballs/"
	echo -e "UpdateOTS.sh [${LINENO}]  \t PRODUCTS path found as: $PRODUCTS"
	IFS=':' read -r -a array <<< "$PRODUCTS"
	UPS_DIR=${array[@]: -1:1}
	echo -e "UpdateOTS.sh [${LINENO}]  \t Unzipping any extra products from otsdaq to: $UPS_DIR"	
	
	cd $UPS_DIR
	for file in $MRB_SOURCE/otsdaq/tarballs/*.bz2 	# undo c++ style comment for Eclipse viewing*/
	do 
		IFS='/' read -r -a array <<< "$file"
		UPS_FILE_NAME=${array[@]: -1:1}
		IFS='-' read -r -a array <<< "$UPS_FILE_NAME"
		UPS_FILE_NAME_FIELDS="${#array[@]}"		
		#echo -e "UpdateOTS.sh [${LINENO}]  \t $UPS_FILE_NAME_FIELDS fields found"
		if [ $UPS_FILE_NAME_FIELDS -lt 7 ]; then
			echo -e "UpdateOTS.sh [${LINENO}]  \t 	$file skipping, (7 fields expected) too few fields in name to identify name, version, qualifier..."
			echo -e "UpdateOTS.sh [${LINENO}]  \t   Would like to do this command, but not sure it is necessary:"
			echo -e "UpdateOTS.sh [${LINENO}]  \t    tar -xf $file"
			continue
		fi
		
		UPS_PRODUCT_NAME=${array[0]}
		UPS_PRODUCT_VERSION=${array[1]}
		UPS_PRODUCT_VERSION=${UPS_PRODUCT_VERSION//./_}
			
		#e.g. slf6.x86_64.e10.s41.debug
		UPS_PRODUCT_QUAL="${array[2]}.${array[3]}.${array[4]}.${array[5]}"
		IFS='.' read -r -a array <<< "${array[6]}"
		UPS_PRODUCT_QUAL="$UPS_PRODUCT_QUAL.${array[0]}"
		
		echo -e "UpdateOTS.sh [${LINENO}]  \t Checking $UPS_PRODUCT_NAME/v$UPS_PRODUCT_VERSION/$UPS_PRODUCT_QUAL..."
		
		if [ ! -d "$UPS_PRODUCT_NAME/v$UPS_PRODUCT_VERSION/$UPS_PRODUCT_QUAL" ]; then
			echo -e "UpdateOTS.sh [${LINENO}]  \t    $file unzipping..."
			echo -e "UpdateOTS.sh [${LINENO}]  \t    tar -xf $file"
			tar -xf $file &>> $UPDATE_LOG_PATH
			
			if [ ! -d "$UPS_PRODUCT_NAME/v$UPS_PRODUCT_VERSION/$UPS_PRODUCT_QUAL" ]; then				
				echo -e "UpdateOTS.sh [${LINENO}]  \t    Something went wrong. Unzip was not successful. (Are special permissions required for products area?)"
				echo
				echo -e "UpdateOTS.sh [${LINENO}]  \t 	 Pausing for 3 seconds (so you read this!)..."
				sleep 3s				
			fi
		else
			echo -e "UpdateOTS.sh [${LINENO}]  \t 	...already found in ups products."
		fi
			
	done
	
	cd $CURRENT_AWESOME_BASE
	
	#done updating ups products from otsdaq repo /tarballs
	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"

fi


echo -e "UpdateOTS.sh [${LINENO}]  \t Git comment '$1'"
echo -e "UpdateOTS.sh [${LINENO}]  \t Check-in status was logged here: $CHECKIN_LOG_PATH"
echo -e "UpdateOTS.sh [${LINENO}]  \t Update status was logged here: $UPDATE_LOG_PATH"
echo
echo -e "UpdateOTS.sh [${LINENO}]  \t log dump in 2 seconds... #######################################################"
sleep 2s
echo
cat $CHECKIN_LOG_PATH
echo -e "UpdateOTS.sh [${LINENO}]  \t end log dump... #######################################################"
echo -e "UpdateOTS.sh [${LINENO}]  \t Check-in status was logged here: $CHECKIN_LOG_PATH"
echo -e "UpdateOTS.sh [${LINENO}]  \t Update status (not shown above) was logged here: $UPDATE_LOG_PATH"

echo
echo -e "UpdateOTS.sh [${LINENO}]  \t =================="
echo
echo -e "UpdateOTS.sh [${LINENO}]  \t =================="
echo -e "UpdateOTS.sh [${LINENO}]  \t ots update script done"
echo -e "UpdateOTS.sh [${LINENO}]  \t *******************************"
echo -e "UpdateOTS.sh [${LINENO}]  \t *******************************"