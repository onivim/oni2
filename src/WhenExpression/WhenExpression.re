module Log = (val Oni_Core.Log.withNamespace("Oni2.WhenExpression"));

[@deriving show({with_path: false})]
type t =
  | Defined(string)
  | Eq(string, value)
  | Neq(string, value)
  | And(t, t)
  | Or(t, t)
  | Not(t)
  | Value(value)

and value =
  | String(string)
  | True
  | False;

let evaluate = (expr, getValue) => {
  let rec eval =
    fun
    | Defined(name) =>
      switch (getValue(name)) {
      | False => false
      | True
      | String(_) => true
      }
    | Eq(variable, value) => getValue(variable) == value
    | Neq(variable, value) => getValue(variable) != value
    | And(e1, e2) => eval(e1) && eval(e2)
    | Or(e1, e2) => eval(e1) || eval(e2)
    | Not(e) => !eval(e)
    | Value(True) => true
    | Value(False) => false
    | Value(String("")) => false
    | Value(String(_)) => true;

  let result = eval(expr);
  Log.tracef(m => m("Expression %s evaluated to: %b", show(expr), result));
  result;
};

module Parse = {
  let deserializeValue = {
    let quoted = Re.Posix.re("^'([^']*)'$") |> Re.compile;

    str =>
      switch (String.trim(str)) {
      | "true" => True
      | "false" => False
      | str =>
        switch (Re.Group.get(Re.exec(quoted, str), 1)) {
        | unquoted => String(unquoted)
        | exception Not_found => String(str)
        }
      };
  };

  let deserializeOne = {
    let eq = Re.str("==") |> Re.compile;
    let neq = Re.str("!=") |> Re.compile;
    let not = Re.Pcre.re("^\\!\\s*") |> Re.compile;

    str => {
      let str = String.trim(str);

      if (Re.execp(neq, str)) {
        switch (Re.split(neq, str)) {
        | [left, right, ..._] =>
          // This matches more than two "pieces", which should be a synatx error.
          // But since vscode accepts it, so do we.
          Neq(left, deserializeValue(right))
        | _ => failwith("unreachable")
        };
      } else if (Re.execp(eq, str)) {
        switch (Re.split(eq, str)) {
        | [left, right, ..._] =>
          // This matches more than two "pieces", which should be a synatx error.
          // But since vscode accepts it, so do we.
          Eq(left, deserializeValue(right))
        | _ => failwith("unreachable")
        // TODO: =~ (regex)
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

    str =>
      str
      |> Re.split(re)
      |> List.fold_left(
           (acc, piece) => And(acc, deserializeOne(piece)),
           Value(True),
         );
  };

  let deserializeOr = {
    let re = Re.str("||") |> Re.compile;

    str =>
      str
      |> Re.split(re)
      |> List.fold_left(
           (acc, piece) => Or(acc, deserializeAnd(piece)),
           Value(False),
         );
  };
};

let parse = str => Ok(Parse.deserializeOr(str));
