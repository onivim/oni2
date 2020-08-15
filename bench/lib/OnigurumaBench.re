open Oniguruma;
open BenchFramework;

let setup = () => OnigRegExp.create("abc") |> Result.get_ok;

let simpleRegex = regex => {
  ignore(OnigRegExp.search("abc", 0, regex): array(OnigRegExp.Match.t));
};

let simpleRegexFastSearch = regex => {
  ignore(OnigRegExp.Fast.search("abc", 0, regex): int);
};

bench(~name="Oniguruma: search", ~setup, ~f=simpleRegex, ());

bench(~name="Oniguruma: search (fast)", ~setup, ~f=simpleRegexFastSearch, ());
