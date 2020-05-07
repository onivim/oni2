module ContextKeys = ContextKeys;

module Value = ContextKeys.Value;

[@deriving show({with_path: false})]
type t =
  | Defined(string)
  | Eq(string, Value.t)
  | Neq(string, Value.t)
  | Regex(string, option(Re.re))
  | And(list(t))
  | Or(list(t))
  | Not(t)
  | Value(Value.t);

let evaluate = (expr, getValue) => {
  let rec eval =
    fun
    | Defined(name) => getValue(name) |> Value.asBool
    | Eq(key, value) => getValue(key) == value
    | Neq(key, value) => getValue(key) != value
    | Regex(_, None) => false
    | Regex(key, Some(re)) => Re.execp(re, key |> getValue |> Value.asString)
    | And(exprs) => List.for_all(eval, exprs)
    | Or(exprs) => List.exists(eval, exprs)
    | Not(expr) => !eval(expr)
    | Value(value) => Value.asBool(value);

  eval(expr);
};

module Parse = {
  // Translated relatively faithfully from
  // https://github.com/microsoft/vscode/blob/e683dce828edccc6053bebab48a1954fb61f8e29/src/vs/platform/contextkey/common/contextkey.ts#L59

  let deserializeValue = {
    let quoted = Re.Posix.re("^'([^']*)'$") |> Re.compile;

    str =>
      switch (String.trim(str)) {
      | "true" => Value.True
      | "false" => Value.False
      | str =>
        switch (Re.Group.get(Re.exec(quoted, str), 1)) {
        | unquoted => Value.String(unquoted)
        | exception Not_found => Value.String(str)
        }
      };
  };

  let deserializeRegexValue = (~strict=false, str) =>
    switch (String.trim(str)) {
    | "" when strict => failwith("missing regexp-value for =~-expression")
    | "" => None
    | str =>
      switch (String.index_opt(str, '/'), String.rindex_opt(str, '/')) {
      | (Some(start), Some(stop)) when start == stop =>
        failwith("bad regexp-value '" ++ str ++ "', missing /-enclosure")

      | (Some(start), Some(stop)) =>
        let pattern = String.sub(str, start + 1, stop - start - 1);
        switch (Re.Pcre.re(pattern) |> Re.compile) {
        | regex => Some(regex)
        | exception Re.Perl.Parse_error when strict =>
          failwith("invalid regexp '" ++ str ++ "', ")
        | exception Re.Perl.Parse_error => None
        };

      | (None, None) when strict =>
        failwith("bad regexp-value '" ++ str ++ "', missing /-enclosure")

      | (None, None) => None

      | _ => failwith("unreachable")
      }
    };

  let deserializeOne = {
    let eq = Re.str("==") |> Re.compile;
    let neq = Re.str("!=") |> Re.compile;
    let regex = Re.str("=~") |> Re.compile;
    let not = Re.Pcre.re("^\\!\\s*") |> Re.compile;

    str => {
      let str = String.trim(str);

      if (Re.execp(neq, str)) {
        switch (Re.split(neq, str)) {
        // This matches more than two "pieces", which should be a synatx error.
        // But since vscode accepts it, so do we.
        | [left, right, ..._] =>
          let key = String.trim(left);
          switch (deserializeValue(right)) {
          | True => Not(Defined(key))
          | False => Defined(key)
          | value => Neq(key, value)
          };
        | _ => failwith("unreachable")
        };
      } else if (Re.execp(eq, str)) {
        switch (Re.split(eq, str)) {
        // This matches more than two "pieces", which should be a synatx error.
        // But since vscode accepts it, so do we.
        | [left, right, ..._] =>
          let key = String.trim(left);
          switch (deserializeValue(right)) {
          | True => Defined(key)
          | False => Not(Defined(key))
          | value => Eq(key, value)
          };
        | _ => failwith("unreachable")
        };
      } else if (Re.execp(regex, str)) {
        switch (Re.split(regex, str)) {
        // This matches more than two "pieces", which should be a syntax error.
        // But since vscode accepts it, so do we.
        | [left, right, ..._] =>
          let key = String.trim(left);
          let maybeRegex = deserializeRegexValue(right);
          Regex(key, maybeRegex);
        | _ => failwith("unreachable")
        };
      } else if (Re.execp(not, str)) {
        Not(Defined(Re.Str.string_after(str, 1) |> String.trim));
      } else {
        Defined(str);
      };
    };
  };

  let deserializeAnd = {
    let re = Re.str("&&") |> Re.compile;

    str => And(str |> Re.split(re) |> List.map(deserializeOne));
  };

  let deserializeOr = {
    let re = Re.str("||") |> Re.compile;

    str => Or(str |> Re.split(re) |> List.map(deserializeAnd));
  };
};

let parse = Parse.deserializeOr;
