## System.Console
Console library class provide API to output and input data using standard console. It uses `in` and `out` streams which can be redefined in VM config, redirecting output to files and etc.
```cs 
namespace System 
{ 
    public static class Console
    {
        public static function Print(object);
        public static function PrintLine(object);
        public static function Read();
        public static function ReadLine();
        public static function ReadInt();
        public static function ReadFloat();
        public static function ReadBool();
    }
}
 ```
### Console.Print(object)
Prints object to the output stream. If object is class or namespace, outputs its assembly name. If object is primitive type, its value is outputted and if object is class instance and ToString() method is defined Console.Print invokes it and uses its return value as new argument:
```cs
Console.Print(-3); // `-3`
var arr = Array(2);
arr[0] = "hello"; arr[1] = "world";
Console.Print("\n"); // new line character
Console.Print(arr[0]); // `hello`
Console.Print(arr.ToString()); // `["hello", "world"]`
Console.Print(arr); // same as above as ToString() method is called implicitly
```
### Console.PrintLine(object)
This method does same as `Console.Print`, but also flushes output adding newline symbol:
```cs
var a; // creating local object. Its value is null by default
Console.PrintLine(a); // `null`
a = 2.5;
Console.PrintLine(a); // `2.5`
Console.PrintLine(a ** 2); // `6.25`
```
### Console.Read()
Reads input from input stream until meets space character or reaches end of file. Space characters are ignored, returns string as result:
```cs
// user enters `hello world`
var s1 = Console.Read();
var s2 = Console.Read(); 
Console.PrintLine(s1); // `hello`
Console.PrintLine(s2); // `world`
```
### Console.ReadLine()
Reads line from input stream until meets newline character of reached end of file:
```cs
// user enters `hello world`
var echo = Console.ReadLine();
Console.PrintLine(echo); // `hello world`
```
### Console.ReadInt()
Reads Integer from input stream until meets space character or reaches end of file and tries to parse it to `System.Integer` object. 
If input was not integer, returns 0 by default. If this method meets unrecognised symbol during parsing, it trims everything after it:
```cs
// input: `42`
var int1 = Console.ReadInt(); // int1 = 42
// input: `-123.973`
var int2 = Console.ReadInt(); // int2 = -123
// input: `inf`
var int3 = Console.ReadInt(); // int3 = inf (`inf` is defined by MSL as integer)
// input: `150sde3444`
var int4 = Console.ReadInt(); // int4 = 150
// input: `aaa1ssl-2`
var int5 = Console.ReadInt(); // int5 = 0
```
### Console.ReadFloat()
Reads Float from input stream until meets space character or reaches end of file and tries to parse it to `System.Float` object.
If input was not float, returns 0.0 by default. If this method meets unrecognised symbol during parsing, it trims everything after it:
```cs
// input: `42`
var f1 = Console.ReadFloat(); // f1 = 42.0
// input: `-123.973`
var f2 = Console.ReadFloat(); // f2 = -123.973
// input: `-1e3`
var f3 = Console.ReadFloat(); // f3 = -1000
// input: `150.3f33e`
var f4 = Console.ReadFloat(); // f4 = 150.3
// input: `aaa1ssl-2`
var f5 = Console.ReadFloat(); // f5 = 0.0
```
### Console.ReadBool()
Reads Boolean from input stream until meets space character or reaches end of file and tries to parse it to `System.True` or `System.False` object.
If input was `1`, `True` or `true`, result will be `System.True` object. All other cases results in `System.False` to avoid accidental user input:
```cs
// input: `true`
var b1 = Console.ReadBool(); // b1 = true
// input: `False`
var b2 = Console.ReadBool(); // b2 = false
// input: `-2`
var b3 = Console.ReadBool(); // b3 = false
// input: `1`
var b4 = Console.ReadBool(); // b4 = true
```
