import asar

asar.init()
print("Asar version: {}".format(asar.version()))
print("Asar API version: {}".format(asar.apiversion()))
print("Py wrapper version: {}".format(asar._target_api_ver))
if asar.apiversion() != asar._target_api_ver:
	print("Py wrapper version and Asar API version don't match")

# TODO: actual tests here
