module StringEx = Utility.StringEx;

// Type defining a simplified 'syntax scope' of a token -
// is it a comment, string, both, or neither?
// This is important for features like auto-closing pairs -
// we may not care to close certain characters in contexts like strings or comments.
type t = {
  isComment: bool,
  isString: bool,
};

let none = {isComment: false, isString: false};

let ofScopes = (scopes: list(string)) => {
  let anyStrings =
    scopes
    |> List.exists(scope => StringEx.startsWith(~prefix="string", scope));

  let anyComments =
    scopes
    |> List.exists(scope => StringEx.startsWith(~prefix="comment", scope));

  let isString = anyStrings;
  let isComment = anyComments;
  {isString, isComment};
};

let ofScope = scope => {
  let isString = StringEx.startsWith(~prefix="string", scope);
  let isComment = StringEx.startsWith(~prefix="comment", scope);
  {isString, isComment};
};

let toString = scope => {
  Printf.sprintf(
    "SyntaxScope: isComment: %s isString: %s",
    string_of_bool(scope.isComment),
    string_of_bool(scope.isString),
  );
};
