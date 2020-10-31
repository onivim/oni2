open Oni_Core;
open EditorCoreTypes;

[@deriving show]
type element = {
  reconcilerKey: [@opaque] Brisk_reconciler.Key.t,
  key: string,
  uniqueId: string,
  line: LineNumber.t,
  height: float,
  hidden: bool,
  view:
    (~theme: Oni_Core.ColorTheme.Colors.t, ~uiFont: UiFont.t, unit) =>
    Revery.UI.element,
};

[@deriving show]
type t = {
  keyToElements: [@opaque] StringMap.t(StringMap.t(element)),
  sortedElements: list(element),
};

let initial = {keyToElements: StringMap.empty, sortedElements: []};

let compare = (a, b) => {
  LineNumber.toZeroBased(a.line) - LineNumber.toZeroBased(b.line);
};

let computeSortedElements =
    (keyToElements: StringMap.t(StringMap.t(element))) => {
  keyToElements
  |> StringMap.bindings
  |> List.map(snd)
  |> List.map(innerMap => {innerMap |> StringMap.bindings |> List.map(snd)})
  |> List.flatten
  |> List.sort(compare);
};

let set = (~key: string, ~elements, model) => {
  // Create a map for new keys... and then union with our current map

  let incomingMap =
    elements
    |> List.fold_left(
         (acc, curr) => {StringMap.add(curr.uniqueId, curr, acc)},
         StringMap.empty,
       );

  // When merging - use our current measured height, but bring in the new view.
  let merge = (_key, maybeCurrent, maybeIncoming) => {
    switch (maybeCurrent, maybeIncoming) {
    | (Some(current), Some(incoming)) =>
      Some({
        ...incoming,
        reconcilerKey: current.reconcilerKey,
        height: current.height,
      })
    | (None, Some(_) as incoming) => incoming
    | (Some(_), None) => None
    | (None, None) => None
    };
  };

  let currentMap =
    model.keyToElements
    |> StringMap.find_opt(key)
    |> Option.value(~default=StringMap.empty);

  let mergedMap = StringMap.merge(merge, currentMap, incomingMap);

  let keyToElements' = StringMap.add(key, mergedMap, model.keyToElements);

  let sortedElements' = computeSortedElements(keyToElements');
  {keyToElements: keyToElements', sortedElements: sortedElements'};
};

let setSize = (~key, ~uniqueId, ~height, model) => {
  let setHeight =
    Option.map((curr: element)
      // Use the max of the current height or previous height,
      // because we might've already measured this element - use
      // the measured value if we have it. This will reserve space
      // in the case where the editor is cloned, making the
      // transition less jarring.
      => {...curr, height: max(height, curr.height)});

  let keyToElements' =
    model.keyToElements
    |> StringMap.update(
         key,
         Option.map(innerMap => {
           innerMap |> StringMap.update(uniqueId, setHeight)
         }),
       );

  let sortedElements' = computeSortedElements(keyToElements');
  {keyToElements: keyToElements', sortedElements: sortedElements'};
};

let allElements = ({sortedElements, _}) => {
  sortedElements;
};

let getReservedSpace = (line: LineNumber.t, elements: t) => {
  let lineNumber = LineNumber.toZeroBased(line);
  let rec loop = (acc, remainingElements) => {
    switch (remainingElements) {
    | [] => acc
    | [hd, ...tail] when LineNumber.toZeroBased(hd.line) <= lineNumber =>
      let height = hd.hidden ? 0. : hd.height;
      loop(acc +. height, tail);
    | _elementsPastLine => acc
    };
  };

  loop(0., elements.sortedElements);
};

let getAllReservedSpace = elements => {
  let rec loop = (acc, remainingElements) => {
    switch (remainingElements) {
    | [] => acc
    | [hd, ...tail] =>
      let height = hd.hidden ? 0. : hd.height;
      loop(acc +. height, tail);
    };
  };

  loop(0., elements.sortedElements);
};

// When there is a buffer update, shift elements as needed
let shift = (update: Oni_Core.BufferUpdate.t, model) => {
  let startLineIdx = update.startLine |> LineNumber.toZeroBased;
  let endLineIdx = update.endLine |> LineNumber.toZeroBased;

  let delta = Array.length(update.lines) - (endLineIdx - startLineIdx);

  if (update.isFull || delta == 0) {
    model;
  } else {
    let updateElement = element => {
      let lineIdx = LineNumber.toZeroBased(element.line);

      if (lineIdx < startLineIdx) {
        Some(element);
      } else if (lineIdx >= startLineIdx && lineIdx < endLineIdx) {
        Some({...element, hidden: true});
      } else {
        Some({...element, line: LineNumber.ofZeroBased(lineIdx + delta)});
      };
    };

    let updateElements = (idToElement: StringMap.t(element)) => {
      StringMap.fold(
        (key, v, acc) => {
          let maybeElement = updateElement(v);
          switch (maybeElement) {
          | None => acc
          | Some(element) => StringMap.add(key, element, acc)
          };
        },
        idToElement,
        StringMap.empty,
      );
    };

    let keyToElements' = model.keyToElements |> StringMap.map(updateElements);

    let sortedElements' = computeSortedElements(keyToElements');
    {keyToElements: keyToElements', sortedElements: sortedElements'};
  };
};
