#!/bin/bash
#
# This script is expected to be run as root from the directory into which you want ots installed.
#
# Note: you can try to install as a standard user, but the yum install commands will probably fail
# 	(this might be ok, if the system has already been setup).
#
# download https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/utilities/revisions/develop/raw/tools/quick_ots_install.sh
# cd my/install/path						# e.g. cd ~/
# mv /where/you/downloaded/the/script .   	# move the downloaded quick_ots_install.sh script to this folder
# ksu  										# to become root superuser, if you can
# chmod 755 quick_ots_install.sh			# to make the installer executable
# ./quick_ots_install.sh					# to run the installer
#


called=$_
if [[ $called != $0 ]]; then
    echo "This script must be executed and not sourced!"

	
	echo -e "quick_ots_install.sh [${LINENO}]  "
	echo -e "quick_ots_install.sh [${LINENO}]  \t ~~ quick_ots_install ~~ "
	echo -e "quick_ots_install.sh [${LINENO}]  "
	echo -e "quick_ots_install.sh [${LINENO}]  \t usage: ./quick_ots_install.sh"
	echo -e "quick_ots_install.sh [${LINENO}]  "
    return 1
fi

	
echo -e "quick_ots_install.sh [${LINENO}]  "
echo -e "quick_ots_install.sh [${LINENO}]  \t ~~ quick_ots_install ~~ "
echo -e "quick_ots_install.sh [${LINENO}]  "
echo -e "quick_ots_install.sh [${LINENO}]  \t usage: ./quick_ots_install.sh"
echo -e "quick_ots_install.sh [${LINENO}]  "

#clear environment
unsetup_all >/dev/null 2>&1
unalias kx >/dev/null 2>&1
unalias ots >/dev/null 2>&1

####################################### start redmine login code
####################################### start redmine login code
####################################### start redmine login code
####################################### start redmine login code
#
# urlencode -- encode special characters for post/get arguments
#
urlencode() {
   perl -pe 'chomp(); s{\W}{sprintf("%%%02x",ord($&))}ge;' "$@"
}
site=https://cdcvs.fnal.gov/redmine
listf=/tmp/list_p$$
cookief=/tmp/cookies_p$$
rlverbose=${rlverbose:=false}
trap 'rm -f /tmp/postdata$$ /tmp/at_p$$ $cookief $listf*' EXIT
#
# login form
#
do_login() {
	get_passwords
	get_auth_token "${site}/login"
	post_url  \
       "${site}/login" \
       "back_url=$site" \
       "authenticity_token=$authenticity_token" \
       "username=`echo $user | urlencode`" \
       "password=`echo $pass | urlencode`" \
       "login=Login ?" 
	if grep '>Sign in' $listf > /dev/null;then
		echo
        echo -e "quick_ots_install.sh [${LINENO}]  \t Login failed."
		unset user #force new login attempt
		unset pass
		LOGIN_WORKED=0
        false
	else
		LOGIN_WORKED=1
        true
	fi
}
get_passwords() {
	
   	case "x${user-}y${pass-}" in
   	xy)
       	if [ -r   ${REDMINE_AUTHDIR:-.}/.redmine_lib_passfile ];then 
	   		read -r user pass < ${REDMINE_AUTHDIR:-.}/.redmine_lib_passfile
       	else
	   
			printf "Enter your Services username: "
			read user

			#user=$USER
			stty -echo
			printf "Services password for $user: "
			read pass
			stty echo
       	fi;;
    esac
}
get_auth_token() {
    authenticity_token=`fetch_url "${1}" |
                  tee /tmp/at_p$$ |
                  grep 'name="authenticity_token"' |
                  head -1 |
                  sed -e 's/.*value="//' -e 's/".*//' | 
                  urlencode `
}
#
# fetch_url -- GET a url from a site, maintaining cookies, etc.
#
fetch_url() {
     wget \
        --no-check-certificate \
	--load-cookies=${cookief} \
        --referer="${lastpage-}" \
	--save-cookies=${cookief} \
	--keep-session-cookies \
	-o ${debugout:-/dev/null} \
	-O - \
	"$1"  | ${debugfilter:-cat}
     lastpage="$1"
}
#
# post_url POST to a url maintaining cookies, etc.
#    takes a url and multiple form data arguments
#    which are joined with "&" signs
#
post_url() {
     url="$1"
     extra=""
     if  [ "$url" == "-b" ];then
         extra="--remote-encoding application/octet-stream"
         shift
         url=$1
     fi
     shift
     the_data=""
     sep=""
     df=/tmp/postdata$$
     :>$df
     for d in "$@";do
        printf "%s" "$sep$d" >> $df
        sep="&"
     done
     wget -O $listf \
        -o $listf.log \
        --debug \
        --verbose \
        $extra \
        --no-check-certificate \
	--load-cookies=${cookief} \
	--save-cookies=${cookief} \
        --referer="${lastpage-}" \
	--keep-session-cookies \
        --post-file="$df"  $url
     if grep '<div.*id=.errorExplanation' $listf > /dev/null;then
        echo "Failed: error was:"
        cat $listf | sed -e '1,/<div.*id=.errorExplanation/d' | sed -e '/<.div>/,$d'
        return 1
     fi
     if grep '<div.*id=.flash_notice.*Success' $listf > /dev/null;then
        $rlverbose && echo "Succeeded"
        return 0
     fi
     # not sure if it worked... 
     $rlverbose && echo "Unknown -- detagged output:"
     $rlverbose && cat $listf | sed -e 's/<[^>]*>//g'
     $rlverbose && echo "-----"
     $rlverbose && cat $listf.log
     $rlverbose && echo "-----"
     return 0
} # post_url

echo
echo -e "quick_ots_install.sh [${LINENO}]  \t Attempting login..."
do_login https://cdcvs.fnal.gov/redmine

if [ $LOGIN_WORKED == 0 ]; then
	echo -e "quick_ots_install.sh [${LINENO}]  \t Check your Fermilab Services name and password!"
	exit 1 
fi

echo -e "quick_ots_install.sh [${LINENO}]  \t Login successful."

		
####################################### end redmine login code
####################################### end redmine login code
####################################### end redmine login code
####################################### end redmine login code

INSTALL_DIR=$PWD
USER=$(whoami)
FOR_USER=$(stat -c "%U" $INSTALL_DIR)
FOR_GROUP=$(stat -c "%G" $INSTALL_DIR)

echo -e "quick_ots_install.sh [${LINENO}]  \t Identified user '$USER' installing ots with permissions for group:user '$FOR_GROUP:$FOR_USER'"
echo -e "quick_ots_install.sh [${LINENO}]  \t (note: target user is set based on the owner of $INSTALL_DIR)"
echo -e "quick_ots_install.sh [${LINENO}]  "

echo -e "quick_ots_install.sh [${LINENO}]  \t The installation will be at $INSTALL_DIR/ots"
echo -e "quick_ots_install.sh [${LINENO}]  \t If there currently is a $INSTALL_DIR/ots directory it will be moved to $INSTALL_DIR/oldOts"
echo

printf "Are you sure you would like to proceed with the install (y/n): "
read DO_INSTALL

echo
if [ $DO_INSTALL == "y" ]; then
	echo -e "quick_ots_install.sh [${LINENO}]  \t Proceeding with ots install..."
else	
	echo -e "quick_ots_install.sh [${LINENO}]  \t User chose not to proceed with ots install."
	exit
fi

# at this point, there must have been valid parameters

if [ $USER == "root" ]; then

	#install ots dependencies
	# yum install -y libuuid-devel openssl-devel python-devel elfutils-libelf-devel
	yum install -y git libuuid-devel openssl-devel curl-devel #as root for basic tools
	yum install -y gcc kernel-devel make #as root for compiling
	yum install -y lsb #as root for lsb_release
	yum install -y kde-baseapps #as root for kdialog
	yum install -y python2-devel elfutils-devel 
	yum install -y epel-release #repository to find libzstd and xxhash
	yum install -y libzstd xxhash xxhash-devel pcre2

	#install cvmfs
	yum install -y https://ecsft.cern.ch/dist/cvmfs/cvmfs-release/cvmfs-release-latest.noarch.rpm
	yum clean all
	yum install -y cvmfs cvmfs-config-default
	
	mkdir /etc
	mkdir /etc/cvmfs
	mkdir /etc/cvmfs/default.d
	
	rm -rf /etc/cvmfs/default.d/70-artdaq.conf
	echo "CVMFS_REPOSITORIES=fermilab.opensciencegrid.org" >> /etc/cvmfs/default.d/70-artdaq.conf
	echo "CVMFS_HTTP_PROXY=DIRECT" >> /etc/cvmfs/default.d/70-artdaq.conf
	
	#refresh cvmfs
	cvmfs_config setup
	#Check if CernVM-FS mounts the specified repositories by (restart if failure): 
	cvmfs_config probe || service autofs restart

fi

#install ots
mv ots oldOts/ && mkdir oldOts && rm -rf oldOts/ots && mv ots oldOts/  &>/dev/null
rm -rf ots
rm quick_ots_install.zip &>/dev/null

# wget https://otsdaq.fnal.gov/downloads/quick_ots_install.zip  \
wget https://cdcvs.fnal.gov/redmine/attachments/download/65977/quick_ots_install.zip \
    --no-check-certificate \
	--load-cookies=${cookief} \
	--save-cookies=${cookief} \
	--keep-session-cookies
unzip quick_ots_install.zip
cd ots


#update all
REPO_DIR="$(find srcs/ -maxdepth 1 -iname 'otsdaq*')"
		
for p in ${REPO_DIR[@]}; do
	if [ -d $p ]; then
		if [ -d $p/.git ]; then
		
			bp=$(basename $p)
			subfolder=${bp//-/_}			
			echo -e "UpdateOTS.sh [${LINENO}]  \t Repo directory found as: $bp $subfolder"
			
			cd $p
			git checkout . # fix repo by checking out missing folders 
			git pull
			cd -
		fi
	fi	   
done

#setup qualifiers
#rm -rf change_ots_qualifiers.sh
#cp srcs/otsdaq_utilities/tools/change_ots_qualifiers.sh .
# chmod 755 change_ots_qualifiers.sh
./srcs/otsdaq_utilities/tools/change_ots_qualifiers.sh DEFAULT DEFAULT

source setup_ots.sh


#clean compile
mrb z
mrb uc
source setup_ots.sh
mrb b

UpdateOTS.sh --tables #update tables and get reset_ots_tutorial.sh

# #remove self so users do not install twice!
rm -rf ../quick_ots_install.sh
rm -rf ../quick_ots_install.zip

wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/demo/revisions/develop/raw/tools/reset_ots_tutorial.sh \
    --no-check-certificate \
	--load-cookies=${cookief} \
	--save-cookies=${cookief} \
	--keep-session-cookies
chmod 755 reset_ots_tutorial.sh
export KDIALOG_ALWAYS_YES=1 #for reset tutorial to not use kdialog
./reset_ots_tutorial.sh
unset KDIALOG_ALWAYS_YES
UpdateOTS.sh --tables

echo -e "quick_ots_install.sh [${LINENO}]  \t =================="
echo -e "quick_ots_install.sh [${LINENO}]  \t quick_ots_install script done!"
echo -e "quick_ots_install.sh [${LINENO}]  \t"
echo -e "quick_ots_install.sh [${LINENO}]  \t Next time, cd to ${INSTALL_DIR}/ots:"
echo -e "quick_ots_install.sh [${LINENO}]  \t\t source setup_ots.sh     #########################################   #to setup ots"
echo -e "quick_ots_install.sh [${LINENO}]  \t\t mrb b                   #########################################   #for incremental build"
echo -e "quick_ots_install.sh [${LINENO}]  \t\t mrb z                   #########################################   #for clean build"
echo -e "quick_ots_install.sh [${LINENO}]  \t\t UpdateOTS.sh            #########################################   #to see update options"
echo -e "quick_ots_install.sh [${LINENO}]  \t\t ./reset_ots_tutorial.sh --list     	 ############################	#to see tutorial options"
echo -e "quick_ots_install.sh [${LINENO}]  \t\t ots -w                  #########################################   #to run ots in wiz(safe) mode"
echo -e "quick_ots_install.sh [${LINENO}]  \t\t ots                     #########################################   #to run ots in normal mode"

echo -e "quick_ots_install.sh [${LINENO}]  \t *******************************"
echo -e "quick_ots_install.sh [${LINENO}]  \t *******************************"


if [ $USER == "root" ]; then
	chown -R $FOR_USER ../ots
	chgrp -R $FOR_GROUP ../ots
fi

# cd $INSTALL_DIR


#cleanup any remaining credentials
rm -f /tmp/postdata$$ /tmp/at_p$$ $cookief $listf*



