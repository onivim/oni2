open Utility;

let indentationToTabs = (~size, str) => {
  let leadingWhitespace = StringEx.leadingWhitespace(str);
  let postWhitespace =
    String.sub(
      str,
      String.length(leadingWhitespace),
      String.length(str) - String.length(leadingWhitespace),
    );

  let regexp = Re.Perl.re(String.make(size, ' ')) |> Re.Posix.compile;
  let replacement = String.make(1, '\t');

  let newWhitespace =
    Re.replace_string(regexp, ~by=replacement, leadingWhitespace);

  newWhitespace ++ postWhitespace;
};

let%test_module "indentationToTabs" =
  (module
   {
     let%test "no spaces" = {
       indentationToTabs(~size=2, "abc") == "abc";
     };
     let%test "only whitepsace" = {
       indentationToTabs(~size=2, "  ") == "\t";
     };
     let%test "single tab" = {
       indentationToTabs(~size=2, "  abc") == "\tabc";
     };
     let%test "multiple tabs tab" = {
       indentationToTabs(~size=2, "    abc") == "\t\tabc";
     };

     let%test "spaces after leading whitespace don't get converted" = {
       indentationToTabs(~size=4, "    abc d") == "\tabc d";
     };
   });

let indentationToSpaces = (~size, str) => {
  let leadingWhitespace = StringEx.leadingWhitespace(str);
  let postWhitespace =
    String.sub(
      str,
      String.length(leadingWhitespace),
      String.length(str) - String.length(leadingWhitespace),
    );

  let replacement = String.make(size, ' ');
  let rec loop = (acc, idx) =>
    if (idx >= String.length(leadingWhitespace)) {
      acc;
    } else if (leadingWhitespace.[idx] == '\t') {
      loop(acc ++ replacement, idx + 1);
    } else {
      loop(acc ++ String.make(1, leadingWhitespace.[idx]), idx + 1);
    };

  let newWhitespace = loop("", 0);
  newWhitespace ++ postWhitespace;
};

let%test_module "indentationToSpaces" =
  (module
   {
     let%test "no tabs" = {
       indentationToSpaces(~size=2, "abc") == "abc";
     };
     let%test "only whitepsace" = {
       indentationToSpaces(~size=2, "\t") == "  ";
     };
     let%test "single tab" = {
       indentationToSpaces(~size=2, "\tabc") == "  abc";
     };
     let%test "multiple tabs" = {
       indentationToSpaces(~size=3, "\t\tabc") == "      abc";
     };
     let%test "doesn't change tabs after leading whitespace tabs" = {
       indentationToSpaces(~size=1, "\t\tabc\td") == "  abc\td";
     };
   });
