#!/usr/bin/env python
import os
import subprocess

artdaqDemoVersion = "s41-e10 prof";

try:
	os.path.isdir(os.environ['Products']);
except KeyError:
	print "Products variable not set";
	exit(1);

########################################################################
#get pull products
########################################################################
os.chdir(os.environ['Products']);
if(os.path.isfile('pullProducts')):
	os.remove('pullProducts');
os.system('wget http://scisoft.fnal.gov/scisoft/bundles/tools/pullProducts');
os.system('chmod +x pullProducts');

########################################################################
#pull the latest version of artdaq_demo from scisoft
########################################################################
cmd = 'curl -s http://scisoft.fnal.gov/scisoft/bundles/artdaq_demo/ | grep id=\\"v | grep -oP \'(?<=id=\\")[^\\">]*\' | tail -1';
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
artdaq_demo_version,err = process.communicate();
if(err != ''):
	print "There was an error executing \"" + cmd + "\"";
	print "Error: " + err;

artdaq_demo_version = artdaq_demo_version.rstrip();

print artdaq_demo_version;

########################################################################
#detect operating system
########################################################################
cmd = "uname -r";
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
operatingSystem,err = process.communicate();
if(err != ''):
	print "There was an error executing \"" + cmd + "\"";
	print "Error: " + err;
#print out
if "el6" in operatingSystem:
	host_os = "slf6"
	xerces_os = "slf6"
elif "el7" in operatingSystem:
	host_os = "slf7"
	xerces_os = "sl7"


os.system("cd $Products")
cmd = "./pullProducts . " + host_os + " artdaq_demo-" + artdaq_demo_version + " " + artdaqDemoVersion;
os.system(cmd);

########################################################################
#_____MRB
########################################################################
cmd = 'curl -s http://scisoft.fnal.gov/scisoft/packages/mrb/ | grep id=\\"v | grep -oP \'(?<=id=\\")[^\\">]*\' | tail -1';
print cmd
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
mrb_version,err = process.communicate();
if(err != ''):
	print "There was an error executing \"" + cmd + "\"";
	print "Error: " + err;
mrb_version = mrb_version.rstrip();
print "-" + mrb_version + "-"
mrb_period_version = mrb_version[1:].replace('_','.');
print "-" + mrb_period_version + "-"

cmd = "curl http://scisoft.fnal.gov/scisoft/packages/mrb/" + mrb_version + "/mrb-" + mrb_period_version + "-noarch.tar.bz2|tar -jx";
print cmd
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
out,err = process.communicate();
if(err != ''):
	print "There was an error executing \"" + cmd + "\"";
	print "Error: " + err;


########################################################################
#_____XERCES
########################################################################
cmd = 'curl -s http://scisoft.fnal.gov/scisoft/packages/xerces_c/ | grep id=\\"v | grep -oP \'(?<=id=\\")[^\\">]*\' | tail -1';
#print cmd
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
xerces_version,err = process.communicate();
if(err != ''):
	print "There was an error executing \"" + cmd + "\"";
	print "Error: " + err;
xerces_version = xerces_version.rstrip();
xerces_period_version = xerces_version[1:].replace('_','.');
cmd = "curl http://scisoft.fnal.gov/scisoft/packages/xerces_c/" + xerces_version + "/xerces_c-" + xerces_period_version + "-" + xerces_os + "-x86_64-e10-prof.tar.bz2|tar -jx";
print cmd
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
out,err = process.communicate();
if(err != ''):
	print "There was an error executing \"" + cmd + "\"";
	print "Error: " + err;

########################################################################
# Removing all tar files
########################################################################
os.system("rm -f *.bz2 *.txt")






