[@deriving show({with_path: false})]
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

exception Invalid_Symbol;

let of_int =
  fun
  | 0 => File
  | 1 => Module
  | 2 => Namespace
  | 3 => Package
  | 4 => Class
  | 5 => Method
  | 6 => Property
  | 7 => Field
  | 8 => Constructor
  | 9 => Enum
  | 10 => Interface
  | 11 => Function
  | 12 => Variable
  | 13 => Constant
  | 14 => String
  | 15 => Number
  | 16 => Boolean
  | 17 => Array
  | 18 => Object
  | 19 => Key
  | 20 => Null
  | 21 => EnumMember
  | 22 => Struct
  | 23 => Event
  | 24 => Operator
  | 25 => TypeParameter
  | _ => raise(Invalid_Symbol);

let of_int_opt = v =>
  try(Some(of_int(v))) {
  | Invalid_Symbol => None
  };

let of_yojson: Yojson.Safe.json => option(t) =
  fun
  | `Int(x) => of_int_opt(x)
  | `Float(x) => of_int_opt(int_of_float(x))
  | _ => None;
