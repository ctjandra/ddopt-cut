"""Data collection module. Responsible for collecting and aggregating data from the output files."""

import collections
import getpass
import subprocess
import shlex
import numpy
import re
import sys

try:
	import paramiko
except ImportError:
	print 'Warning: The library paramiko must be installed for remote file support; remote attempts will fail'



def run(command, ssh=None):
	"""Run a command, remotely or not"""
	if ssh:
		stdin, stdout, stderr = ssh.exec_command(command)
		for line in stderr: print line
		return stdout
	else:
		args = shlex.split(command)
		process = subprocess.Popen(args, stdout=subprocess.PIPE)
		return process.stdout


def filter_fail(filename, filename_filter):
	"""Check if filename fails the criteria in the filename filter"""
	if filename_filter is None:
		return False
	if not re.search(filename_filter, filename):
		return True
	return False


def filter_per_field_fail(field, filename, filename_filter_per_field):
	"""Check if filename fails the criteria in the field-specific filename filter"""
	if filename_filter_per_field is None:
		return False
	for filter_field, filename_filter in filename_filter_per_field.iteritems():
		if filter_field != field:
			continue
		if not re.search(filename_filter, filename):
			return True
	return False


def ssh_connect(username, hostname):
	"""Connect via SSH to another machine"""
	ssh = paramiko.SSHClient()
	ssh.load_system_host_keys()
	ssh.set_missing_host_key_policy(paramiko.WarningPolicy())
	try:
		ssh.connect(hostname, username=username)
	except paramiko.ssh_exception.SSHException: # try asking for a password
		pw = getpass.getpass('Password for %s@%s: ' % (username, hostname))
		ssh.connect(hostname, username=username, password=pw)
	return ssh


# Main function that should be called externally
def collect_data(fields, instance_index, path, userhost="", additional_fields={}, filename_filter=None, 
				 filename_filter_per_field=None, open_ssh_session=None):
	"""Collect data from the output files and return data and averaged data"""

	if path == "":
		print 'Error: Path is not specified'
		sys.exit(1)
	if userhost != "" and userhost.count('@') != 1:
		print 'Error: Host must be of form "username@hostname" or empty if local'
		sys.exit(1)

	# Set up SSH
	ssh = None
	if open_ssh_session != None:
		ssh = open_ssh_session
	else:
		if userhost != "":
			username, hostname = userhost.split('@')
			ssh = ssh_connect(username, hostname)

	# Format of data: data[key][field][instance] = value
	# key is everything except instance, taken from instance_index
	data = collections.defaultdict(lambda: collections.defaultdict(dict))

	# Format of avg_data: avg_data[key][field] = (value_avg, value_std)
	# If data is unavailable, return (None, None) so the data point is not plotted
	avg_data = collections.defaultdict(lambda: collections.defaultdict(lambda: (None, None)))

	# Runs commands for each field and collects all data into data dictionary
	# Warning: This assumes the grepped value is found and may produce an unexpected behavior otherwise
	#          (typically the field function will fail)
	instance_list = set([])
	for field, t in fields.iteritems():
		print 'Collecting field ' + field + '...'

		# Field is of the form (Bash command, post-processing function, True/False whether should be piped after cat)
		assert len(t) == 2 or len(t) == 3
		if len(t) == 2:
			fcmd, fn = t
			runcat = True
		else:
			fcmd, fn, runcat = t

		# Note that it is much faster to iterate in the server than locally.
		if runcat:
			command = 'bash -c \'for f in ' + path + '/*; do echo -n "`basename ${f}` "; cat ${f} | ' + fcmd + '; echo; done\''
		else:
			command = 'bash -c \'for f in ' + path + '/*; do echo -n "`basename ${f}` "; ' + fcmd + '; echo; done\''

		# Run command and process output
		out = run(command, ssh)
		for line in out:

			# Uncomment for debugging
			# print line

			if line == '' or len(line.strip().split(' ')) == 1: continue
			filename, value_str = line.strip('\n').split(' ', 1)

			# Check for allowed strings
			if filter_fail(filename, filename_filter): continue
			if filter_per_field_fail(field, filename, filename_filter_per_field): continue

			basename = filename.rsplit('.', 1)[0] # Cut off file extension
			fparams = basename.split('_')
			instance = fparams.pop(instance_index) # Extract instance number
			key = tuple(fparams) # Use rest as key

			instance_list.add(instance)

			# print key, field, instance, value_str
			data[key][field][instance] = fn(value_str)

	# Assert instances are consistent across fields
	for key, d in data.iteritems():
		for key2, d2 in d.iteritems():
			for instance in instance_list:
				if instance not in d2:
					print "Warning: Missing data: ",
					print instance, "not in", key, "for", key2,
					print "(" + path + ")"
					print "         Possibly due to time/memory limit. This data point will be ignored."
					# sys.exit(1)

	# Create additional fields based on collected data
	for add_field, fn in additional_fields.iteritems():
		for key, d in data.iteritems():
			for instance in instance_list:
				data[key][add_field][instance] = fn(d, instance)

	# Average out data across instances
	for key, d1 in data.iteritems():
		for field, d2 in d1.iteritems():
			if isinstance(d2.values()[0], list):
				for v in d2.values():
					assert isinstance(v, list) and len(v) == len(d2.values()[0]) # All elements must be lists of the *same length*
				avg_data[key][field] = (map(numpy.mean, zip(*d2.values())), map(numpy.std, zip(*d2.values())))
			else:
				avg_data[key][field] = (numpy.mean(d2.values()), numpy.std(d2.values()))

	if ssh and open_ssh_session == None: # if open session was given, keep it open
		ssh.close()

	# print data # Uncomment for debugging
	# print avg_data # Uncomment for debugging
	# for k, v in avg_data.iteritems():
	# 	print k, v

	return data, avg_data
