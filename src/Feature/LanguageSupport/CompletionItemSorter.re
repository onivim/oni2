open Oni_Core;

let isSnippet = (item: CompletionItem.t) => {
  item.insertTextRules
  |> Exthost.SuggestItem.InsertTextRules.matches(
       ~rule=Exthost.SuggestItem.InsertTextRules.InsertAsSnippet,
     );
};

let compareSnippet = (~snippetSortOrder, a, b) => {
  let isSnippetA = isSnippet(a);
  let isSnippetB = isSnippet(b);

  if (snippetSortOrder == `Inline || snippetSortOrder == `Hidden) {
    0;
  } else if (isSnippetA != isSnippetB) {
    if (isSnippetA) {
      snippetSortOrder == `Top ? (-1) : 1;
    } else {
      snippetSortOrder == `Top ? 1 : (-1);
    };
  } else {
    0;
  };
};

let compare =
    (
      ~snippetSortOrder=`Inline,
      a: Filter.result(CompletionItem.t),
      b: Filter.result(CompletionItem.t),
    ) => {
  let snippetCompare = compareSnippet(~snippetSortOrder, a.item, b.item);
  if (snippetCompare == 0) {
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
  } else {
    snippetCompare;
  };
};

let%test_module "compare" =
  (module
   {
     let create = (~isSnippet, ~isFuzzyMatching, ~label, ~sortText) => {
       CompletionItem.{
         chainedCacheId: None,
         handle: None,
         label,
         kind: Exthost.CompletionKind.Method,
         detail: None,
         documentation: None,
         insertText: label,
         insertTextRules:
           isSnippet
             ? Exthost.SuggestItem.InsertTextRules.insertAsSnippet
             : Exthost.SuggestItem.InsertTextRules.none,
         filterText: label,
         sortText,
         suggestRange: None,
         commitCharacters: [],
         additionalTextEdits: [],
         command: None,
         isFuzzyMatching,
       }
       |> Filter.result;
     };

     let%test_module "snippet sorting" =
       (module
        {
          let aNotSnippet =
            create(
              ~isSnippet=false,
              ~isFuzzyMatching=false,
              ~label="a",
              ~sortText="a",
            );
          let bSnippet =
            create(
              ~isSnippet=true,
              ~isFuzzyMatching=false,
              ~label="b",
              ~sortText="b",
            );
          let cNotSnippet =
            create(
              ~isSnippet=false,
              ~isFuzzyMatching=false,
              ~label="c",
              ~sortText="c",
            );

          let%test "snippetSortOrder 'inline' sorts correctly" = {
            [cNotSnippet, bSnippet, aNotSnippet]
            |> List.sort(compare) == [aNotSnippet, bSnippet, cNotSnippet];
          };
          let%test "snippetSortOrder 'top' sorts correctly" = {
            [cNotSnippet, bSnippet, aNotSnippet]
            |> List.sort(compare(~snippetSortOrder=`Top))
            == [bSnippet, aNotSnippet, cNotSnippet];
          };
          let%test "snippetSortOrder 'bottom' sorts correctly" = {
            [cNotSnippet, bSnippet, aNotSnippet]
            |> List.sort(compare(~snippetSortOrder=`Bottom))
            == [aNotSnippet, cNotSnippet, bSnippet];
          };
        });

     let%test "sortText takes precedence" = {
       let sortTextZ =
         create(
           ~isSnippet=false,
           ~isFuzzyMatching=false,
           ~label="abc",
           ~sortText="z",
         );
       let sortTextA =
         create(
           ~isSnippet=false,
           ~isFuzzyMatching=false,
           ~label="def",
           ~sortText="a",
         );

       [sortTextZ, sortTextA]
       |> List.sort(compare) == [sortTextA, sortTextZ];
     };

     let%test "falls back to label" = {
       let abc =
         create(
           ~isSnippet=false,
           ~isFuzzyMatching=false,
           ~label="abc",
           ~sortText="a",
         );
       let def =
         create(
           ~isSnippet=false,
           ~isFuzzyMatching=false,
           ~label="def",
           ~sortText="a",
         );

       [def, abc] |> List.sort(compare) == [abc, def];
     };

     let%test "when falling back to label, prefer shorter result" = {
       let toString =
         create(
           ~isSnippet=false,
           ~isFuzzyMatching=false,
           ~label="toString",
           ~sortText="a",
         );
       let toLocaleString =
         create(
           ~isSnippet=false,
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
           ~isSnippet=false,
           ~isFuzzyMatching=true,
           ~label="forEachWithIndex",
           ~sortText="a",
         );
       let range =
         create(
           ~isSnippet=false,
           ~isFuzzyMatching=true,
           ~label="range",
           ~sortText="z",
         );

       // ...prioritize fuzzy score (well, until fzy is hooked up - string length as a proxy)
       [forEachWithIndex, range]
       |> List.sort(compare) == [range, forEachWithIndex];
     };
   });
