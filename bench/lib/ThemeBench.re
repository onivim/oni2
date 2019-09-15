open Oni_Syntax;
open BenchFramework;

let oneDarkThemePath = "extensions/onedark-pro/themes/OneDark-Pro.json";

let themeJson = Yojson.Safe.from_file(oneDarkThemePath);
let tokenColorsJson = Yojson.Safe.Util.member("tokenColors", themeJson);
let tmTheme = TextMateTheme.of_yojson(
  ~defaultBackground=Revery.Colors.black,
  ~defaultForeground=Revery.Colors.white,
  tokenColorsJson
);

let themeWithCaching = Theme.create(tmTheme);
let themeWithoutCaching = Theme.create(~useCache=false, tmTheme);

let simpleScopeTest = (themeToUse, ()) => {
  let _ = Theme.match(themeToUse, "source.reason");
  ();
};

let largerScopeTest = (themeToUse, ()) => {
  let _ = Theme.match(themeToUse, "entity.name.filename support.property-value constant.language markup.inserted source.reason");
  ();
};

let setup = () => ();

bench(
  ~name="Theme: small scope (cached)",
  ~setup,
  ~f=simpleScopeTest(themeWithCaching),
  (),
);

bench(
  ~name="Theme: multiple scopes (cached)",
  ~setup,
  ~f=largerScopeTest(themeWithCaching),
  (),
);

bench(
  ~name="Theme: small scope (uncached)",
  ~setup,
  ~f=simpleScopeTest(themeWithoutCaching),
  (),
);

bench(
  ~name="Theme: multiple scopes (uncached)",
  ~setup,
  ~f=largerScopeTest(themeWithoutCaching),
  (),
);
