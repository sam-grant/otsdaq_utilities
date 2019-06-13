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
CHECKIN_LOG_PATH=$CURRENT_AWESOME_BASE/.UpdateOTS_pull.log
UPDATE_LOG_PATH=$CURRENT_AWESOME_BASE/.UpdateOTS_push.log

echo -e "UpdateOTS.sh [${LINENO}]  "
echo -e "UpdateOTS.sh [${LINENO}]  \t ~~ UpdateOTS ~~ "
echo -e "UpdateOTS.sh [${LINENO}]  "
echo -e "UpdateOTS.sh [${LINENO}]  "

#replace StartOTS.sh in any setup file!
sed -i s/StartOTS\.sh/ots/g ${MRB_SOURCE}/../setup_*

if [ "x$1" == "x" ] || [[ "$1" != "--fetch" && "$1" != "--fetchall" && "$1" != "--pull" && "$1" != "--push" && "$1" != "--pullcore" && "$1" != "--pushcore" && "$1" != "--pullall" && "$1" != "--pushall" && "$1" != "--tables" ]]; then
    echo -e "UpdateOTS.sh [${LINENO}]  \t Usage: Parameter 1 is the operation and, for pushes, Parameter 2 is the comment for git commit"
	echo -e "UpdateOTS.sh [${LINENO}]  "
    echo -e "UpdateOTS.sh [${LINENO}]  \t Note: git status will be logged here: $CHECKIN_LOG_PATH"
	echo -e "UpdateOTS.sh [${LINENO}]  "
	echo -e "UpdateOTS.sh [${LINENO}]  \t Parameter 1 operations:"
	echo -e "UpdateOTS.sh [${LINENO}]  \t\t --fetch               \t #will fetch otsdaq repositories in srcs/"
	echo -e "UpdateOTS.sh [${LINENO}]  \t\t --fetchall            \t #will fetch otsdaq repositories in srcs/"
	echo -e "UpdateOTS.sh [${LINENO}]  \t\t --pull                \t #will pull otsdaq user repositories in srcs/"
	echo -e "UpdateOTS.sh [${LINENO}]  \t\t --push \"comment\"    \t #will push otsdaq user repositories in srcs/"
	echo -e "UpdateOTS.sh [${LINENO}]  \t\t --pullcore            \t #will pull otsdaq core repositories in srcs/"
	echo -e "UpdateOTS.sh [${LINENO}]  \t\t --pushcore \"comment\"\t #will push otsdaq core repositories in srcs/"
	echo -e "UpdateOTS.sh [${LINENO}]  \t\t --pullall             \t #will pull all    repositories in srcs/ (i.e. not just otsdaq)."
	echo -e "UpdateOTS.sh [${LINENO}]  \t\t --pushall \"comment\" \t #will push all    repositories in srcs/ (i.e. not just otsdaq)."
	echo -e "UpdateOTS.sh [${LINENO}]  \t\t --tables              \t #will not pull or push; it will just update tables."
		
	echo -e "UpdateOTS.sh [${LINENO}]  "
	exit
fi

# at this point, there must have been a valid option


#############################
#############################
# function to update USER DATA configuration files and table definitions
function updateUserData 
{	
	echo -e "UpdateOTS.sh [${LINENO}]  \t Updating tables..."
			
	
	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################" 
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo -e "UpdateOTS.sh [${LINENO}]  \t "
	echo -e "UpdateOTS.sh [${LINENO}]  \t Updating USER_DATA path ${USER_DATA}..."
	echo -e "UpdateOTS.sh [${LINENO}]  \t "
	echo -e "UpdateOTS.sh [${LINENO}]  \t "
	echo -e "UpdateOTS.sh [${LINENO}]  \t "
	echo -e "UpdateOTS.sh [${LINENO}]  \t Table info is updated based on the list in..."
	echo -e "UpdateOTS.sh [${LINENO}]  \t "
	echo -e "UpdateOTS.sh [${LINENO}]  \t \t ${USER_DATA}/ServiceData/CoreTableInfoNames.dat"
	echo -e "UpdateOTS.sh [${LINENO}]  \t "
	echo -e "UpdateOTS.sh [${LINENO}]  \t ... each line will be copied into user data relative to path ${OTSDAQ_DIR}/data-core/TableInfo/"
	echo -e "UpdateOTS.sh [${LINENO}]  \t "
	echo -e "UpdateOTS.sh [${LINENO}]  \t If CoreTableInfoNames.dat doesn't exist the whole directory ${OTSDAQ_DIR}/data-core/TableInfo/ will be copied!"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo
	
	echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/TableInfo/TableInfo.xsd $USER_DATA/TableInfo/"
	cp $OTSDAQ_DIR/data-core/TableInfo/TableInfo.xsd $USER_DATA/TableInfo/
			
	if [ -e "$USER_DATA/ServiceData/CoreTableInfoNames.dat" ]; then
		echo -e "UpdateOTS.sh [${LINENO}]  \t $USER_DATA/ServiceData/CoreTableInfoNames.dat exists!"
		echo -e "UpdateOTS.sh [${LINENO}]  \t Loading updated info for core tables (relative paths and wildcards are allowed) from $OTSDAQ_DIR/data-core/TableInfo/ ..."
		echo
		

		#replace TheSupervisorConfiguration with GatewaySupervisorConfiguration for updating
		sed -i s/TheSupervisorConfiguration/GatewaySupervisorConfiguration/g $USER_DATA/ServiceData/CoreTableInfoNames.dat
		
		cat $USER_DATA/ServiceData/CoreTableInfoNames.dat
		echo
		
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp -r $USER_DATA/TableInfo $USER_DATA/TableInfo.updateots.bk"
		rm -rf $USER_DATA/TableInfo.updateots.bk
		cp -r $USER_DATA/TableInfo $USER_DATA/TableInfo.updateots.bk		
		
		#NOTE: relative paths are allowed from otsdaq/data-core/TableInfo
		LAST_LINE=
		while read line; do
			if [[ "x${line}" != "x" && "${LAST_LINE}" != "${line}" ]]; then
				#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/TableInfo/${line}Info.xml $USER_DATA/TableInfo/"						
				cp $OTSDAQ_DIR/data-core/TableInfo/${line}Info.xml $USER_DATA/TableInfo/ #do not hide failures anymore --- &>/dev/null #hide output		
				LAST_LINE=${line}
			fi
		done < $USER_DATA/ServiceData/CoreTableInfoNames.dat
		
		#do one more time after loop to make sure last line is read 
		# (even if user did not put new line) 
		if [[ "x${line}" != "x" && "${LAST_LINE}" != "${line}" ]]; then
			#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/TableInfo/${line}Info.xml $USER_DATA/TableInfo/"						
			cp $OTSDAQ_DIR/data-core/TableInfo/${line}Info.xml $USER_DATA/TableInfo/ #do not hide failures anymore ---&>/dev/null #hide output
		fi
	else
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp -r $USER_DATA/TableInfo $USER_DATA/TableInfo_update_bk"
		rm -rf $USER_DATA/TableInfo_update_bk
		cp -r $USER_DATA/TableInfo/ $USER_DATA/TableInfo_update_bk
		
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/TableInfo/ARTDAQ/*Info.xml $USER_DATA/TableInfo/"
		cp $OTSDAQ_DIR/data-core/TableInfo/ARTDAQ/*Info.xml $USER_DATA/TableInfo/ 		# undo c++ style comment for Eclipse viewing*/
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/TableInfo/BackboneGroup/*Info.xml $USER_DATA/TableInfo/"
		cp $OTSDAQ_DIR/data-core/TableInfo/BackboneGroup/*Info.xml $USER_DATA/TableInfo/			# undo c++ style comment for Eclipse viewing*/
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/TableInfo/ConfigCore/*Info.xml $USER_DATA/TableInfo/"
		cp $OTSDAQ_DIR/data-core/TableInfo/ConfigCore/*Info.xml $USER_DATA/TableInfo/ 		# undo c++ style comment for Eclipse viewing*/
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/TableInfo/ContextGroup/*Info.xml $USER_DATA/TableInfo/"
		cp $OTSDAQ_DIR/data-core/TableInfo/ContextGroup/*Info.xml $USER_DATA/TableInfo/			# undo c++ style comment for Eclipse viewing*/
		echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/TableInfo/IterateGroup/*Info.xml $USER_DATA/TableInfo/"
		cp $OTSDAQ_DIR/data-core/TableInfo/IterateGroup/*Info.xml $USER_DATA/TableInfo/ 		# undo c++ style comment for Eclipse viewing*/
		
	fi
	
	echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/XDAQConfigurations/otsConfigurationNoRU_Wizard_CMake.xml $USER_DATA/XDAQConfigurations/"
	cp $OTSDAQ_DIR/data-core/XDAQConfigurations/otsConfigurationNoRU_Wizard_CMake.xml $USER_DATA/XDAQConfigurations/
	echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/XDAQConfigurations/otsConfigurationNoRU_MacroMaker_CMake.xml $USER_DATA/XDAQConfigurations/"
	cp $OTSDAQ_DIR/data-core/XDAQConfigurations/otsConfigurationNoRU_MacroMaker_CMake.xml $USER_DATA/XDAQConfigurations/
		
	echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/data-core/MessageFacilityConfigurations/* $USER_DATA/MessageFacilityConfigurations/"
	cp $OTSDAQ_DIR/data-core/MessageFacilityConfigurations/* $USER_DATA/MessageFacilityConfigurations/ # undo c++ style comment for Eclipse viewing*/
		
	#make sure permissions are usable
	echo -e "UpdateOTS.sh [${LINENO}]  \t chmod 755 $USER_DATA/TableInfo/*.xml"
	chmod 755 $USER_DATA/TableInfo/*.xml #*/ just resetting comment coloring
	echo -e "UpdateOTS.sh [${LINENO}]  \t chmod 755 $USER_DATA/TableInfo/*Info.xsd"
	chmod 755 $USER_DATA/TableInfo/*Info.xsd #*/ just resetting comment coloring

	echo -e "UpdateOTS.sh [${LINENO}]  \t "
	echo -e "UpdateOTS.sh [${LINENO}]  \t Reminder, table info is updated based on the list in..."
	echo -e "UpdateOTS.sh [${LINENO}]  \t "
	echo -e "UpdateOTS.sh [${LINENO}]  \t \t ${USER_DATA}/ServiceData/CoreTableInfoNames.dat"
	echo -e "UpdateOTS.sh [${LINENO}]  \t "
	
	echo -e "UpdateOTS.sh [${LINENO}]  \t Done updating USER DATA."
			
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo -e "UpdateOTS.sh [${LINENO}]  \t #######################################################################################################################"
	echo
	
} # end updateUserData function
export -f updateUserData

#clear git comment to avoid push
GIT_COMMENT=

ALL_REPOS=0
TABLES_ONLY=0
SKIP_CORE=0
ONLY_CORE=0

FETCH_ONLY=0

if [ "$1"  == "--tables" ]; then
	TABLES_ONLY=1
	echo -e "UpdateOTS.sh [${LINENO}]  \t Updating tables only!"
	updateUserData
	exit
fi
if [ "$1"  == "--pullall" ]; then
	ALL_REPOS=1
	echo -e "UpdateOTS.sh [${LINENO}]  \t Updating all repositories (i.e. not only otsdaq)!"
fi
if [ "$1"  == "--pushall" ]; then
	ALL_REPOS=1
	GIT_COMMENT=$2
	if [ "x$2" == "x" ]; then
		echo -e "UpdateOTS.sh [${LINENO}]  \t For git push, a comment must be placed in Parameter 2!"
		exit
	fi
	echo -e "UpdateOTS.sh [${LINENO}]  \t Pushing all repositories (i.e. not only otsdaq)!"
fi
if [ "$1"  == "--push" ]; then
	SKIP_CORE=1
	GIT_COMMENT=$2
	if [ "x$2" == "x" ]; then
		echo -e "UpdateOTS.sh [${LINENO}]  \t For git push, a comment must be placed in Parameter 2!"
		exit
	fi
	echo -e "UpdateOTS.sh [${LINENO}]  \t Pushing otsdaq user repositories!"
fi
if [ "$1"  == "--pull" ]; then		
	SKIP_CORE=1
	echo -e "UpdateOTS.sh [${LINENO}]  \t Pulling otsdaq user repositories!"
fi
if [ "$1"  == "--pushcore" ]; then
	ONLY_CORE=1
	GIT_COMMENT=$2	
	if [ "x$2" == "x" ]; then
		echo -e "UpdateOTS.sh [${LINENO}]  \t For git push, a comment must be placed in Parameter 2!"
		exit
	fi
	echo -e "UpdateOTS.sh [${LINENO}]  \t Pushing otsdaq core repositories!"
fi
if [ "$1"  == "--pullcore" ]; then	
	ONLY_CORE=1
	echo -e "UpdateOTS.sh [${LINENO}]  \t Pulling otsdaq core repositories!"
fi

if [ "$1"  == "--fetchall" ]; then
	ALL_REPOS=1
	FETCH_ONLY=1
	echo -e "UpdateOTS.sh [${LINENO}]  \t Fetching all repositories (i.e. not only otsdaq)!"
fi
if [ "$1"  == "--fetch" ]; then		
	FETCH_ONLY=1
	echo -e "UpdateOTS.sh [${LINENO}]  \t Fetching otsdaq repositories!"
fi

echo -e "UpdateOTS.sh [${LINENO}]  \t REPO_FILTER = ${REPO_FILTER}"
echo
echo
echo -e "UpdateOTS.sh [${LINENO}]  \t Finding paths..."

SCRIPT_DIR="$( 
  cd "$(dirname "$(readlink "$0" || printf %s "$0")")"
  pwd -P 
)"
		
echo -e "UpdateOTS.sh [${LINENO}]  \t Script directory found as: $SCRIPT_DIR"
echo -e "UpdateOTS.sh [${LINENO}]  \t Finding target repositories..."
 
#REPO_DIR="$(find $SCRIPT_DIR/../../../srcs -maxdepth 1 -iname 'otsdaq*')" #old way before using MRB path
if [ $ALL_REPOS = 1 ]; then
	REPO_DIR="$(find $MRB_SOURCE -maxdepth 1 -iname '*')"
else
	REPO_DIR="$(find $MRB_SOURCE -maxdepth 1 -iname 'otsdaq*')"
fi
					
for p in ${REPO_DIR[@]}; do
    if [ -d $p ]; then
    if [ -d $p/.git ]; then
    
    	bp=$(basename $p)
    	if [ $SKIP_CORE = 1 ] && [[ $bp = "otsdaq" || $bp = "otsdaq_utilities" || $bp = "otsdaq_components" ]]; then
    		continue #skip core repos
		fi
		if [ $ONLY_CORE = 1 ] && [[ $bp != "otsdaq" && $bp != "otsdaq_utilities" && $bp != "otsdaq_components" ]]; then
			continue #skip non-core repos
		fi
    	
		echo -e "UpdateOTS.sh [${LINENO}]  \t Repo directory found as: $bp"
	fi
    fi	   
done


 
#######################################################################################################################

echo
echo -e "UpdateOTS.sh [${LINENO}]  \t =================="

echo -e "UpdateOTS.sh [${LINENO}]  \t Git comment '$GIT_COMMENT'"
echo -e "UpdateOTS.sh [${LINENO}]  \t Status will be logged here: $CHECKIN_LOG_PATH"


echo
echo -e "UpdateOTS.sh [${LINENO}]  \t =================="

echo -e "UpdateOTS.sh [${LINENO}]  \t log start:" > $CHECKIN_LOG_PATH
for p in ${REPO_DIR[@]}; do
    if [ -d $p ]; then
    if [ -d $p/.git ]; then

	bp=$(basename $p)
	if [ $SKIP_CORE = 1 ] && [[ $bp = "otsdaq" || $bp = "otsdaq_utilities" || $bp = "otsdaq_components" ]]; then
		continue #skip core repos
	fi
	if [ $ONLY_CORE = 1 ] && [[ $bp != "otsdaq" && $bp != "otsdaq_utilities" && $bp != "otsdaq_components" ]]; then
		continue #skip non-core repos
	fi
	
	cd $p
	
	if [ $FETCH_ONLY = 1 ]; then
		echo -e "UpdateOTS.sh [${LINENO}]  \t Fetching updates from $p"
		git fetch
	else
		echo -e "UpdateOTS.sh [${LINENO}]  \t Pulling updates from $p"
		git pull
	fi
	
	echo -e "UpdateOTS.sh [${LINENO}]  \t ==================" >> $CHECKIN_LOG_PATH
	pwd >> $CHECKIN_LOG_PATH
	git status &>> $CHECKIN_LOG_PATH
	
	if [ "x$GIT_COMMENT" != "x" ]; then
		
		echo -e "UpdateOTS.sh [${LINENO}]  \t Checking in $p"
	    git commit -m "$GIT_COMMENT " .  &>> $CHECKIN_LOG_PATH  #add space in comment for user
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
if [[ "x$GIT_COMMENT" == "x" && $FETCH_ONLY = 0 ]]; then

	echo -e "UpdateOTS.sh [${LINENO}]  \t Update status will be logged here: $UPDATE_LOG_PATH"
	echo -e "UpdateOTS.sh [${LINENO}]  \t Update log start:" > $UPDATE_LOG_PATH

	updateUserData #call function
	
	#copy tutorial launching scripts
	echo
	echo -e "UpdateOTS.sh [${LINENO}]  \t updating tutorial launch scripts..."
	chmod 755 $MRB_SOURCE/../get_tutorial_data.sh #make sure permissions allow deleting
	chmod 755 $MRB_SOURCE/../get_tutorial_database.sh #make sure permissions allow deleting
	chmod 755 $MRB_SOURCE/../reset_ots_tutorial.sh #make sure permissions allow deleting
	rm $MRB_SOURCE/../get_tutorial_data.sh &>/dev/null 2>&1 #hide output
	rm $MRB_SOURCE/../get_tutorial_database.sh &>/dev/null 2>&1 #hide output
	rm $MRB_SOURCE/../reset_ots_tutorial.sh &>/dev/null 2>&1 #hide output
	#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_tutorial.sh $OTSDAQ_DIR/../../reset_ots_tutorial.sh"
	#cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_tutorial.sh $OTSDAQ_DIR/../../reset_ots_tutorial.sh
	wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/demo/revisions/develop/raw/tools/reset_ots_tutorial.sh -P $MRB_SOURCE/../ --no-check-certificate	
	chmod 644 $MRB_SOURCE/../reset_ots_tutorial.sh
	
	rm $MRB_SOURCE/../reset_ots_artdaq_tutorial.sh &>/dev/null 2>&1 #hide output
	#now there is only one reset_tutorial script (that includes the artdaq tutorial), so do not get script
	#echo -e "UpdateOTS.sh [${LINENO}]  \t cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_artdaq_tutorial.sh $OTSDAQ_DIR/../../reset_ots_artdaq_tutorial.sh"
	#cp $OTSDAQ_DIR/../otsdaq_demo/tools/reset_ots_artdaq_tutorial.sh $OTSDAQ_DIR/../../reset_ots_artdaq_tutorial.sh
	#wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/demo/revisions/develop/raw/tools/reset_ots_artdaq_tutorial.sh -P $OTSDAQ_DIR/../../ --no-check-certificate	
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
			echo -e "UpdateOTS.sh [${LINENO}]  \t running script $MRB_SOURCE/${line}/tools/update_ots_repo.sh"	
			$MRB_SOURCE/${line}/tools/update_ots_repo.sh			
		done < $USER_DATA/ServiceData/InstalledRepoNames.dat
	
		#do one more time after loop to make sure last line is read (even if user did not put new line)
		echo
		echo -e "UpdateOTS.sh [${LINENO}]  \t updating ${line} repository...."		
		echo -e "UpdateOTS.sh [${LINENO}]  \t running script $MRB_SOURCE/${line}/tools/update_ots_repo.sh"	
		$MRB_SOURCE/${line}/tools/update_ots_repo.sh			
			
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


echo -e "UpdateOTS.sh [${LINENO}]  \t Git comment '$GIT_COMMENT'"
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

for p in ${REPO_DIR[@]}; do
    if [ -d $p ]; then
    if [ -d $p/.git ]; then
    

		bp=$(basename $p)
		if [ $SKIP_CORE = 1 ] && [[ $bp = "otsdaq" || $bp = "otsdaq_utilities" || $bp = "otsdaq_components" ]]; then
			continue #skip core repos
		fi
		if [ $ONLY_CORE = 1 ] && [[ $bp != "otsdaq" && $bp != "otsdaq_utilities" && $bp != "otsdaq_components" ]]; then
			continue #skip non-core repos
		fi
    
		echo -e "UpdateOTS.sh [${LINENO}]  \t Repo directory handled: $bp"
	fi
    fi	   
done

echo
echo
echo -e "UpdateOTS.sh [${LINENO}]  \t Note: Here are your localProducts directories..."
echo
ls ${MRB_SOURCE}/../ | grep localProducts | grep -v href #-v is inverse grep
echo
echo

echo -e "UpdateOTS.sh [${LINENO}]  \t Note: below are the available otsdaq releases..."
echo -e "UpdateOTS.sh [${LINENO}]  \t ----------------------------"
curl http://scisoft.fnal.gov/scisoft/bundles/otsdaq/ | grep \<\/a\> | grep _ | grep v
echo -e "UpdateOTS.sh [${LINENO}]  \t ----------------------------"
echo -e "UpdateOTS.sh [${LINENO}]  \t Note: above are the available otsdaq releases..."
echo
echo
echo -e "UpdateOTS.sh [${LINENO}]  \t To determine the available qualifiers go here in your browser:"
echo
echo -e "\t\t\t\t http://scisoft.fnal.gov/scisoft/bundles/otsdaq/"
echo
echo -e "UpdateOTS.sh [${LINENO}]  \t ... then click the version, and manifest folder to view qualifiers."
echo
echo -e "UpdateOTS.sh [${LINENO}]  \t To switch qualifiers, do the following: mrb newDev -v v2_02_00 -q s67:e15:prof"
echo -e "UpdateOTS.sh [${LINENO}]  \t ...and replace 'v2_02_00' with your target version. and 's67:e15:prof' with your qualifiers"
echo -e "UpdateOTS.sh [${LINENO}]  \t ...a new localProducts directory will be created, which you should use when you setup ots."
echo
echo

echo
echo
echo -e "UpdateOTS.sh [${LINENO}]  \t Note: Here are your localProducts directories..."
echo
ls ${MRB_SOURCE}/../ | grep localProducts
echo
echo


echo -e "UpdateOTS.sh [${LINENO}]  \t =================="
echo -e "UpdateOTS.sh [${LINENO}]  \t ots update script done"
echo -e "UpdateOTS.sh [${LINENO}]  \t *******************************"
echo -e "UpdateOTS.sh [${LINENO}]  \t *******************************"

		
		



