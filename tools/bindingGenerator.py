#!/usr/bin/env python

# Eventually this will be integrated with tasks.py

import sys
import re
import os
import json

scriptMethodSearch = re.compile("([ \t]+)/\*\*\*(.*?)\*/\n\s+ENGINE_SCRIPT_METHOD\((\w+)\);\n", re.DOTALL and re.DOTALL)

class MarshalType:
	def __init__(self, validate, sType, value, check):
		self.validate = validate
		self.sType = sType
		self.check = check
		self.value = value

defaultType = MarshalType(lambda typeName: typeName == "*", "v8::Handle<v8::Value>",
				lambda argPosition: "args[" + str(argPosition) + "]",
				lambda argPosition: "true")

marshalTypes = [
	# String
	MarshalType(lambda typeName: typeName == "string" or typeName == "String", "std::string",
		lambda argPosition: "args.StringValue(" + str(argPosition) + ")",
		lambda argPosition: "args[" + str(argPosition) + "]->IsString()"),

	# Int32
	MarshalType(lambda typeName: typeName == "int", "int",
		lambda argPosition: "args.Int32Value(" + str(argPosition) + ")",
		lambda argPosition: "args[" + str(argPosition) + "]->IsInt32()"),
	
	# Number
	MarshalType(lambda typeName: typeName == "number" or typeName == "Number", "double",
		lambda argPosition: "args.NumberValue(" + str(argPosition) + ")",
		lambda argPosition: "args[" + str(argPosition) + "]->IsNumber()"),
	
	# Boolean
	MarshalType(lambda typeName: typeName == "bool" or typeName == "boolean", "bool",
		lambda argPosition: "args.BooleanValue(" + str(argPosition) + ")",
		lambda argPosition: "args[" + str(argPosition) + "]->IsBoolean()"),
	
	# Function
	MarshalType(lambda typeName: typeName == "Function", "v8::Handle<v8::Function>",
		lambda argPosition: "args[" + str(argPosition) + "]->ToFunction()",
		lambda argPosition: "args[" + str(argPosition) + "]->IsFunction()"),

	MarshalType(lambda typeName: typeName == "Object", "v8::Handle<v8::Object>",
		lambda argPosition: "args[" + str(argPosition) + "]->ToObject()",
		lambda argPosition: "args[" + str(argPosition) + "]->IsObject()"),
	
	MarshalType(lambda typeName: typeName == "...*", "v8::Handle<v8::Value>*",
		lambda argPosition: "args.Slice(" + str(argPosition) + ")",
		lambda argPosition: "true"),
	
	MarshalType(lambda typeName: typeName == "void", "void",
		lambda argPosition: "true",
		lambda argPosition: "true")
]

def getMarshalType(typeName):
	for mType in marshalTypes:
		if mType.validate(typeName):
			return mType
	return defaultType

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

	methodSignature += (getMarshalType(jsonBlob["returns"]).sType if jsonBlob.has_key("returns") else "void") + " " + methodName + "("

	bindingType = jsonBlob["bindingType"] if "bindingType" in jsonBlob else "smart"

	if bindingType == "none":
		return "// bindingGenerator.py: Skipped \"" + methodName + "\" due to @bind none\n"

	signaturesOnly = jsonBlob["signaturesOnly"] if "signaturesOnly" in jsonBlob else False

	condition = jsonBlob["condition"] if "condition" in jsonBlob else ""

	if bindingType != "smart_noContext":
		methodArgs += ["ScriptingManager::Arguments& args"]
		methodCallArgs += ["args"]

	if condition != "":
		ret += whitespace + "    if (args.Assert(" + condition + ", \"" + methodName + " requires " + condition + "\")) return;" + "\n"

	index = 0
	for arg in jsonBlob["args"]:
		typeName, argName, helpText = arg
		marshalType = getMarshalType(typeName)
		methodArgs += [marshalType.sType + " " + argName]
		methodCallArgs += [argName]
		cppCheck = marshalType.check(index)
		if len(helpText) > 0 and helpText[0] == "-":
			helpText = helpText[1:].strip()
		if cppCheck is not "true":
			ret += whitespace + "    if (args.Assert(" + cppCheck + \
				", \"" + helpText.replace("\"", "\\\"") + "\")) return;" + "\n"
		ret += whitespace + "    " + marshalType.sType + " " + argName + " = " + \
			marshalType.value(index) + ";" + "\n"
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