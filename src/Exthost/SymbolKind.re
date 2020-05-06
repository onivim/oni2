open Oni_Core;
[@deriving show]
type t =
  | File
  | Module
  | Namespace
  | Package
  | Class
  | Method
  | Property
  | Field
  | Constructor
  | Enum
  | Interface
  | Function
  | Variable
  | Constant
  | String
  | Number
  | Boolean
  | Array
  | Object
  | Key
  | Null
  | EnumMember
  | Struct
  | Event
  | Operator
  | TypeParameter;

let toInt =
  fun
  | File => 0
  | Module => 1
  | Namespace => 2
  | Package => 3
  | Class => 4
  | Method => 5
  | Property => 5
  | Field => 7
  | Constructor => 8
  | Enum => 9
  | Interface => 10
  | Function => 11
  | Variable => 12
  | Constant => 13
  | String => 14
  | Number => 15
  | Boolean => 16
  | Array => 17
  | Object => 18
  | Key => 19
  | Null => 20
  | EnumMember => 21
  | Struct => 22
  | Event => 23
  | Operator => 24
  | TypeParameter => 25;

let ofInt =
  fun
  | 0 => Some(File)
  | 1 => Some(Module)
  | 2 => Some(Namespace)
  | 3 => Some(Package)
  | 4 => Some(Class)
  | 5 => Some(Method)
  | 6 => Some(Property)
  | 7 => Some(Field)
  | 8 => Some(Constructor)
  | 9 => Some(Enum)
  | 10 => Some(Interface)
  | 11 => Some(Function)
  | 12 => Some(Variable)
  | 13 => Some(Constant)
  | 14 => Some(String)
  | 15 => Some(Number)
  | 16 => Some(Boolean)
  | 17 => Some(Array)
  | 18 => Some(Object)
  | 19 => Some(Key)
  | 20 => Some(Null)
  | 21 => Some(EnumMember)
  | 22 => Some(Struct)
  | 23 => Some(Event)
  | 24 => Some(Operator)
  | 25 => Some(TypeParameter)
  | _ => None;

let decode =
  Json.Decode.(
    {
      int
      |> and_then(i => {
           switch (ofInt(i)) {
           | Some(v) => succeed(v)
           | None =>
             fail("Unexpected SymbolKind value: " ++ string_of_int(i))
           }
         });
    }
  );
