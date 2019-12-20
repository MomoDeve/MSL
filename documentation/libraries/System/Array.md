## System.Array
Array class provide necessary API to allocate memory for large amount of data and manipulate elements of collections
```cs
namespace System
{
    public class Array
    {
        public function Array();
        public function Array(size);
        public function Size();
        public function Empty();
        public function Append(object);
        public function Pop();
        public function Front();
        public function Back();
        public function GetByIndex(index);
        public function GetByIter(iterator);
        public function Next(iterator);
        public function Begin();
        public function End();
        public function Cloned();
        public function Merged(array);
        public function Reversed();
        public function Sort();
    }
}
```
*Documentation for this class will be updated soon. For now, source code of System library and dll library is available in MSL repositoty. If you have any questions, you can contact me personally.*