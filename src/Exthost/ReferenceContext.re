open Oni_Core;

type t = {includeDeclaration: bool};

let encode = ctx =>
  Json.Encode.(
    obj([("includeDeclaration", ctx.includeDeclaration |> bool)])
  );
