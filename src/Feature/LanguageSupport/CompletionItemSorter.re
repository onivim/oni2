open Oni_Core;
open Utility;

let isSnippet = (item: CompletionItem.t) => {
  item.kind == Exthost.CompletionKind.Snippet;
};

module Compare = {
  let snippetSort = (~snippetSortOrder, a, b) => {
    let isSnippetA = isSnippet(a);
    let isSnippetB = isSnippet(b);

    if (snippetSortOrder == `Inline || snippetSortOrder == `Hidden) {
      None;
    } else if (isSnippetA != isSnippetB) {
      if (isSnippetA) {
        Some(snippetSortOrder == `Top ? (-1) : 1);
      } else {
        Some(snippetSortOrder == `Top ? 1 : (-1));
      };
    } else {
      None;
    };
  };

  let score = (a: CompletionItem.t, b: CompletionItem.t) =>
    if (Float.equal(a.score, b.score)) {
      None;
    } else {
      Some(int_of_float((b.score -. a.score) *. 1000.));
    };

  let sortBySortText = (a: CompletionItem.t, b: CompletionItem.t) => {
    let compare = String.compare(a.sortText, b.sortText);
    if (compare == 0) {
      None;
    } else {
      Some(compare);
    };
  };

  let sortByLabel = (a: CompletionItem.t, b: CompletionItem.t) => {
    let aLen = String.length(a.label);
    let bLen = String.length(b.label);
    if (aLen == bLen) {
      Some(String.compare(a.label, b.label));
    } else {
      Some(aLen - bLen);
    };
  };
};

let compare =
    (~snippetSortOrder=`Inline, a: CompletionItem.t, b: CompletionItem.t) => {
  Compare.snippetSort(~snippetSortOrder, a, b)
  |> OptionEx.or_lazy(() => Compare.score(a, b))
  |> OptionEx.or_lazy(() => Compare.sortBySortText(a, b))
  |> OptionEx.or_lazy(() => Compare.sortByLabel(a, b))
  |> Option.value(~default=0);
};
