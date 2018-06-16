using AsarCLR;
using System;
using System.Reflection;

class AsarTest
{
	static int Main(string[] args)
	{
		// TODO: actual tests here, right now i'm just figuring out C#
		Asar.init();
		Console.WriteLine("Asar version: {0}", Asar.version());
		Console.WriteLine("Asar API version: {0}", Asar.apiversion());
		// having some fun with reflection
		// see guys, private fields are useless!
		int expectedapiversion = (int)typeof(Asar).GetField("expectedapiversion", BindingFlags.NonPublic | BindingFlags.Static).GetValue(null);
		Console.WriteLine("C# wrapper version: {0}", expectedapiversion);
		if(Asar.apiversion() != expectedapiversion) {
			Console.WriteLine("C# wrapper version and Asar API version don't match");
			return 0;
		}
		return 0;
	}
}