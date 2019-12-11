## System.Reflection
Reflection library class provide useful API for inspecting MSL types and determine methods to be invoked in runtime:
```cs
namespace System
{
    public static class Reflection
    {
        public static function GetType(object);
        public static function CreateInstance(type, args); 
        public static function Invoke(object, method, args);
        public static function ContainsMethod(object, method, argCount);
        public static function GetNamespace(name);
        public static function GetMember(parent, child);
        public static function IsNamespaceExists(name);
        public static function ContainsMember(object, member);
    }
}
```
### Reflection.GetType(object)
Returns type of object provided. If object is primitive type, returns System static wrapper of that class. If object is already a type, returns object:
```cs
// primitive types
var intType = Reflection.GetType(0); // intType = System.Integer
var nullType = Reflection.GetType(null); // nullType = System.Null

// get type of class instance
var v = MathUtils.Vector2(3, 4);
var vecType = Reflection.GetType(v); // vecType = MathUtils.Vector2

// returns same object when argument is class or namespace
var ns = Reflection.GetType(System); // ns = System
var arr = ns.Array(10); // arr = System.Array(10);
```
### Reflection.CreateInstance(type, args)
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
var vec = Reflection.CreateInstance(MathUtils.Vector3, args);
Console.PrintLine(vec); // prints [-1, 3, 0] calling MathUtils.Vector3.ToString()
```
*note:* if constructor accepts System.Array as its only argument so that *args* should not be unpacked, array must be wrapper into another array with size 1:
```cs
var arg = System.Array(); // arg to SomeClass
var args = System.Array(1);
args[0] = arg;
var obj = Reflection.CreateInstance(SomeClass, args); // args are unpacked and SomeClass recieves arg
```
### Reflection.Invoke(object, method, args)
Invokes *object*'s method with *args* as parameter. If object is a type reference, only static methods can be invoked. If object is class instance, all methods can be invoked. If type of *args* is **System.Array**, it is being unpacked and constructor recieves arguments stored in that array.

*note:* **Reflection.Invoke** method cannot call static constructor, as it can be called only once during program runtime. To force static constructor to be invoked, any other static method can be called
```cs
// call of object method
var v1 = MathUtils.Vector2(3, 4);
var v2 = MathUtils.Vector2(1, -2);
var v3 = Reflection.Invoke(v1, "Dot", v2); // same as v3 = v1.Dot(v2);

// call of static method with unpacking args
var args = System.Array(1);
args[0] = "Hello World!";
Reflection.Invoke(System.Console, "PrintLine", args); // prints `Hello World!`
```
### Reflection.ContainsMethod(object, method, argCount)
Returns **true** if *object* contains public method with *argCount* arguments and name provided as second parameter, returns **false** either. Guaranteed that if this method returns **true**, object method can be called with *argCount* arguments.
```cs
var array = System.Array(1);
var str = "";
if (Reflection.ContainsMethod(array, "ToString", 0))
    str = array.ToString();
else
    str = "no ToString() method in this class";
Console.PrintLine(str);
```
### Reflection.GetNamespace(name)
returns namespace reference with name provided as an argument. If namespace does not exists in current runtime, this method result in error:
```cs
var ns = Reflection.GetNamespace("MathUtils");
var vec = ns.Vector2(-1, 1);
Console.PrintLine(vec); // prints `[-1; 1]`
```
### Reflection.GetMember(parent, child)
returns member of namespace, static attribute of class, or attribute of class object provided as *parent* with name provided to method as *child*. If *parent* is not one of the above objects or it does not contain attribute / class, this method result in error:
```cs
var vector = Reflection.GetMember(MathUtils, "Vector2"); // same as MathUtils.Vector2
var v = Reflection.CreateInstance(vector, System.Array()); // construct object with 0 arguments
Reflection.GetMember(v, "x") = 3; // same as vec.x = 3
Console.PrintLine(v); // prints `[3; 0]`
```
### Reflection.IsNamespaceExists(name)
Returns **true** if assembly contains namespace with name provided as argument, returns **false** either:
```cs
if (Reflection.IsNamespaceExists("MathUtils"))
    Console.PrintLine(MathUtils); // prints `namespace MathUtils`
else
    Console.PrintLine("no namespace was found with name 'MathUtils'");
```
### Reflection.ContainsMember(object, member)
Returns **true** if *object* is namespace and contains public class, if *object* is class an contains public static attribute, or if *object* is class object and contains public attribute with name provided as *member*, returns **false** either. Guaranteed that if this method returns **true** object member can be accessed with with `object.member`:
```cs
var v1 = MathUtils.Vector2();
var v2 = MathUtils.Vector3();
if (Reflection.ContainsMember(v1, "z")) // false as Vector2 has only `x` and `y` attributes
    v1.z = 3;
if (Reflection.ContainsMember(v2, "z")) // true as Vector3 has attribute `z`
    v2.z = 3;
Console.PrintLine(v1); // prints `[0; 0]`
Console.PrintLine(v2); // prints `[0; 0; 3]`
```
