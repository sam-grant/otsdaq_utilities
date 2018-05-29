#!/bin/bash

# build in $WORKSPACE/build
# copyback directory is $WORKSPACE/copyBack

usage()
{
  cat 1>&2 <<EOF
Usage: $(basename ${0}) [-h]

Options:

  -h    This help.

EOF
}

working_dir=${WORKSPACE}
version=${VERSION}
qual_set="${QUAL}"
build_type=${BUILDTYPE}

case ${qual_set} in
    s64:e15)
        basequal=e15
        squal=s64
        artver=v2_10_02
        ;;
	s50:e14)
		basequal=e14
		squal=s50
		artver=v2_07_03
		;;
    s46:e10)
        basequal=e10
        squal=s46
        artver=v2_06_01
        ;;
    *)
	echo "unexpected qualifier set ${qual_set}"
	usage
	exit 1
esac

wget https://cdcvs.fnal.gov/redmine/projects/otsdaq/repository/revisions/${version}/raw/ups/product_deps && \
artdaq_ver=`grep "^artdaq " product_deps|awk '{print $2}'` || \
$(echo "Unexpected version ${version}" && usage && exit 1)

case ${build_type} in
    debug) ;;
    prof) ;;
    *)
	echo "ERROR: build type must be debug or prof"
	usage
	exit 1
esac

dotver=`echo ${version} | sed -e 's/_/./g' | sed -e 's/^v//'`
art_dotver=`echo ${artver} | sed -e 's/_/./g' | sed -e 's/^v//'`
artdaq_dotver=`echo ${artdaq_ver} | sed -e 's/_/./g' | sed -e 's/^v//'`

echo "building the otsdaq distribution for ${version} ${dotver} ${qual_set} ${build_type}"

OS=`uname`
if [ "${OS}" = "Linux" ]
then
    flvr=slf`lsb_release -r | sed -e 's/[[:space:]]//g' | cut -f2 -d":" | cut -f1 -d"."`
else 
  echo "ERROR: unrecognized operating system ${OS}"
  exit 1
fi
echo "build flavor is ${flvr}"
echo ""

qualdir=`echo ${qual_set} | sed -e 's%:%-%'`

set -x

srcdir=${working_dir}/source
blddir=${working_dir}/build
# start with clean directories
rm -rf ${blddir}
rm -rf ${srcdir}
rm -rf $WORKSPACE/copyBack 
# now make the dfirectories
mkdir -p ${srcdir} || exit 1
mkdir -p ${blddir} || exit 1
mkdir -p $WORKSPACE/copyBack || exit 1

cd ${blddir} || exit 1
curl --fail --silent --location --insecure -O http://scisoft.fnal.gov/scisoft/bundles/tools/pullProducts || exit 1
chmod +x pullProducts
curl --fail --silent --location --insecure -O http://scisoft.fnal.gov/scisoft/bundles/tools/buildFW || exit 1
chmod +x buildFW

mv ${blddir}/*source* ${srcdir}/

cd ${blddir} || exit 1
# pulling binaries is allowed to fail
# we pull what we can so we don't have to build everything
./pullProducts ${blddir} ${flvr} art-${artver} ${basequal} ${build_type}
./pullProducts ${blddir} ${flvr} artdaq-${artdaq_ver} ${squal}-${basequal} ${build_type}
./pullProducts ${blddir} ${flvr} otsdaq-${version} ${squal}-${basequal} ${build_type}
# remove any artdaq_demo entities that were pulled so it will always be rebuilt
if [ -d ${blddir}/otsdaq ] || [ -d ${blddir}/otsdaq_utilities ] || [ -d ${blddir}/otsdaq_components ]; then
  echo "Removing ${blddir}/otsdaq*"
  rm -rf ${blddir}/otsdaq*
  if [ `ls -l ${blddir}/otsdaq*.tar.bz2 | wc -l` -gt 0 ]; then rm -fv ${blddir}/otsdaq*.tar.bz2; fi
fi 

echo
echo "begin build"
echo
./buildFW -t -b ${basequal} -s ${squal} ${blddir} ${build_type} otsdaq-${version} || \
 { mv ${blddir}/*.log  $WORKSPACE/copyBack/
   exit 1 
 }

source ${blddir}/setups
upsflavor=`ups flavor`
echo "Fix Manifests"

artManifest=`ls ${blddir}/art-*_MANIFEST.txt|tail -1`
artdaqManifest=`ls ${blddir}/artdaq-*_MANIFEST.txt|tail -1`
otsdaqManifest=`ls ${blddir}/otsdaq-*_MANIFEST.txt|tail -1`

cat ${artManifest} >>${artdaqManifest}
cat ${artdaqManifest} >>${otsdaqManifest}
cat ${artdaqManifest}|grep -v source|sort|uniq >>${artdaqManifest}.tmp
mv ${artdaqManifest}.tmp ${artdaqManifest}
cat ${otsdaqManifest}|grep -v source|sort|uniq >>${otsdaqManifest}.tmp
mv ${otsdaqManifest}.tmp ${otsdaqManifest}

cat ${blddir}/art-${art_dotver}-${upsflavor}-${basequal}-${build_type}_MANIFEST.txt >>${blddir}/otsdaq-${dotver}-${upsflavor}-${squal}-${basequal}-${build_type}_MANIFEST.txt
cat ${blddir}/artdaq-${artdaq_dotver}-${upsflavor}-${squal}-${basequal}-${build_type}_MANIFEST.txt >>${blddir}/otsdaq-${dotver}-${upsflavor}-${squal}-${basequal}-${build_type}_MANIFEST.txt
cat ${blddir}/otsdaq-${dotver}-${upsflavor}-${squal}-${basequal}-${build_type}_MANIFEST.txt|grep -v source|sort|uniq >>${blddir}/otsdaq-${dotver}-${upsflavor}-${squal}-${basequal}-${build_type}_MANIFEST.txt.tmp
mv ${blddir}/otsdaq-${dotver}-${upsflavor}-${squal}-${basequal}-${build_type}_MANIFEST.txt{.tmp,}

echo
echo "move files"
echo
mv ${blddir}/*.bz2  $WORKSPACE/copyBack/
mv ${blddir}/*.txt  $WORKSPACE/copyBack/
mv ${blddir}/*.log  $WORKSPACE/copyBack/

echo
echo "cleanup"
echo
rm -rf ${blddir}
rm -rf ${srcdir}

exit 0
