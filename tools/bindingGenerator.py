#!/usr/bin/env python

# Eventually this will be integrated with tasks.py

import sys
import re
import os
import json

scriptMethodSearch = re.compile("([ \t]+)/\*\*\*(.*?)\*/\n\s+ENGINE_SCRIPT_METHOD\((\w+)\);\n", re.DOTALL and re.DOTALL)

def stypeToCPPType(typeName):
	if typeName == "string" or typeName == "String":
		return "std::string"
	elif typeName == "int":
		return "int"
	elif typeName == "number" or typeName == "Number":
		return "double"
	elif typeName == "bool" or typeName == "boolean":
		return "bool"
	elif typeName == "Function":
		return "v8::Handle<v8::Function>"
	elif typeName == "*":
		return "v8::Handle<v8::Value>"
	elif typeName == "Object":
		return "v8::Handle<v8::Object>"
	elif typeName == "...*":
		return "v8::Handle<v8::Value>*"
	elif typeName == "void":
		return "void"
	else:
		#print "[bindingGenerator] WARN: Bad typeName: " + typeName
		return "v8::Handle<v8::Value>"

def marshalSTypeToCPPValue(typeName, argPosition):
	if typeName == "string" or typeName == "String":
		return "args.StringValue(" + str(argPosition) + ")"
	elif typeName == "int":
		return "args.Int32Value(" + str(argPosition) + ")"
	elif typeName == "number" or typeName == "Number":
		return "args.NumberValue(" + str(argPosition) + ")"
	elif typeName == "bool" or typeName == "boolean":
		return "args.BooleanValue(" + str(argPosition) + ")"
	elif typeName == "Function":
		return "args[" + str(argPosition) + "]->ToFunction()"
	elif typeName == "Object":
		return "args[" + str(argPosition) + "]->ToObject()"
	elif typeName == "*":
		return "args[" + str(argPosition) + "]"
	elif typeName == "...*":
		return "args.Slice(" + str(argPosition) + ")"
	else:
		sys.stderr.write("[bindingGenerator] WARN: Bad typeName: " + typeName + "\n")
		return "args[" + str(argPosition) + "]"

def marshalSTypeToCPPCheck(typeName, argPosition):
	if typeName == "string" or typeName == "String":
		return "args[" + str(argPosition) + "]->IsString()"
	elif typeName == "int":
		return "args[" + str(argPosition) + "]->IsInt32()"
	elif typeName == "number" or typeName == "Number":
		return "args[" + str(argPosition) + "]->IsNumber()"
	elif typeName == "bool" or typeName == "boolean":
		return "args[" + str(argPosition) + "]->IsBoolean()"
	elif typeName == "Function":
		return "args[" + str(argPosition) + "]->IsFunction()"
	elif typeName == "Object":
		return "args[" + str(argPosition) + "]->IsObject()"
	elif typeName == "*":
		return "true"
	elif typeName == "...*":
		return "true"
	else:
		#print "[bindingGenerator] WARN: Bad typeName: " + typeName
		return "true"

def compileMethod(whitespace, jsonBlob, methodName):
	ret = ""

	# write generated method signature
	ret += whitespace + "_ENGINE_SCRIPT_METHOD_SIGNATURE(" + methodName + ") {" + "\n"
	ret += whitespace + "    ScriptingManager::Arguments args(_args);" + "\n"

	# assert on flags 
	argsLength = len(jsonBlob["args"])
	if argsLength > 0:
		ret += whitespace + "    if (args.AssertCount(" + str(argsLength) + ")) return;" + "\n"
	for flag in jsonBlob["flags"]:
		if flag == "requiresGraphicsContext":
			ret += whitespace + "    if (args.Assert(HasGLContext(), \"No OpenGL Context\")) return;" + "\n"

	methodCallArgs = []
	methodSignature = ""
	methodArgs = []

	methodSignature += (stypeToCPPType(jsonBlob["returns"]) if jsonBlob.has_key("returns") else "void") + " " + methodName + "("

	bindingType = jsonBlob["bindingType"] if "bindingType" in jsonBlob else "smart"
	signaturesOnly = jsonBlob["signaturesOnly"] if "signaturesOnly" in jsonBlob else False

	if bindingType != "smart_noContext":
		methodArgs += ["ScriptingManager::Arguments& args"]
		methodCallArgs += ["args"]

	index = 0
	for arg in jsonBlob["args"]:
		typeName, argName, helpText = arg
		methodArgs += [stypeToCPPType(typeName) + " " + argName]
		methodCallArgs += [argName]
		cppCheck = marshalSTypeToCPPCheck(typeName, index)
		if cppCheck is not "true":
			ret += whitespace + "    if (args.Assert(" + cppCheck + \
				", \"" + helpText + "\")) return;" + "\n"
		ret += whitespace + "    " + stypeToCPPType(typeName) + " " + argName + " = " + \
			marshalSTypeToCPPValue(typeName, index) + ";" + "\n"
		index += 1

	methodSignature += ", ".join(methodArgs) + ");"

	# write method tail
	if jsonBlob.has_key("returns"):
		ret += whitespace + "    args.SetReturnValue(" + methodName + "(" + ", ".join(methodCallArgs) + "));" + "\n";
	else:
		ret += whitespace + "    " + methodName + "(" + ", ".join(methodCallArgs) +  ");" + "\n";
	ret += whitespace + "}" + "\n\n"

	if signaturesOnly:
		return whitespace + methodSignature
	else:
		return whitespace + methodSignature + "\n\n" + ret

def parseMethod(m):
	whitespace = m.group(1)
	jsonBlob = json.loads(m.group(2))
	methodName = m.group(3)
	return compileMethod(whitespace, jsonBlob, methodName)

def parseFile(path, filename):
	lines = []
	with open(path, "r") as f:
		lines += f.readlines()
	linesStr = "".join(lines)
	outStr = re.sub(scriptMethodSearch, parseMethod, linesStr)
	with open("src/gen/" + filename, "w") as f:
		f.write("// AUTOGENERATED SCRIPT BINDING FILE\n");
		f.write(outStr)