#!/bin/bash
#
# This script is expected to be in otsdaq utilities repository in a specific directory
# but it can be executed from any path (do not source it, execute with ./ )
#
# ./path/to/script/checkinAll.sh "comment for git commit"
#
# If no comment is given, the script will only pull updates - it will not checkin.
#



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



 
#######################################################################################################################

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


#######################################################################################################################
#handle manual updates that should take place ONLY if it is UPDATING not committing
if [ "x$1" == "x" ]; then

	echo "Update status will be logged here: $CURRENT_AWESOME_BASE/updateAll.log"
	echo "Update log start:" > $CURRENT_AWESOME_BASE/updateAll.log
	
	echo
	echo "#######################################################################################################################" 
	echo "#######################################################################################################################" 
	echo "Updating USER_DATA path $USER_DATA,"
	echo "based on the list in $USER_DATA/ServiceData/CoreTableInfoNames.dat."
	echo "If CoreTableInfoNames.dat doesn't exist the whole directory $OTSDAQ_DIR/data-core/ConfigurationInfo/ will be copied!"
	echo "#######################################################################################################################"
	echo "#######################################################################################################################"
	echo
	
	echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ConfigurationInfo.xsd $USER_DATA/ConfigurationInfo/"
	cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ConfigurationInfo.xsd $USER_DATA/ConfigurationInfo/
			
	if [ -e "$USER_DATA/ServiceData/CoreTableInfoNames.dat" ]; then
		echo "$USER_DATA/ServiceData/CoreTableInfoNames.dat exists!"
		echo "Loading updated info for core tables..."
		cat $USER_DATA/ServiceData/CoreTableInfoNames.dat
		echo
		
		echo "cp -r $USER_DATA/ConfigurationInfo $USER_DATA/ConfigurationInfo.updateots.bk"
		rm -rf $USER_DATA/ConfigurationInfo.updateots.bk
		cp -r $USER_DATA/ConfigurationInfo $USER_DATA/ConfigurationInfo.updateots.bk		
		
		#NOTE: relative paths are allowed from otsdaq/data-core/ConfigurationInfo
		while read line; do
			#echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
			cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/${line}Info.xml $USER_DATA/ConfigurationInfo/	&>/dev/null #hide output	
			#echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
			cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/${line}Info.xml $USER_DATA/ConfigurationInfo/ &>/dev/null #hide output		
			#echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
			cp $OTSDAQ_DIR/data-core/ConfigurationInfo/${line}Info.xml $USER_DATA/ConfigurationInfo/ &>/dev/null #hide output		
		done < $USER_DATA/ServiceData/CoreTableInfoNames.dat
		
		#do one more time after loop to make sure last line is read (even if user did not put new line) 
		#echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/${line}Info.xml $USER_DATA/ConfigurationInfo/ &>/dev/null #hide output		
		#echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/${line}Info.xml $USER_DATA/ConfigurationInfo/ &>/dev/null #hide output		
		#echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/${line}Info.xml $USER_DATA/ConfigurationInfo/"						
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/${line}Info.xml $USER_DATA/ConfigurationInfo/ &>/dev/null #hide output
	else
		echo "cp -r $USER_DATA/ConfigurationInfo $USER_DATA/ConfigurationInfo_update_bk"
		rm -rf $USER_DATA/ConfigurationInfo_update_bk
		cp -r $USER_DATA/ConfigurationInfo/ $USER_DATA/ConfigurationInfo_update_bk
		echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/*Info.xml $USER_DATA/ConfigurationInfo/"
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/*Info.xml $USER_DATA/ConfigurationInfo/
		# undo c++ style comment for Eclipse viewing*/
		echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/*Info.xml $USER_DATA/ConfigurationInfo/"
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/ARTDAQ/*Info.xml $USER_DATA/ConfigurationInfo/
		# undo c++ style comment for Eclipse viewing*/
		echo "cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/*Info.xml $USER_DATA/ConfigurationInfo/"
		cp $OTSDAQ_DIR/data-core/ConfigurationInfo/CORE/*Info.xml $USER_DATA/ConfigurationInfo/
		# undo c++ style comment for Eclipse viewing*/
	fi
	
	echo "cp $OTSDAQ_DIR/data-core/XDAQConfigurations/otsConfigurationNoRU_Wizard_CMake.xml $USER_DATA/XDAQConfigurations/"
	cp $OTSDAQ_DIR/data-core/XDAQConfigurations/otsConfigurationNoRU_Wizard_CMake.xml $USER_DATA/XDAQConfigurations/

	#copy tutorial launching scripts
	echo
	echo "updating tutorial launch scripts..."
	rm $OTSDAQ_DIR/../../reset_ots_tutorial.sh
	echo "cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_tutorial.sh $OTSDAQ_DIR/../../reset_ots_tutorial.sh"	
	cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_tutorial.sh $OTSDAQ_DIR/../../reset_ots_tutorial.sh
	rm $OTSDAQ_DIR/../../reset_ots_artdaq_tutorial.sh
	echo "cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_artdaq_tutorial.sh $OTSDAQ_DIR/../../reset_ots_artdaq_tutorial.sh"	
	cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_artdaq_tutorial.sh $OTSDAQ_DIR/../../reset_ots_artdaq_tutorial.sh
	
	echo
	echo "#######################################################################################################################"
	echo "#######################################################################################################################"
	#end copy tables
	
	
	
	

	echo
	echo "#######################################################################################################################" 
	echo "#######################################################################################################################" 
	echo "Updating installed Repositories,"
	echo "based on the list in $USER_DATA/ServiceData/InstalledRepoNames.dat."
	echo "If InstalledRepoNames.dat doesn't exist, then nothing happens"
	echo "#######################################################################################################################"
	echo "#######################################################################################################################"
	echo
	
	if [ -e "$USER_DATA/ServiceData/InstalledRepoNames.dat" ]; then
		echo "$USER_DATA/ServiceData/InstalledRepoNames.dat exists!"
		echo "Loading list of repos to update..."
		cat $USER_DATA/ServiceData/InstalledRepoNames.dat
		echo
			
	
		#NOTE: relative paths are allowed from otsdaq/../
		while read line; do
			echo
			echo "updating ${line} repository...."
			echo "running script $OTSDAQ_DIR/../${line}/tools/update_ots_repo.sh"	
			$OTSDAQ_DIR/../${line}/tools/update_ots_repo.sh			
		done < $USER_DATA/ServiceData/InstalledRepoNames.dat
	
		#do one more time after loop to make sure last line is read (even if user did not put new line)
		echo
		echo "updating ${line} repository...."		
		echo "running script $OTSDAQ_DIR/../${line}/tools/update_ots_repo.sh"	
		$OTSDAQ_DIR/../${line}/tools/update_ots_repo.sh			
			
	fi
	
	echo
	echo "#######################################################################################################################"
	echo "#######################################################################################################################"
		
	
	
	
	
	
	
	
	
	
	
	echo
	echo "Updating ups products based on .bz2 files in $MRB_SOURCE/otsdaq/tarballs/"
	echo "PRODUCTS path found as: $PRODUCTS"
	IFS=':' read -r -a array <<< "$PRODUCTS"
	UPS_DIR=${array[@]: -1:1}
	echo "Unzipping any extra products from otsdaq to: $UPS_DIR"	
	
	cd $UPS_DIR
	for file in $MRB_SOURCE/otsdaq/tarballs/*.bz2 	# undo c++ style comment for Eclipse viewing*/
	do 
		IFS='/' read -r -a array <<< "$file"
		UPS_FILE_NAME=${array[@]: -1:1}
		IFS='-' read -r -a array <<< "$UPS_FILE_NAME"
		UPS_FILE_NAME_FIELDS="${#array[@]}"		
		#echo "$UPS_FILE_NAME_FIELDS fields found"
		if [ $UPS_FILE_NAME_FIELDS -lt 7 ]; then
			echo "	$file skipping, (7 fields expected) too few fields in name to identify name, version, qualifier..."
			echo "  Would like to do this command, but not sure it is necessary:"
			echo "   tar -xf $file"
			continue
		fi
		
		UPS_PRODUCT_NAME=${array[0]}
		UPS_PRODUCT_VERSION=${array[1]}
		UPS_PRODUCT_VERSION=${UPS_PRODUCT_VERSION//./_}
			
		#e.g. slf6.x86_64.e10.s41.debug
		UPS_PRODUCT_QUAL="${array[2]}.${array[3]}.${array[4]}.${array[5]}"
		IFS='.' read -r -a array <<< "${array[6]}"
		UPS_PRODUCT_QUAL="$UPS_PRODUCT_QUAL.${array[0]}"
		
		echo "Checking $UPS_PRODUCT_NAME/v$UPS_PRODUCT_VERSION/$UPS_PRODUCT_QUAL..."
		
		if [ ! -d "$UPS_PRODUCT_NAME/v$UPS_PRODUCT_VERSION/$UPS_PRODUCT_QUAL" ]; then
			echo "   $file unzipping..."
			echo "   tar -xf $file"
			tar -xf $file &>> $CURRENT_AWESOME_BASE/updateAll.log
			
			if [ ! -d "$UPS_PRODUCT_NAME/v$UPS_PRODUCT_VERSION/$UPS_PRODUCT_QUAL" ]; then				
				echo "   Something went wrong. Unzip was not successful. (Are special permissions required for products area?)"
				echo
				echo "	 Pausing for 3 seconds (so you read this!)..."
				sleep 3s				
			fi
		else
			echo "	...already found in ups products."
		fi
			
	done
	
	cd $CURRENT_AWESOME_BASE
	
	#done updating ups products from otsdaq repo /tarballs
	echo
	echo "#######################################################################################################################"
	echo "#######################################################################################################################"

fi


echo "Git comment '$1'"
echo "Check-in status was logged here: $CURRENT_AWESOME_BASE/checkinAll.log"
echo "Update status was logged here: $CURRENT_AWESOME_BASE/updateAll.log"
echo
echo "log dump in 2 seconds... #######################################################"
sleep 2s
echo
cat $CURRENT_AWESOME_BASE/checkinAll.log
echo "end log dump... #######################################################"
echo "Check-in status was logged here: $CURRENT_AWESOME_BASE/checkinAll.log"
echo "Update status (not shown above) was logged here: $CURRENT_AWESOME_BASE/updateAll.log"

echo
echo "=================="
echo
echo "=================="
echo "Awesome script done"
echo "*******************************"
echo "*******************************"