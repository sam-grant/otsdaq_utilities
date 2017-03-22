#!/usr/bin/env python
import os
import subprocess

try:
	os.path.isdir(os.environ["Products"])
except KeyError:
	print "Products variable not set"
	exit(1)

#get pull products
os.chdir(os.environ["Products"])
os.system('wget http://scisoft.fnal.gov/scisoft/bundles/tools/pullProducts')
os.system('chmod +x pullProducts')

	
#pull the latest version of artdaq_demo from scisoft
artdaq_demo_version =  os.popen('(curl -s http://scisoft.fnal.gov/scisoft/bundles/artdaq_demo/ | grep id=\\"v | grep -oP \'(?<=id=\\")[^\\">]*\' | tail -1)').read().rstrip()

#detect operating system
operatingSystem = subprocess.check_output(['uname', '-a'])
#print operatingSystem
if "el6" in operatingSystem:
	host_os = "slf6"
	xerces_os = "slf6"
elif "el7" in operatingSystem:
	host_os = "slf7"
	xerces_os = "sl7"


os.system("cd $Products")
#print    ". /pullProducts . " + host_os + "artdaq_demo-" + artdaq_demo_version + " s41-e10 prof"
os.system(". /pullProducts . " + host_os + "artdaq_demo-" + artdaq_demo_version + " s41-e10 prof")

#_____MRB
mrb_version        = os.popen('(curl -s http://scisoft.fnal.gov/scisoft/packages/mrb/ | grep id=\\"v | grep -oP \'(?<=id=\\")[^\\">]*\' | tail -1)').read().rstrip()
mrb_period_version = os.popen('(curl -s http://scisoft.fnal.gov/scisoft/packages/mrb/ | grep id=\\"v | grep -oP \'(?<=id=\\"v)[^\\">]*\' | tail -1 | awk \'{gsub(\"_\",\".\"); print $0}\')').read().rstrip()

#print    "curl http://scisoft.fnal.gov/scisoft/packages/mrb/" + mrb_version + "/mrb-" + mrb_period_version + "-noarch.tar.bz2|tar -jx"
os.system("curl http://scisoft.fnal.gov/scisoft/packages/mrb/" + mrb_version + "/mrb-" + mrb_period_version + "-noarch.tar.bz2|tar -jx")


#_____XERCES
xerces_version        = os.popen('(curl -s http://scisoft.fnal.gov/scisoft/packages/xerces_c/ | grep id=\\"v | grep -oP \'(?<=id=\\")[^\\">]*\' | tail -1)').read().rstrip()
xerces_period_version = os.popen('(curl -s http://scisoft.fnal.gov/scisoft/packages/xerces_c/ | grep id=\\"v | grep -oP \'(?<=id=\\"v)[^\\">]*\' | tail -1 | awk \'{gsub(\"_\",\".\"); print $0}\')').read().rstrip()

#print    "curl http://scisoft.fnal.gov/scisoft/packages/xerces_c/" + xerces_version + "/xerces_c-" + xerces_period_version + "-" + xerces_os + "-x86_64-e10-prof.tar.bz2|tar -jx"
os.system("curl http://scisoft.fnal.gov/scisoft/packages/xerces_c/" + xerces_version + "/xerces_c-" + xerces_period_version + "-" + xerces_os + "-x86_64-e10-prof.tar.bz2|tar -jx")
os.system("rm -f *.bz2 *.txt")






