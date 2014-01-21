#!/usr/bin/env python

# Engine2D Task Scripts
# Built for Python 2.7

import sys;
import os;
import subprocess;

PROJECT_ROOT = 0;
PROJECT_BUILD_PATH = 1;

commands = {};

class command(object):
	def __init__(self, requires=[], usage="TODO: "):
		self.requires = requires;
		self.usage = usage;
	
	def __call__(self, func):
		commands[func.__name__] = func;
		commands[func.__name__].requires = self.requires;
		commands[func.__name__].usage = self.usage;
		commands[func.__name__].run = False;
		return func;

def get_exe_name():
	if sys.platform == "darwin":
		return "engine2D";

def resolve_path(base, path=""):
	if base == PROJECT_ROOT:
		return os.path.relpath(path);
	elif base == PROJECT_BUILD_PATH:
		return os.path.join(resolve_path(PROJECT_ROOT, "build/Default"), path)
	else:
		raise "Invalid base";

def require_in_path(arr):
	for x in arr:
		pass;
	return True;

def shell_command(cmd, throw=True):
	print("shell : %s" % (cmd));
	if throw:
		subprocess.check_call(cmd);
	else:
		subprocess.call(cmd);

@command(usage="Downloads a local copy of SVN if we can't find one")
def fetch_svn():
	try:
		require_in_path(["svn"]);
	except Exception, e:
		# check for SVN in path, if we can find it then put it in the path
		print("SVN not found: " + str(e));
		# download a standalone copy of SVN and put it in third_party/svn then put it in the path
		raise e
	# at this point we have SVN in the path and can download GYP
	pass;

@command(requires=["fetch_svn"], usage="Downloads the latest version of GYP using SVN")
def fetch_gyp():
	# check for GYP in third_party first
	# if it's not there then download it using SVN
	pass;

@command(requires=["fetch_gyp"], usage="Builds platform project files")
def gyp():
	# run GYP
	shell_command([
			resolve_path(PROJECT_ROOT, "third_party/gyp/gyp"),
			"--depth=0",
			resolve_path(PROJECT_ROOT, "engine2D.gyp")
		]);

@command(requires=["gyp"], usage="Open's the platform specfic IDE")
def ide():
	print("Opening IDE");

@command(requires=["gyp"], usage="Builds executables")
def build_bin():
	if sys.platform == "darwin": # OSX
		shell_command(["xcodebuild"]);

@command(requires=["build_bin"], usage="Copys support files to the build path")
def build_env():
	print("Copying Enviroment");

@command(requires=["build_env"], usage="Runs the engine in Development Mode")
def run():
	shell_command([
			resolve_path(PROJECT_BUILD_PATH, get_exe_name()),
			"-devmode",
			"-debug"
		]);

@command(usage="Prints the commandName and usage for each command")
def help():
	for x in commands:
		print("%s | %s" % (x, commands[x].usage));

@command(usage="Prints the current building enviroment")
def env():
	print("sys.platform = %s" % (sys.platform));

def run_command(cmdName):
	# run all required commands
	for cmd in commands[cmdName].requires:
		if not commands[cmdName].run: ## make sure the command is not run twice
			run_command(cmd);

	# finaly run the commadn the user is intrested in
	print("==== Running: %s ====" % (cmdName));
	commands[cmdName]();
	commands[cmdName].run = True;

def main(args):
	# require python version 2.7

	# get the command and run it
	if len(args) < 2:
		print("tasks.py : usage: ./tasks.py [commandName]");
	elif not args[1] in commands:
		print("tasks.py : command %s not found" % (args[1]));
	else:
		run_command(args[1]);

if __name__ == '__main__':
  main(sys.argv);