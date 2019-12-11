## System.GC
GC library class provide API to perform memory management of MSL VM. It implements methods to force MSL garbage collector perform iterations, display logs and limit memory usage.
```cs 
namespace System 
{ 
    public static class GC
    {
        public static function Collect();
        public static function Disable();
        public static function Enable();
        public static function ReleaseMemory();
        public static function SetMinimalMemory(value);
        public static function SetMaximalMemory(value);
        public static function SetLogPermissions(value);
    }
}
 ```
### GC.SetLogPermissions(value)
*This method will be described first because it is used a lot in the code samples.*
Outputs garbage collection information to the `log` GC stream provided in VM config, including: iteration number, time of iteration, collected object count, managed object count, cleared memory and managed memory. `value` is a Boolean parameter which sets permission to output log to stream:
```cs
Console.PrintLine("Log enabled");
GC.SetLogPermissions(true);
GC.Collect(); // see documentation below
GC.SetLogPermissions(false);
Console.PrintLine("Log disabled");
GC.Collect(); // no output
```
### GC.Collect()
Forces MSL garbage collector to perform iteration even if it was disabled in config or by GC.Disable() method:
```cs
GC.SetLogPermissions(true);
Console.PrintLine("GC log enabled, forcing to collect...");
GC.Collect(); // Collect iteration
```
### GC.Disable()
Disables garbage collection in VM. If memory limit was hit, application will crush with `OutOfMemory` exception. This method throws `InvalidMethodCall` exception if `safeMode` was set to `true` in VM config:
```cs
GC.Disable();
Console.PrintLine("GC Disabled!");
while(true) Array(10000000); // OutOfMemory will be thrown soon
```
### GC.Enable()
Enabled garbage collection in VM. By default GC is enabled, but can be manually turned off in configuration file. This method throws `InvalidMethodCall` exception if `safeMode` was set to `true` in VM config:
```cs
GC.SetLogPermissions(true);
GC.Disable();
Console.PrintLine("GC disabled");
Console.PrintLine("Lets allocate objects...");
for (var i = 0; i < 100; i += 1)
    var array = Array(100000);
Console.PrintLine("Done, enabling GC...");
GC.Enable();
```
### GC.ReleaseMemory()
Forces garbage collector to release all free memory back to the OS. This method can be called after large temporary allocations but can force performance issues due to the future allocation requests to the OS:
```cs
GC.SetLogPermissions(true);
var array = Array(10000000);
Console.PrintLine("Lets free all memory!");
array = null;
GC.Collect();
GC.ReleaseMemory(); // need to use some kind of memory profiler to see differences
```
### GC.SetMinimalMemory(value)
Sets minimal memory for running application. Throws `InvalidMethodCall` exception if `safeMode` is set to `true` in VM config. Before hitting this limit no garbage collection will be performed. But setting it too high can cause application crush due to `OutOfMemory` error:
```cs
GC.SetLogPermissions(true);
GC.SetMinimalMemory(0); // set it to high number to see OutOfMemory error
Console.PrintLine("GC will collect garbage each cycle!");
```
### GC.SetMaximalMemory(value)
Sets maximal memory for running application. If program hits this limit `OutOfMemory` exception is thrown but setting it too high can cause application crush (nearly 2 GB on Windows OS). Throws `InvalidMethodCall` exception if `safeMode` is set to `true` in VM config. This method can be used to limit user actions when embedding MSL to another application:
```cs
GC.SetLogPermissions(true);
GC.SetMaximalMemory(2 * 1024 * 1024 * 1024); // OS limit
var bigArray = Array(128 * 1024 * 1024); // ~ 1 GB
bigArray = null;
Console.PrintLine("Now array will be collected by GC...");
for(var i = 0; i < 100000; i += 1) { } // wait until next collection
```