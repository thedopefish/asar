import unittest
import asar

class AsarTest(unittest.TestCase):
	def setUp(self):
		asar.init()

	def tearDown(self):
		asar.close()

	def testVersion(self):
		self.assertEqual(asar.version(), 10600)
		self.assertEqual(asar.apiversion(), 303)

	def testBasicMemoryFiles(self):
		patch = b"db $01"
		ok, out = asar.patch("patch", b"", memory_files={"patch": patch})
		# not using assertTrue(ok) because i want unittest to print the errors
		self.assertEqual(asar.geterrors(), [])
		self.assertEqual(out, b"\x01")

	def testGetLabels(self):
		patch = b"""
norom
org $0
label1:
db $42,$42
label2:
"""
		ok, out = asar.patch("patch", b"", memory_files={"patch": patch})
		self.assertEqual(asar.geterrors(), [])
		expected_labels = {
			"label1": 0,
			"label2": 2
		}
		self.assertEqual(asar.getalllabels(), expected_labels)

	def testMemoryFileIncludes(self):
		patch_a = b"""
incsrc "b"
incbin "c"
"""
		patch_b = b'incsrc "path/with/slashes"'
		bin_c = b'just some data'
		another_patch = b'incsrc "../one_more"'
		last = b"db $00"
		mem_files = {
			"patch": patch_a,
			"b": patch_b,
			"c": bin_c,
			"path/with/slashes": another_patch,
			"path/one_more": last
		}
		ok, out = asar.patch("patch", b"", memory_files=mem_files)
		self.assertEqual(asar.geterrors(), [])
		self.assertEqual(out, b"\x00just some data")

	def testChecksumOverrideOn(self):
		patch = b"norom : org $FFFF : db $00"
		ok, out = asar.patch("patch", b"", memory_files={"patch":patch}, override_checksum=True)
		self.assertEqual(asar.geterrors(), [])
		self.assertEqual(out, b"\x00"*0xFFDC + b"\x01\xfe\xfe\x01" + b"\x00"*0x20)

	def testChecksumOverrideOff(self):
		patch = b"lorom : org $FFFF : db $00"
		ok, out = asar.patch("patch", b"", memory_files={"patch":patch}, override_checksum=False)
		self.assertEqual(asar.geterrors(), [])
		self.assertEqual(out, b"\x00"*0x8000)


if __name__ == '__main__':
	unittest.main()
