type t =
  | Method
  | Function
  | Constructor
  | Field
  | Variable
  | Class
  | Struct
  | Interface
  | Module
  | Property
  | Event
  | Operator
  | Unit
  | Value
  | Constant
  | Enum
  | EnumMember
  | Keyword
  | Text
  | Color
  | File
  | Reference
  | Customcolor
  | Folder
  | TypeParameter
  | User
  | Issue
  | Snippet;

let ofInt =
  fun
  | 0 => Some(Method)
  | 1 => Some(Function)
  | 2 => Some(Constructor)
  | 3 => Some(Field)
  | 4 => Some(Variable)
  | 5 => Some(Class)
  | 6 => Some(Struct)
  | 7 => Some(Interface)
  | 8 => Some(Module)
  | 9 => Some(Property)
  | 10 => Some(Event)
  | 11 => Some(Operator)
  | 12 => Some(Unit)
  | 13 => Some(Value)
  | 14 => Some(Constant)
  | 15 => Some(Enum)
  | 16 => Some(EnumMember)
  | 17 => Some(Keyword)
  | 18 => Some(Text)
  | 19 => Some(Color)
  | 20 => Some(File)
  | 21 => Some(Reference)
  | 22 => Some(Customcolor)
  | 23 => Some(Folder)
  | 24 => Some(TypeParameter)
  | 25 => Some(User)
  | 26 => Some(Issue)
  | 27 => Some(Snippet)
  | _ => None;
