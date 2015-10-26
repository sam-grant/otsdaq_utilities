# This script is expected to be in otsdaq utilities repository in a specific directory
# but it can be executed from any path (do not source it, execute with ./ )
#
# ./path/to/script/checkinAll.sh "comment for git commit"
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
    echo "Note: git status will be logged here: $CURRENT_AWESOME_BASE/checkinAll.log"
    exit
fi

echo "Finding paths..."

SCRIPT_DIR="$( 
  cd "$(dirname "$(readlink "$0" || printf %s "$0")")"
  pwd -P 
)"

echo "Script directory found as: $SCRIPT_DIR"

REPO_DIR="$(find $SCRIPT_DIR/../../ -maxdepth 1 -iname 'otsdaq*')"

for p in ${REPO_DIR[@]}; do
    if [ -d $p ]; then
	echo "Repo directory found as: $(basename $p)"
    fi	   
done


echo
echo "=================="

echo "Git comment '$1'"
echo "Status will be logged here: $CURRENT_AWESOME_BASE/checkinAll.log"


echo
echo "=================="

for p in ${REPO_DIR[@]}; do
    if [ -d $p ]; then
	echo "Checking in $p"
	cd $p
	git pull
	echo "==================" > $CURRENT_AWESOME_BASE/checkinAll.log
	pwd >> $CURRENT_AWESOME_BASE/checkinAll.log
	git status &>> $CURRENT_AWESOME_BASE/checkinAll.log
	git commit -m "$1" .
	git push   
	cd $CURRENT_AWESOME_BASE
    fi	   
done

exit

CURRENT_AWESOME_BASE=$PWD
if [ "x$OTSDAQ_DIR" == "x" ]; then
    echo "OTSDAQ_DIR is not defined. Please run setup scripts"
    return
fi

if [ -d "$OTSDAQ_DIR" ]; then
    echo "Checking in $OTSDAQ_DIR"
    cd $OTSDAQ_DIR
    git pull
    echo "==================" > $CURRENT_AWESOME_BASE/checkinAll.log
    pwd >> $CURRENT_AWESOME_BASE/checkinAll.log
    git status &>> $CURRENT_AWESOME_BASE/checkinAll.log
    git commit -m "$1" .
    git push   
    cd $CURRENT_AWESOME_BASE
else
    echo "Path doesn't exist $OTSDAQ_DIR"
fi

echo
echo "=================="

if [ -d "$OTSDAQ_UTILITIES_DIR" ]; then
    echo "Checking in $OTSDAQ_UTILITIES_DIR"
    cd $OTSDAQ_UTILITIES_DIR
    git pull
    echo "==================" >> $CURRENT_AWESOME_BASE/checkinAll.log
    pwd >> $CURRENT_AWESOME_BASE/checkinAll.log
    git status &>> $CURRENT_AWESOME_BASE/checkinAll.log
    git commit -m "$1" .
    git push   
    cd $CURRENT_AWESOME_BASE
else
    echo "Path doesn't exist $OTSDAQ_UTILITIES_DIR"
fi

echo
echo "=================="

if [ -d "$OTSDAQ_DEMO_DIR" ]; then
    echo "Checking in $OTSDAQ_DEMO_DIR"
    cd $OTSDAQ_DEMO_DIR
    git pull
    echo "==================" >> $CURRENT_AWESOME_BASE/checkinAll.log
    pwd >> $CURRENT_AWESOME_BASE/checkinAll.log
    git status &>> $CURRENT_AWESOME_BASE/checkinAll.log
    git commit -m "$1" .
    git push   
    cd $CURRENT_AWESOME_BASE
else
    echo "Path doesn't exist $OTSDAQ_DEMO_DIR"
fi

echo
echo "=================="

if [ -d "$OTSDAQ_DEMO_DIR/../otsdaq-firmware" ]; then
    echo "Checking in $OTSDAQ_DEMO_DIR/../otsdaq-firmware"
    cd $OTSDAQ_DEMO_DIR/../otsdaq-firmware
    git pull
    pwd >> $CURRENT_AWESOME_BASE/checkinAll.log
    git status &>> $CURRENT_AWESOME_BASE/checkinAll.log
    git commit -m "$1" .
    git push   
    cd $CURRENT_AWESOME_BASE
else
    echo "Path doesn't exist $OTSDAQ_DEMO_DIR/../otsdaq-firmware"
fi

echo
echo "=================="

if [ -d "$OTSDAQ_DEMO_DIR/../otsdaq-web" ]; then
    echo "Checking in $OTSDAQ_DEMO_DIR/../otsdaq-web"
    cd $OTSDAQ_DEMO_DIR/../otsdaq-web
    git pull
    echo "==================" >> $CURRENT_AWESOME_BASE/checkinAll.log
    pwd >> $CURRENT_AWESOME_BASE/checkinAll.log
    git status &>> $CURRENT_AWESOME_BASE/checkinAll.log
    git commit -m "$1" .
    git push   
    cd $CURRENT_AWESOME_BASE
else
    echo "Path doesn't exist $OTSDAQ_DEMO_DIR/../otsdaq-web"
fi

echo
echo "=================="

echo "Git comment '$1'"
echo "Status will be logged here: $CURRENT_AWESOME_BASE/checkinAll.log"


echo
echo "=================="
echo
echo "=================="
echo "Awesome script done"
echo "*******************************"
echo "*******************************"