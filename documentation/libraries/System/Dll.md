## System.Dll
Dll library class provides API for executing foreign C code from MSL using dll libraries. MSL System library requires **msl_system.dll** fule to function and uses this class to load and call C/C++ functions during runtime. If you need to create your own dll file for MSL, go to the dll files documentation. System dll files can be found in the /bin directory. This article explains how to load dll file and call its functions inside MSL language:
```cs
namespace System
{
    public static class Dll
    {
        public static function LoadLibrary(path);
        public static function FreeLibrary(path);
        public static function Call(module, function);
        public static function Call(module, function, arg1);
        // ... Dll.Call method overloads with more arguments
    }
}
```
### Dll.LoadLibrary(path)
Loads dll library using path provided. If dll was successfully loaded, returns true, returns false either:
```cs
var path = "msl_system.dll";
var loaded = Dll.LoadLibrary(path);
if (loaded)
    Console.PrintLine(path + " loaded");
else
    Console.PrintLine(path + " not loaded");
```
### Dll.FreeLibrary
Frees dll library by path provided. If library was not loaded or was already freed, nothing happens:
```cs
Dll.LoadLibrary("msl_system.dll"); // load library
var t = Dll.Call("msl_system.dll", "MathSqrt", 6.25); // t = 2.5, see Dll.Call method reference
Dll.FreeLibrary("msl_system.dll"); // free library after usage
```
*Note that all dlls are automatically freed after VM instance destruction*.
### Dll.Call(module, function, ...)
Calls function by its name in module provided by its path. This method accepts from 0 arguments up to any amount depending on the VM implementation. If any errors occurred during dll call, error is generated. If dll function has invalid or cause internal errors, behavior is undefined and can lead to VM crash:
```cs
Dll.LoadLibrary("msl_system.dll");
var text = Dll.Call("msl_system.dll", "ConsoleReadLine"); // call with 0 arguments
Dll.Call("msl_system.dll", "ConsolePrintLine", text); // call with 1 argument
Dll.FreeLibrary("msl_system.dll");
```
