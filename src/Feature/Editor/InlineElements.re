open Oni_Core;
open EditorCoreTypes;

module Animation = {
  open Revery;
  open Revery.UI;
  let fadeIn =
    Animation.(animate(Time.milliseconds(500)) |> tween(0., 1.0));

  let expand = (previousHeight, height) =>
    Animation.(
      animate(Time.milliseconds(200)) |> tween(previousHeight, height)
    );
};

type msg =
  | OpacityAnimation({
      key: string,
      line: LineNumber.t,
      uniqueId: string,
      msg: Component_Animation.msg,
    })
  | HeightAnimation({
      key: string,
      line: LineNumber.t,
      uniqueId: string,
      msg: Component_Animation.msg,
    });

[@deriving show]
type element = {
  reconcilerKey: [@opaque] Brisk_reconciler.Key.t,
  key: string,
  uniqueId: string,
  line: LineNumber.t,
  height: [@opaque] Component_Animation.t(float),
  opacity: [@opaque] Component_Animation.t(float),
  view:
    (~theme: Oni_Core.ColorTheme.Colors.t, ~uiFont: UiFont.t, unit) =>
    Revery.UI.element,
};

// A map of inline element key -> inline elements
// An example of a key might be `"codelens"`, `"diff"` -
// a category of inline elements.
module KeyMap = StringMap;

[@deriving show]
type t = {
  // A map of line number -> key -> uniqueId -> element
  keyToElements: [@opaque] IntMap.t(KeyMap.t(StringMap.t(element))),
  sortedElements: list(element),
};

let initial = {keyToElements: IntMap.empty, sortedElements: []};

let toString = ({keyToElements, _}) => {
  let str = ref("");
  keyToElements
  |> IntMap.iter((line, keyMap) => {
       str := str^ ++ "-- LINE: " ++ string_of_int(line) ++ "\n";
       keyMap
       |> KeyMap.iter((key, idMap) => {
            str := str^ ++ "--- KEY: " ++ key ++ "\n";

            idMap
            |> StringMap.iter((uniqueId, elem) => {
                 str :=
                   str^
                   ++ "--- "
                   ++ uniqueId
                   ++ "("
                   ++ string_of_float(Component_Animation.get(elem.height))
                   ++ ")\n"
               });
          });
     });
  str^;
};

let lines = ({keyToElements, _}) =>
  keyToElements |> IntMap.bindings |> List.map(fst);

let compare = (a, b) => {
  LineNumber.toZeroBased(a.line) - LineNumber.toZeroBased(b.line);
};

let computeSortedElements =
    (keyToElements: IntMap.t(KeyMap.t(StringMap.t(element)))) => {
  keyToElements
  |> IntMap.bindings
  |> List.map(((lineNumber, innerMap)) => {
       innerMap
       |> KeyMap.bindings
       |> List.map(bindings => (lineNumber, snd(bindings)))
     })
  |> List.flatten
  |> List.map(((lineNumber, uniqueIdMap)) => {
       uniqueIdMap
       |> StringMap.bindings
       |> List.map(((_uniqueId, elem)) => {
            {...elem, line: LineNumber.ofZeroBased(lineNumber)}
          })
     })
  |> List.flatten
  |> List.sort(compare);
};

let elementsToMap = (elements: list(element)) => {
  elements
  |> List.fold_left(
       (acc, curr) => {
         let lineNumber = curr.line |> LineNumber.toZeroBased;
         acc
         |> IntMap.update(
              lineNumber,
              fun
              | None =>
                Some(StringMap.empty |> StringMap.add(curr.uniqueId, curr))
              | Some(map) => Some(map |> StringMap.add(curr.uniqueId, curr)),
            );
       },
       IntMap.empty,
     );
};

let set = (~key: string, ~elements, model) => {
  let mergeLine =
      (
        previousLineElements: StringMap.t(element),
        newLineElements: StringMap.t(element),
      ) => {
    // Special case - replacement. In the case where the codelens
    // is a single element, we want to handle 'replace' gracefully.
    // For example, switching from `int` to `float` should not trigger a new transition.

    let isReplace =
      previousLineElements != StringMap.empty
      && newLineElements != StringMap.empty;

    let maybeSentinelValue =
      previousLineElements
      |> StringMap.bindings
      |> (l => List.nth_opt(l, 0) |> Option.map(snd));

    StringMap.merge(
      (_key, maybePrev, maybeNew) => {
        switch (maybePrev, maybeNew) {
        | (Some(_) as prev, Some(_)) => prev
        | (Some(_), None) => None
        | (None, Some(_) as newItem) =>
          if (isReplace) {
            Utility.OptionEx.map2(
              (newItem, prevItem) =>
                {
                  ...newItem,
                  height: prevItem.height,
                  opacity: prevItem.opacity,
                },
              newItem,
              maybeSentinelValue,
            );
          } else {
            newItem;
          }
        | (None, None) => None
        }
      },
      previousLineElements,
      newLineElements,
    );
  };

  let mergeKeys = (previousLineMap, incomingLineMap) => {
    KeyMap.merge(
      (_key, maybePrev, maybeNew) => {
        switch (maybePrev, maybeNew) {
        | (Some(prev), Some(incoming)) => Some(mergeLine(prev, incoming))
        | (None, Some(incoming)) => Some(incoming)
        | (Some(_), None) => None
        | (None, None) => None
        }
      },
      previousLineMap,
      incomingLineMap,
    );
  };

  let incomingMap = elements |> elementsToMap;

  let keyToElements' =
    IntMap.merge(
      (_line, maybePrev, maybeIncoming) => {
        switch (maybePrev, maybeIncoming) {
        | (Some(prev), Some(incoming)) =>
          let incomingKeyMap =
            StringMap.empty |> StringMap.add(key, incoming);
          Some(mergeKeys(prev, incomingKeyMap));
        | (None, Some(incoming)) =>
          let incomingKeyMap =
            StringMap.empty |> StringMap.add(key, incoming);
          Some(incomingKeyMap);
        | (Some(prev), None) => Some(prev |> StringMap.remove(key))
        | (None, None) => None
        }
      },
      model.keyToElements,
      incomingMap,
    );

  let sortedElements' = computeSortedElements(keyToElements');
  {keyToElements: keyToElements', sortedElements: sortedElements'};
};

let updateElement = (~key, ~uniqueId, ~line, ~f: element => element, model) => {
  let lineNumber = EditorCoreTypes.LineNumber.toZeroBased(line);
  let keyToElements' =
    model.keyToElements
    |> IntMap.update(
         lineNumber,
         Option.map(keyMap => {
           keyMap
           |> KeyMap.update(
                key,
                Option.map(idMap => {
                  idMap |> StringMap.update(uniqueId, Option.map(f))
                }),
              )
         }),
       );

  let sortedElements' = computeSortedElements(keyToElements');
  {keyToElements: keyToElements', sortedElements: sortedElements'};
};

let update = (msg, model) =>
  switch (msg) {
  | OpacityAnimation({key, uniqueId, line, msg}) =>
    let updateOpacity = element => {
      ...element,
      opacity: Component_Animation.update(msg, element.opacity),
    };
    updateElement(~key, ~uniqueId, ~line, ~f=updateOpacity, model);
  | HeightAnimation({key, uniqueId, line, msg}) =>
    let updateHeight = element => {
      ...element,
      height: Component_Animation.update(msg, element.height),
    };
    updateElement(~key, ~uniqueId, ~line, ~f=updateHeight, model);
  };

let setSize = (~key, ~line, ~uniqueId, ~height, model) => {
  let setHeight = (curr: element) => {
    {
      ...curr,
      height:
        Component_Animation.make(
          Animation.expand(Component_Animation.get(curr.height), height),
        ),
    };
  };
  updateElement(~key, ~line, ~uniqueId, ~f=setHeight, model);
};

let allElementsForLine = (~line, {keyToElements, _}) => {
  let keyToElements =
    IntMap.find_opt(
      EditorCoreTypes.LineNumber.toZeroBased(line),
      keyToElements,
    )
    |> Option.value(~default=StringMap.empty);

  keyToElements
  |> StringMap.bindings
  |> List.map(snd)
  |> List.map(stringMap => {
       stringMap |> StringMap.bindings |> List.map(snd)
     })
  |> List.flatten
  |> List.sort(compare);
};

let allElements = ({sortedElements, _}) => sortedElements;

let getReservedSpace = (line: LineNumber.t, elements: t) => {
  let lineNumber = LineNumber.toZeroBased(line);
  let rec loop = (acc, remainingElements) => {
    switch (remainingElements) {
    | [] => acc
    | [hd, ...tail] when LineNumber.toZeroBased(hd.line) <= lineNumber =>
      loop(acc +. Component_Animation.get(hd.height), tail)
    | _elementsPastLine => acc
    };
  };

  loop(0., elements.sortedElements);
};

let getAllReservedSpace = elements => {
  let rec loop = (acc, remainingElements) => {
    switch (remainingElements) {
    | [] => acc
    | [hd, ...tail] => loop(acc +. Component_Animation.get(hd.height), tail)
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
    let keyToElements' =
      model.keyToElements
      |> IntMap.shift(
           ~default=_ => None,
           ~startPos=startLineIdx,
           ~endPos=endLineIdx,
           ~delta,
         );

    let sortedElements' = computeSortedElements(keyToElements');
    {keyToElements: keyToElements', sortedElements: sortedElements'};
  };
};

let sub = model => {
  let allElements = model.sortedElements;

  let rec loop = (acc, elements) => {
    switch (elements) {
    | [] => acc
    | [elem, ...tail] =>
      if (!Component_Animation.isComplete(elem.opacity)
          || !Component_Animation.isComplete(elem.height)) {
        loop(
          [
            Component_Animation.sub(elem.height)
            |> Isolinear.Sub.map(msg =>
                 HeightAnimation({
                   key: elem.key,
                   line: elem.line,
                   uniqueId: elem.uniqueId,
                   msg,
                 })
               ),
            Component_Animation.sub(elem.opacity)
            |> Isolinear.Sub.map(msg =>
                 OpacityAnimation({
                   key: elem.key,
                   line: elem.line,
                   uniqueId: elem.uniqueId,
                   msg,
                 })
               ),
            ...acc,
          ],
          tail,
        );
      } else {
        loop(acc, tail);
      }
    };
  };

  loop([], allElements) |> Isolinear.Sub.batch;
};
