open Utility;

let hasLeadingComment = (~lineComment, line) =>
  if (StringEx.isEmpty(line)) {
    false;
  } else {
    let trimmed = StringEx.trimLeft(line);
    StringEx.startsWith(~prefix=lineComment, trimmed);
  };

let%test_module "hasLeadingComment" =
  (module
   {
     let%test "empty string" = {
       hasLeadingComment(~lineComment="#", "") == false;
     };
     let%test "whitespace" = {
       hasLeadingComment(~lineComment="#", "  ") == false;
     };

     let%test "just comment" = {
       hasLeadingComment(~lineComment="#", "#") == true;
     };

     let%test "multi-character comment" = {
       hasLeadingComment(~lineComment="##", "#") == false;
     };

     let%test "multi-character comment" = {
       hasLeadingComment(~lineComment="##", "##") == true;
     };
   });

let shouldAddComment = (~lineComment, lines) => {
  Array.exists(
    line =>
      !StringEx.isEmpty(line) && !hasLeadingComment(~lineComment, line),
    lines,
  );
};

let addComment = (~idxToAppend, ~lineComment, line) =>
  if (hasLeadingComment(~lineComment, line) || StringEx.isEmpty(line)) {
    line;
  } else {
    let (leadingWhitespace, after) =
      if (idxToAppend > 0) {
        let whitespace = String.sub(line, 0, idxToAppend);
        let after =
          String.sub(line, idxToAppend, String.length(line) - idxToAppend);
        (whitespace, after);
      } else {
        ("", line);
      };

    leadingWhitespace ++ lineComment ++ after;
  };

let removeComment = (~lineComment, line) =>
  if (hasLeadingComment(~lineComment, line)) {
    let idxToRemove =
      StringEx.findNonWhitespace(line) |> Option.value(~default=0);

    let leadingWhitespace =
      if (idxToRemove > 0) {
        String.sub(line, 0, idxToRemove);
      } else {
        "";
      };

    let len = String.length(line);
    let commentLen = String.length(lineComment);
    let afterComment =
      String.sub(
        line,
        idxToRemove + commentLen,
        len - commentLen - idxToRemove,
      );
    leadingWhitespace ++ afterComment;
  } else {
    line;
  };

let%test_module "removeComment" =
  (module
   {
     let%test "empty string" = {
       removeComment(~lineComment="#", "") == "";
     };
     let%test "whitespace" = {
       removeComment(~lineComment="#", "  ") == "  ";
     };

     let%test "just comment" = {
       removeComment(~lineComment="#", "#") == "";
     };

     let%test "just comment, leading whitespace" = {
       removeComment(~lineComment="#", "   #") == "   ";
     };

     let%test "comment with text" = {
       removeComment(~lineComment="#", "#abc") == "abc";
     };

     let%test "comment with text, leading whitespace" = {
       removeComment(~lineComment="#", "   #abc") == "   abc";
     };
   });

let toggle = (~lineComment, lines) =>
  if (shouldAddComment(~lineComment, lines)) {
    // Find the minimum index to add comments
    let maybeIdx =
      Array.fold_left(
        (acc, cur) => {
          let currIdx = StringEx.findNonWhitespace(cur);
          switch (currIdx, acc) {
          | (Some(v), None) => Some(v)
          | (Some(v), Some(prev)) when v < prev => Some(v)
          | (_, Some(_) as prev) => prev
          | (None, None) => None
          };
        },
        None,
        lines,
      );
    let idxToAppend = maybeIdx |> Option.value(~default=0);
    Array.map(
      addComment(~idxToAppend, ~lineComment=lineComment ++ " "),
      lines,
    );
  } else {
    Array.map(removeComment(~lineComment=lineComment ++ " "), lines);
  };

let%test_module "toggle" =
  (module
   {
     let%test "comments -> no comments" = {
       toggle(~lineComment="#", [|"# a", "# b"|]) == [|"a", "b"|];
     };
     let%test "no comments -> comments" = {
       toggle(~lineComment="#", [|"a", "b"|]) == [|"# a", "# b"|];
     };
     let%test "mixed comments -> comments" = {
       toggle(~lineComment="#", [|"# a", "b"|]) == [|"# a", "# b"|];
     };
     let%test "varying whitespace" = {
       toggle(~lineComment="#", [|"a", " b", "  c"|])
       == [|"# a", "#  b", "#   c"|];
     };
   });
