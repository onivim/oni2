open Oni_Core;

let compare =
    (a: Filter.result(CompletionItem.t), b: Filter.result(CompletionItem.t)) => {
  // First, use the sortText, if available
  let sortValue =
    if (!a.item.isFuzzyMatching && !b.item.isFuzzyMatching) {
      String.compare(a.item.sortText, b.item.sortText);
    } else {
      0;
      // If we're fuzzy matching,
    };

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
     let create = (~isFuzzyMatching, ~label, ~sortText) => {
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
         isFuzzyMatching,
       }
       |> Filter.result;
     };

     let%test "sortText takes precedence" = {
       let sortTextZ =
         create(~isFuzzyMatching=false, ~label="abc", ~sortText="z");
       let sortTextA =
         create(~isFuzzyMatching=false, ~label="def", ~sortText="a");

       [sortTextZ, sortTextA]
       |> List.sort(compare) == [sortTextA, sortTextZ];
     };

     let%test "falls back to label" = {
       let abc = create(~isFuzzyMatching=false, ~label="abc", ~sortText="a");
       let def = create(~isFuzzyMatching=false, ~label="def", ~sortText="a");

       [def, abc] |> List.sort(compare) == [abc, def];
     };

     let%test "when falling back to label, prefer shorter result" = {
       let toString =
         create(~isFuzzyMatching=false, ~label="toString", ~sortText="a");
       let toLocaleString =
         create(
           ~isFuzzyMatching=false,
           ~label="toLocaleString",
           ~sortText="a",
         );

       [toLocaleString, toString]
       |> List.sort(compare) == [toString, toLocaleString];
     };

     let%test "#2235: should prefer fuzzy-matches over sort text" = {
       // ...but once we start searching
       let forEachWithIndex =
         create(
           ~isFuzzyMatching=true,
           ~label="forEachWithIndex",
           ~sortText="a",
         );
       let range =
         create(~isFuzzyMatching=true, ~label="range", ~sortText="z");

       // ...prioritize fuzzy score (well, until fzy is hooked up - string length as a proxy)
       [forEachWithIndex, range]
       |> List.sort(compare) == [range, forEachWithIndex];
     };
   });
