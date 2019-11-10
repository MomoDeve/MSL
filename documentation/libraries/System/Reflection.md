# System.Reflection
Reflection library class provide useful API for inspecting MSL types and determine methods to be invoked in runtime:
```cs
namespace System
{
    public static class Reflection
    {
        public static function GetType(object);
        public static function CreateInstance(type);
        public static function CreateInstance(type, args);
        public static function Invoke(object, method);
        public static function Invoke(object, method, args);
    }
}
```
## Reflection.GetType(object)
Returns type of object provided. If object is primitive type, returns System static wrapper of that class. If object is already a type, returns object:
```cs
// primitive types
var intType = Reflection.GetType(0); // intType = System.Integer
var nullType = Reflection.GetType(null); // nullType = System.Null

// get type of class instance
var v = Math.Vector2(3, 4);
var vecType = Reflection.GetType(v); // vecType = Math.Vector2

// returns same object when argument is class or namespace
var ns = Reflection.GetType(System); // ns = System
var arr = ns.Array(10); // arr = System.Array(10);
```
## Reflection.CreateInstance(type)
Creates object of class provided as argument. If class does not have constructor with no arguments, call results in error:
```cs
// create Math.Vector2 instance using CreateInstance method
var obj = Reflection.CreateInstance(Math.Vector2);
obj.x = 3;
obj.y = 4;
Console.Print(obj); // prints [3; 4] calling Math.Vector2.ToString()
```
## Reflection.CreateInstance(type, args)
Creates object of class provided as first argument, passing *args* parameter to constructor. If type of *args* is **System.Array**, it is unpacked and constructor recieves arguments stored in that array:
```cs
// one argument example
var obj = Reflection.CreateInstance(System.Array, 3);
Console.PrintLine(obj); // prints [null, null, null] calling System.Array.ToString()

// multiple arguments example
var args = System.Array(3); // create array with 3 arguments
args[0] = -1;
args[1] = 3;
args[2] = 0;
var vec = Reflection.CreateInstance(Math.Vector3, args);
Console.PrintLine(vec); // prints [-1, 3, 0] calling Math.Vector3.ToString()
```
*note:* if constructor accepts System.Array as its only argument so that *args* should not be unpacked, array must be wrapper into another array with size 1:
```cs
var arg = System.Array(); // arg to SomeClass
var args = System.Array(1);
args[0] = arg;
var obj = Reflection.CreateInstance(SomeClass, args); // args are unpacked and SomeClass recieves arg
```
## Reflection.Invoke(object, method)
Invokes *object*'s method with no parameters. If object is a type reference, only static methods can be invoked. If object is class instance, all methods can be invoked.

*note:* **Reflection.Invoke** method cannot call static constructor, as it can be called only once during program runtime. To force static constructor to be invoked, any other static method can be called
```cs
// call of object method
var v1 = Math.Vector2(3, 4);
var str = Reflection.Invoke(v1, "ToString"); // str = "[3, 4]"

// call of static method
Reflection.Invoke(System.GC, "Collect"); // invokes GC.Collect() method
```
## Reflection.Invoke(object, method, args)
Invokes *object*'s method with *args* as parameter. If object is a type reference, only static methods can be invoked. If object is class instance, all methods can be invoked. If type of *args* is **System.Array**, it is being unpacked (see documentation for **Reflection.CreateInstance(type, args)** method):
```cs
// call of object method
var v1 = Math.Vector2(3, 4);
var v2 = Math.Vector2(1, -2);
var v3 = Reflection.Invoke(v1, "Dot", v2); // same as v3 = v1.Dot(v2);

// call of static method with unpacking args
var args = System.Array(1);
args[0] = "Hello World!";
Reflection.Invoke(System.Console, "PrintLine", args); // prints `Hello World!`
```
## Other methods
Right now, only these 5 methods are supported in MSL. This paper is updated synchroniously with MSL compiler, so any new methods in the Reflection class will be documented here. Possible methods, which can be built on top of the Reflection class, are also declared in **Utils.Type** class (see **Utils** library documentation).
