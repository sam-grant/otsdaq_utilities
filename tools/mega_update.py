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
cmd=os.environ['Products'];
print cmd
os.chdir(cmd);

if(os.path.isfile('pullProducts')):
	os.remove('pullProducts');
	
######################################################################
cmd='curl -O http://scisoft.fnal.gov/scisoft/bundles/tools/pullProducts';
print cmd
os.system(cmd);

######################################################################
cmd='chmod +x pullProducts';
print cmd
os.system(cmd);

########################################################################
#pull the latest version of artdaq_demo from scisoft
########################################################################
cmd = 'curl -s http://scisoft.fnal.gov/scisoft/bundles/artdaq_demo/ | grep id=\\"v | grep -oP \'(?<=id=\\")[^\\">]*\'';
print cmd;
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
artdaq_demo_version,err = process.communicate();
if(err != '' and not err.find('Total')):
	print "There was an error executing \"" + cmd + "\"";
	print "Error:\n" + err;

artdaq_demo_versions = artdaq_demo_version.rstrip().split();

#print artdaq_demo_versions;

########################################################################
#detect operating system
########################################################################
cmd = "uname -r";
print cmd
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
operatingSystem,err = process.communicate();
if(err != ''):
	print "There was an error executing \"" + cmd + "\"";
	print "Error:\n" + err;
#print out
if "el6" in operatingSystem:
	host_os = "slf6"
	xerces_os = "slf6"
elif "el7" in operatingSystem:
	host_os = "slf7"
	xerces_os = "sl7"


cmd='cd $Products';
print cmd
os.system(cmd)

for artdaq_demo_version in reversed(artdaq_demo_versions):
	print "Fetching products for artdaq_demo_version: " + artdaq_demo_version;
	cmd = "./pullProducts . " + host_os + " artdaq_demo-" + artdaq_demo_version + " " + artdaqDemoVersion;
	print cmd;
	process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
	operatingSystem,err = process.communicate();
	if(err != ''):
		print "There was an error executing \"" + cmd + "\"";
		print "Error:\n" + err;
		print "Trying to fetch an older artdaq_demo_version..."
	else:
		break;


########################################################################
#_____MRB
########################################################################
cmd = 'curl -s http://scisoft.fnal.gov/scisoft/packages/mrb/ | grep id=\\"v | grep -oP \'(?<=id=\\")[^\\">]*\' | tail -1';
print cmd
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
mrb_version,err = process.communicate();
if(err != '' and not err.find('Total')):
	print "There was an error executing \"" + cmd + "\"";
	print "Error:\n" + err;
mrb_version = mrb_version.rstrip();
#print "-" + mrb_version + "-"
mrb_period_version = mrb_version[1:].replace('_','.');
#print "-" + mrb_period_version + "-"

cmd = "curl http://scisoft.fnal.gov/scisoft/packages/mrb/" + mrb_version + "/mrb-" + mrb_period_version + "-noarch.tar.bz2|tar -jx";
print cmd
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
out,err = process.communicate();
if(err != '' and not err.find('Total')):
	print "There was an error executing \"" + cmd + "\"";
	print "Error: " + err;


########################################################################
#_____XERCES
########################################################################
cmd = 'curl -s http://scisoft.fnal.gov/scisoft/packages/xerces_c/ | grep id=\\"v | grep -oP \'(?<=id=\\")[^\\">]*\' | tail -1';
#print cmd
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
xerces_version,err = process.communicate();
if(err != '' and not err.find('Total')):
	print "There was an error executing \"" + cmd + "\"";
	print "Error:\n" + err;
xerces_version = xerces_version.rstrip();
xerces_period_version = xerces_version[1:].replace('_','.');

cmd = "curl http://scisoft.fnal.gov/scisoft/packages/xerces_c/" + xerces_version + "/xerces_c-" + xerces_period_version + "-" + xerces_os + "-x86_64-e10-prof.tar.bz2|tar -jx";
print cmd
process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
out,err = process.communicate();
if(err != '' and not err.find('Total')):
	print "There was an error executing \"" + cmd + "\"";
	print "Error:\n" + err;

########################################################################
# Removing all tar files
########################################################################
cmd="rm -f *.bz2 *.txt";
print cmd
os.system(cmd)






