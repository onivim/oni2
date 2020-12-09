open Oni_Core;
open EditorCoreTypes;
open Utility;

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

module Constants = {
  let maxElementsToAnimate = 10;
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

let compare = (a, b) => {
  LineNumber.toZeroBased(a.line) - LineNumber.toZeroBased(b.line);
};

// A module storing the cached computations -
// allowing for efficient answering of the questions:
// 1) What is the full set of sorted elements?
// 2) For a given line, how much space is taken up by that line's inline elements?
// 3) For a given line, how much _total space_ is taken up by the line's inline elemenets, and all before it?
module Cache = {
  open EditorCoreTypes;

  type perLineCache = {
    inlineElementSize: float,
    totalInlineElementSize: float,
  };

  type t = {
    cache: array(perLineCache),
    allReservedSpace: float,
  };

  let empty = {
    cache:
      Array.make(0, {inlineElementSize: 0., totalInlineElementSize: 0.}),
    allReservedSpace: 0.,
  };

  let ofList = (~sortedElements: list(element)) => {
    let arraySize =
      sortedElements
      |> ListEx.last
      |> Option.map(element =>
           element.line |> EditorCoreTypes.LineNumber.toOneBased
         )
      |> Option.value(~default=0);

    let cache =
      Array.make(
        arraySize,
        {inlineElementSize: 0., totalInlineElementSize: 0.},
      );

    let setTotalSize = (~start, ~stop, ~totalSize) => {
      for (idx in start to stop) {
        cache[idx] = {
          inlineElementSize: 0.,
          totalInlineElementSize: totalSize,
        };
      };
    };

    let rec loop = (lastLine, totalSize, remainingElements) => {
      switch (remainingElements) {
      | [] => totalSize
      | [hd, ...tail] =>
        // Set indices for all the previous elements
        let currentLine = hd.line |> EditorCoreTypes.LineNumber.toZeroBased;
        if (currentLine > lastLine) {
          setTotalSize(~start=lastLine, ~stop=currentLine - 1, ~totalSize);
        };

        // Update index
        let prevSize = cache[currentLine].inlineElementSize;
        let currentLineHeight = Component_Animation.get(hd.height);

        let totalSize = currentLineHeight +. totalSize;
        cache[currentLine] = {
          inlineElementSize: prevSize +. currentLineHeight,
          totalInlineElementSize: totalSize,
        };

        loop(currentLine, totalSize, tail);
      };
    };

    let allReservedSpace = loop(0, 0., sortedElements);
    {allReservedSpace, cache};
  };

  let size = (lnum: EditorCoreTypes.LineNumber.t, {cache, _}: t) => {
    let idx = lnum |> LineNumber.toZeroBased;
    if (idx >= Array.length(cache)) {
      0.;
    } else {
      cache[idx].inlineElementSize;
    };
  };

  let totalSize =
      (lnum: EditorCoreTypes.LineNumber.t, {allReservedSpace, cache, _}) => {
    let idx = lnum |> LineNumber.toZeroBased;
    if (idx >= Array.length(cache)) {
      allReservedSpace;
    } else {
      cache[idx].totalInlineElementSize;
    };
  };

  let totalReservedSpace = ({allReservedSpace, _}) => allReservedSpace;
};

[@deriving show]
type t = {
  // A map of line number -> key -> uniqueId -> element
  keyToElements: [@opaque] IntMap.t(KeyMap.t(StringMap.t(element))),
  cache: [@opaque] Lazy.t(Cache.t),
  sortedElements: list(element),
};

let initial = {
  keyToElements: IntMap.empty,
  sortedElements: [],
  cache: Lazy.from_val(Cache.empty),
};

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

let recomputeSortedElements = keyToElements => {
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
  |> List.flatten;
};

let recomputeCache = (~sortedElements) => {
  let ret = Cache.ofList(~sortedElements);
  ret;
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

  let sortedElements = recomputeSortedElements(keyToElements');
  let cache' = Lazy.from_fun(() => recomputeCache(~sortedElements));
  {keyToElements: keyToElements', sortedElements, cache: cache'};
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

  let sortedElements = recomputeSortedElements(keyToElements');

  let cache' = Lazy.from_fun(() => recomputeCache(~sortedElements));
  {keyToElements: keyToElements', sortedElements, cache: cache'};
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

let setSize = (~animated, ~key, ~line, ~uniqueId, ~height, model) => {
  let setHeight = (curr: element) => {
    {
      ...curr,
      opacity: animated ? curr.opacity : Component_Animation.constant(1.0),
      height:
        animated
          ? Component_Animation.make(
              Animation.expand(Component_Animation.get(curr.height), height),
            )
          : Component_Animation.constant(height),
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
  |> List.map(((_key, stringMap)) => {
       stringMap |> StringMap.bindings |> List.map(snd)
     })
  |> List.flatten
  |> List.sort(compare);
};

let allElements = ({sortedElements, _}) => sortedElements;

let getReservedSpace = (line: LineNumber.t, {cache, _}: t) => {
  Cache.totalSize(line, Lazy.force(cache));
};

let getAllReservedSpace = ({cache, _}) => {
  Cache.totalReservedSpace(Lazy.force(cache));
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

    let sortedElements = recomputeSortedElements(keyToElements');
    let cache' = Lazy.from_fun(() => recomputeCache(~sortedElements));
    {keyToElements: keyToElements', sortedElements, cache: cache'};
  };
};

let sub = model => {
  let allElements = model.sortedElements;

  let rec loop = (count, acc, elements) =>
    // Gate the number of animated elements, since the animation is expensive
    if (count > Constants.maxElementsToAnimate) {
      acc;
    } else {
      switch (elements) {
      | [] => acc
      | [elem, ...tail] =>
        if (!Component_Animation.isComplete(elem.opacity)
            || !Component_Animation.isComplete(elem.height)) {
          loop(
            count + 1,
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
          loop(count, acc, tail);
        }
      };
    };

  loop(0, [], allElements) |> Isolinear.Sub.batch;
};
