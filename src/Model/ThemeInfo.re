/*
 * ThemeInfo.re
 */

open Oni_Core;
open Oni_Core_Kernel;
open Oni_Extensions;

open Oni_Extensions.ExtensionScanner;

type t = {
  themes: list(ExtensionContributions.Theme.t),
  nameToTheme: StringMap.t(ExtensionContributions.Theme.t),
};

let _getThemes = (extensions: list(ExtensionScanner.t)) => {
  extensions |> List.map(v => v.manifest.contributes.themes) |> List.flatten;
};

let create = () => {themes: [], nameToTheme: StringMap.empty};

let getThemes = v => v.themes;

let getThemeByName = (v, name) => StringMap.find_opt(name, v.nameToTheme);

let ofExtensions = (extensions: list(ExtensionScanner.t)) => {
  let themes = _getThemes(extensions);

  let nameToTheme =
    List.fold_left(
      (prev, curr: ExtensionContributions.Theme.t) => {
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
      (t: ExtensionContributions.Theme.t) => " - " ++ t.label,
      v.themes,
    )
    |> String.concat("\n");

  "Themes: \n" ++ themeStr;
};
