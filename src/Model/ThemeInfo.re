/*
 * ThemeInfo.re
 */

open Oni_Core;
open Oni_Extensions;

open Exthost.Extension;

type t = {
  themes: list(Contributions.Theme.t),
  nameToTheme: StringMap.t(Contributions.Theme.t),
};

let _getThemes = (extensions: list(Scanner.ScanResult.t)) => {
  open Scanner.ScanResult;
  extensions |> List.map(v => v.manifest.contributes.themes) |> List.flatten;
};

let create = () => {themes: [], nameToTheme: StringMap.empty};

let getThemes = v => v.themes;

let getThemeByName = (v, name) => StringMap.find_opt(name, v.nameToTheme);

let ofExtensions = (extensions: list(Scanner.ScanResult.t)) => {
  let themes = _getThemes(extensions);

  let nameToTheme =
    List.fold_left(
      (prev, curr: Contributions.Theme.t) => {
        StringMap.add(curr.label, curr, prev)
      },
      StringMap.empty,
      themes,
    );

  let ret: t = {themes, nameToTheme};
  ret;
};

let show = (v: t) => {
  let themeStr =
    List.map(
      (t: Contributions.Theme.t) => " - " ++ t.label,
      v.themes,
    )
    |> String.concat("\n");

  "Themes: \n" ++ themeStr;
};
