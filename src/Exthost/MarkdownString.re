// TODO: Implement full type here
// https://github.com/onivim/vscode-exthost/blob/c7df89c1cf0087ca5decaf8f6d4c0fd0257a8b7a/src/vs/base/common/htmlContent.ts#L10
//type t = {
//	stringValue: string, // value
//	isTrusted: bool,
//	supportThemeIcons: bool,
// uris
//}
open Oni_Core;

type t = string;

let decode =
  Json.Decode.(obj(({field, _}) => {field.required("value", string)}));
