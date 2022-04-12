import zipfile
import sys
import urllib.request
import os

if len(sys.argv) >= 2:
	filename = "asar" + sys.argv[1] + ".zip"
else:
	filename = "asar_windows_x86.zip"

if os.path.exists("asar/Release/asar-standalone.exe"):
	binary_path = "asar/Release"
elif os.path.exists("asar/Debug/asar-standalone.exe"):
    binary_path = "asar/Debug"
else:
	print("Cannot find asar-standalone.exe, please make sure you've built the project first!")
	sys.exit(1)

zipf = zipfile.ZipFile(filename, 'x', compression=zipfile.ZIP_DEFLATED)

f = open(binary_path+"/asar-standalone.exe", "rb")
exe_data = f.read()
f.close()
f = open(binary_path+"/asar.dll", "rb")
dll_data = f.read()
f.close()

zipf.writestr("asar.exe", exe_data)
zipf.writestr("dll/asar.dll", dll_data)

for (dirpath, dirnames, filenames) in os.walk("docs"):
	for x in filenames:
		zipf.write(dirpath + "/" + x)

for (dirpath, dirnames, filenames) in os.walk("ext"):
	for x in filenames:
		zipf.write(dirpath + "/" + x)

zipf.write("README.txt")
zipf.write("LICENSE")
zipf.write("license-gpl.txt")
zipf.write("license-lgpl.txt")
zipf.write("license-wtfpl.txt")

for (dirpath, dirnames, filenames) in os.walk("src/asar-dll-bindings"):
	for x in filenames:
		zipf.write(dirpath+"/"+x, dirpath.replace("src/asar-dll-bindings", "dll/bindings")+"/"+x)
