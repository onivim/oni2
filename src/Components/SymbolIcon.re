/*
 * SymbolIcon.re
 */

open Oni_Core;

module Colors = Feature_Theme.Colors;

module Internal = {
  let symbolToIcon =
    Exthost.SymbolKind.(
      fun
      | File => Codicon.symbolFile
      | Module => Codicon.symbolModule
      | Namespace => Codicon.symbolNamespace
      | Package => Codicon.symbolPackage
      | Class => Codicon.symbolClass
      | Method => Codicon.symbolMethod
      | Property => Codicon.symbolProperty
      | Field => Codicon.symbolField
      | Constructor => Codicon.symbolConstructor
      | Enum => Codicon.symbolEnum
      | Interface => Codicon.symbolInterface
      | Function => Codicon.symbolFunction
      | Variable => Codicon.symbolVariable
      | Constant => Codicon.symbolConstant
      | String => Codicon.symbolString
      | Number => Codicon.symbolNumber
      | Boolean => Codicon.symbolBoolean
      | Array => Codicon.symbolArray
      | Object => Codicon.symbolObject
      | Key => Codicon.symbolKey
      | Null => Codicon.symbolNull
      | EnumMember => Codicon.symbolEnumMember
      | Struct => Codicon.symbolStruct
      | Event => Codicon.symbolEvent
      | Operator => Codicon.symbolOperator
      | TypeParameter => Codicon.symbolTypeParameter
    );

  let symbolToColor =
    Exthost.SymbolKind.(
      fun
      | File => Colors.SymbolIcon.fileForeground
      | Module => Colors.SymbolIcon.moduleForeground
      | Namespace => Colors.SymbolIcon.namespaceForeground
      | Package => Colors.SymbolIcon.packageForeground
      | Class => Colors.SymbolIcon.classForeground
      | Method => Colors.SymbolIcon.methodForeground
      | Property => Colors.SymbolIcon.propertyForeground
      | Field => Colors.SymbolIcon.fieldForeground
      | Constructor => Colors.SymbolIcon.constructorForeground
      | Enum => Colors.SymbolIcon.enumeratorForeground
      | Interface => Colors.SymbolIcon.interfaceForeground
      | Function => Colors.SymbolIcon.functionForeground
      | Variable => Colors.SymbolIcon.variableForeground
      | Constant => Colors.SymbolIcon.constantForeground
      | String => Colors.SymbolIcon.stringForeground
      | Number => Colors.SymbolIcon.constantForeground
      | Boolean => Colors.SymbolIcon.booleanForeground
      | Array => Colors.SymbolIcon.arrayForeground
      | Object => Colors.SymbolIcon.objectForeground
      | Key => Colors.SymbolIcon.keyForeground
      | Null => Colors.SymbolIcon.nullForeground
      | EnumMember => Colors.SymbolIcon.enumeratorMemberForeground
      | Struct => Colors.SymbolIcon.structForeground
      | Event => Colors.SymbolIcon.eventForeground
      | Operator => Colors.SymbolIcon.operatorForeground
      | TypeParameter => Colors.SymbolIcon.typeParameterForeground
    );
};

let make = (~symbol: Exthost.SymbolKind.t, ~theme, ~fontSize=15., ()) => {
  let colorDefinition: Oni_Core.ColorTheme.Schema.definition =
    Internal.symbolToColor(symbol);
  <Codicon
    icon={Internal.symbolToIcon(symbol)}
    fontSize
    color={colorDefinition.from(theme)}
  />;
};
