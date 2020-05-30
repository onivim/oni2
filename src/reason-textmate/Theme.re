/*
 Theme.re
 */

open Rench;

type t = {
  colors: ColorTheme.t,
  tokenColors: TokenTheme.t,
  isDark: bool,
};

type themeLoader = string => result(t, string);

let of_yojson = (~isDark=?, ~themeLoader, json: Yojson.Safe.t) => {
  let parse = json => {
    let colorsJson = Yojson.Safe.Util.member("colors", json);

    let colorTheme = ColorTheme.of_yojson(colorsJson);

    let isDark =
      switch (isDark) {
      | Some(v) => v
      | None =>
        switch (Yojson.Safe.Util.member("type", json)) {
        | `String("dark") => true
        | _ => false
        }
      };

    let defaultBackground = isDark ? "#1E1E1E" : "#FFFFFF";
    let defaultForeground = isDark ? "#D4D4D4" : "#000000";

    let defaultBackground =
      ColorTheme.getFirstOrDefault(
        ~default=defaultBackground,
        ["background", "editor.background"],
        colorTheme,
      );

    let defaultForeground =
      ColorTheme.getFirstOrDefault(
        ~default=defaultForeground,
        ["foreground", "editor.foreground"],
        colorTheme,
      );

    let tokenColorsJson = Yojson.Safe.Util.member("tokenColors", json);

    let tokenTheme =
      TokenTheme.of_yojson(
        ~defaultBackground,
        ~defaultForeground,
        tokenColorsJson,
      );

    // Is there an included theme? If so - we need to parse that
    let incl = Yojson.Safe.Util.member("include", json);

    let (colorTheme, tokenTheme) =
      switch (incl) {
      | `String(includePath) =>
        themeLoader(includePath)
        |> Result.map(parentTheme => {
             let mergedColorTheme =
               ColorTheme.union(parentTheme.colors, colorTheme);

             let defaultBackground =
               ColorTheme.getFirstOrDefault(
                 ~default="#000",
                 ["background", "editor.background"],
                 mergedColorTheme,
               );

             let defaultForeground =
               ColorTheme.getFirstOrDefault(
                 ~default="#FFF",
                 ["foreground", "editor.foreground"],
                 mergedColorTheme,
               );

             let mergedTokenTheme =
               TokenTheme.union(
                 ~defaultBackground,
                 ~defaultForeground,
                 parentTheme.tokenColors,
                 tokenTheme,
               );

             (mergedColorTheme, mergedTokenTheme);
           })
        |> Result.value(~default=(colorTheme, tokenTheme))
      // No 'include' - pass through as-is
      | _ => (colorTheme, tokenTheme)
      };

    {isDark, colors: colorTheme, tokenColors: tokenTheme};
  };

  parse(json);
};

module PlistDecoder = {
  open Plist;

  let repository = scopeName =>
    assoc(
      oneOf([
        ("pattern", Pattern.of_plist(scopeName) |> map(pat => [pat])),
        (
          "patterns",
          property("patterns", array(Pattern.of_plist(scopeName))),
        ),
      ]),
    );

  let theme = (~isDark=?, plist) => {
    let%bind maybeType = option(property("type", string), plist);
    let isDark = isDark |> Option.value(~default=maybeType == Some("dark"));

    let%bind settings = property("settings", array(id), plist);

    let%bind (first, rest) =
      switch (List.rev(settings)) {
      | [first, ...rest] => Ok((first, Array(rest)))
      | [] => Error("Expected an array of one or more settings, got none")
      };

    let%bind colors = property("settings", ColorTheme.of_plist, first);

    let defaultBackground =
      ColorTheme.getFirstOrDefault(
        ~default=isDark ? "#1E1E1E" : "#FFFFFF",
        ["background", "editor.background"],
        colors,
      );

    let defaultForeground =
      ColorTheme.getFirstOrDefault(
        ~default=isDark ? "#D4D4D4" : "#000000",
        ["foreground", "editor.foreground"],
        colors,
      );

    let%bind tokenColors =
      TokenTheme.of_plist(~defaultBackground, ~defaultForeground, rest);

    Ok({isDark, colors, tokenColors});
  };
};

let _themeCache: Hashtbl.t(string, result(t, string)) = Hashtbl.create(16);

let rec from_file = (~isDark=?, path: string) => {
  switch (Hashtbl.find_opt(_themeCache, path)) {
  | Some(v) => v
  | None =>
    let currentDirectory = Path.dirname(path);
    let themeLoader = p => {
      let fullPath = Path.join(currentDirectory, p);
      from_file(fullPath);
    };
    let theme =
      switch (Utility.JsonEx.from_file(path)) {
      | Ok(json) => Ok(of_yojson(~isDark?, ~themeLoader, json))
      | Error(_) =>
        let%bind plist =
          SimpleXml.of_file(path) |> Option.get |> XmlPlistParser.parse;

        PlistDecoder.theme(~isDark?, plist);
      };

    Hashtbl.add(_themeCache, path, theme);
    theme;
  };
};

let getColors = v => v.colors;
let getTokenColors = v => v.tokenColors;

let isDark = v => v.isDark;
