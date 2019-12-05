/*
 * CompletionItemKind.re
 */

[@deriving show]
type t =
  | Text
  | Method
  | Function
  | Constructor
  | Field
  | Variable
  | Class
  | Interface
  | Module
  | Property
  | Unit
  | Value
  | Enum
  | Keyword
  | Snippet
  | Color
  | File
  | Reference
  | Folder
  | EnumMember
  | Constant
  | Struct
  | Event
  | Operator
  | TypeParameter;

// This needs to be kept in sync with the definition here:
// https://github.com/onivim/vscode-exthost/blob/a25f426a04fe427beab7465be660f89a794605b5/src/vs/workbench/api/node/extHostTypes.ts#L1230

let ofInt = i => {
  switch (i) {
  | 0 => Some(Text)
  | 1 => Some(Method)
  | 2 => Some(Function)
  | 3 => Some(Constructor)
  | 4 => Some(Field)
  | 5 => Some(Variable)
  | 6 => Some(Class)
  | 7 => Some(Interface)
  | 8 => Some(Module)
  | 9 => Some(Property)
  | 10 => Some(Unit)
  | 11 => Some(Value)
  | 12 => Some(Enum)
  | 13 => Some(Keyword)
  | 14 => Some(Snippet)
  | 15 => Some(Color)
  | 16 => Some(File)
  | 17 => Some(Reference)
  | 18 => Some(Folder)
  | 19 => Some(EnumMember)
  | 20 => Some(Constant)
  | 21 => Some(Struct)
  | 22 => Some(Event)
  | 23 => Some(Operator)
  | 24 => Some(TypeParameter)
  | _ => None
  };
};
