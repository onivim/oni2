open Oni_Core;

let compare =
    (a: Filter.result(CompletionItem.t), b: Filter.result(CompletionItem.t)) => {
  // First, use the sortText, if available
  let sortValue = String.compare(a.item.sortText, b.item.sortText);
  if (sortValue == 0) {
    let aLen = String.length(a.item.label);
    let bLen = String.length(b.item.label);
    if (aLen == bLen) {
      String.compare(a.item.label, b.item.label);
    } else {
      aLen - bLen;
    };
  } else {
    sortValue;
  };
};

let%test_module "compare" =
  (module
   {
     let create = (~label, ~sortText) => {
       CompletionItem.{
         chainedCacheId: None,
         handle: 0,
         label,
         kind: Exthost.CompletionKind.Method,
         detail: None,
         documentation: None,
         insertText: label,
         insertTextRules: Exthost.SuggestItem.InsertTextRules.none,
         sortText,
         suggestRange: None,
         commitCharacters: [],
         additionalTextEdits: [],
         command: None,
       }
       |> Filter.result;
     };

     let%test "sortText takes precedence" = {
       let sortTextZ = create(~label="abc", ~sortText="z");
       let sortTextA = create(~label="def", ~sortText="a");

       [sortTextZ, sortTextA]
       |> List.sort(compare) == [sortTextA, sortTextZ];
     };

     let%test "falls back to label" = {
       let abc = create(~label="abc", ~sortText="a");
       let def = create(~label="def", ~sortText="a");

       [def, abc] |> List.sort(compare) == [abc, def];
     };

     let%test "when falling back to label, prefer shorter result" = {
       let toString = create(~label="toString", ~sortText="a");
       let toLocaleString = create(~label="toLocaleString", ~sortText="a");

       [toLocaleString, toString]
       |> List.sort(compare) == [toString, toLocaleString];
     };
   });
