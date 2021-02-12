open EditorCoreTypes;
open Oni_Core;
open Utility;
open Snippet;

type snippet = Snippet.t;

module Placeholder = {
  type t = IntMap.t(list(Snippet.segment));

  type positions =
    | Positions(list(BytePosition.t))
    | Ranges(list(ByteRange.t));

  type position =
    | Position(BytePosition.t)
    | Range(ByteRange.t);

  let normalizePositions: list(position) => positions =
    positions => {
      let isRange =
        fun
        | Range(_) => true
        | Position(_) => false;

      let isAllRanges = positions |> List.for_all(isRange);

      if (isAllRanges) {
        // We can map this to a `Ranges`

        let rec loop = (acc, positions) => {
          switch (positions) {
          | [] => acc
          | [hd, ...tail] =>
            switch (hd) {
            | Range(range) => loop([range, ...acc], tail)
            | Position(_) => loop(acc, tail)
            }
          };
        };

        Ranges(loop([], positions));
      } else {
        let rec loop = (acc, positions) => {
          switch (positions) {
          | [] => acc
          | [hd, ...tail] =>
            switch (hd) {
            | Range(_) => loop(acc, tail)
            | Position(pos) => loop([pos, ...acc], tail)
            }
          };
        };

        Positions(loop([], positions));
      };
    };

  let hasAny = (snippet: Snippet.t) => {
    let rec lineHasPlaceHolder = (snippetLine: list(Snippet.segment)) => {
      switch (snippetLine) {
      | [] => false
      | [hd, ...tail] =>
        Snippet.(
          switch (hd) {
          | Text(_) => lineHasPlaceHolder(tail)
          | Placeholder(_) => true
          | Choice(_) => true
          | Variable(_) => lineHasPlaceHolder(tail)
          }
        )
      };
    };

    snippet |> List.exists(lineHasPlaceHolder);
  };

  // Get a placeholder as text
  let text = (~index: int, placeholders: t) => {
    // Add a max recursion depth... so we don't get stuck
    // ping-ponging between recursive placeholders.
    let calculate = (cache, index, placeholders) => {
      placeholders
      |> IntMap.find_opt(index)
      |> Option.map(segments => {
           let rec loop = (cache, acc, remaining) => {
             switch (remaining) {
             | [] => (cache, acc)
             | [hd, ...tail] =>
               switch (hd) {
               | Text(text) => loop(cache, acc ++ text, tail)
               | Placeholder({index: placeholderIndex, contents}) =>
                 if (placeholderIndex == index) {
                   // This is a nested one: ${2:${2}}... skip
                   loop(
                     cache,
                     acc,
                     tail,
                   );
                 } else {
                   // A nested placeholder we haven't encountered...
                   // is it cached or do we need to calculate it, too?
                   switch (IntMap.find_opt(placeholderIndex, cache)) {
                   // Not cached... let's calculate
                   | None =>
                     let (newCache, text) = loop(cache, "", contents);
                     loop(newCache, acc ++ text, tail);

                   | Some(text) => loop(cache, acc ++ text, tail)
                   };
                 }
               | Choice({choices, _}) =>
                 let text =
                   List.nth_opt(choices, 0) |> Option.value(~default="");

                 loop(cache, acc ++ text, tail);
               | Variable(_) => loop(cache, acc, tail)
               }
             };
           };

           loop(cache, "", segments);
         });
    };

    calculate(IntMap.empty, index, placeholders) |> Option.map(snd);
  };

  let%test_module "Placeholder.text" =
    (module
     {
       let make = placeholders =>
         placeholders
         |> List.fold_left(
              (acc, curr) => {
                let (index, segments) = curr;
                IntMap.add(index, segments, acc);
              },
              IntMap.empty,
            );

       let%test "simple placeholder" = {
         let placeholders = make([(1, [Text("abc")])]);

         text(~index=1, placeholders) == Some("abc");
       };

       let%test "empty placeholder" = {
         let placeholders = make([(1, [])]);

         text(~index=1, placeholders) == Some("");
       };

       let%test "non-existent placeholder" = {
         let placeholders = make([(1, [])]);

         text(~index=2, placeholders) == None;
       };
       let%test "nested placeholder" = {
         let placeholders =
           make([
             (1, [Text("abc")]),
             (
               2,
               [
                 Text("def"),
                 Placeholder({index: 1, contents: [Text("abc")]}),
               ],
             ),
           ]);

         text(~index=2, placeholders) == Some("defabc");
       };
     });

  let positions = (~placeholders: t, ~index: int, snippet: snippet) => {
    placeholders
    |> IntMap.find_opt(index)
    |> Option.map(_placeholder => {
         let rec loop =
                 (
                   lineNumber: EditorCoreTypes.LineNumber.t,
                   acc: list(position),
                   offset: int,
                   segments: list(Snippet.segment),
                 ) => {
           switch (segments) {
           | [] => (acc, offset)
           | [hd, ...tail] =>
             switch (hd) {
             | Text(text) =>
               loop(lineNumber, acc, offset + String.length(text), tail)

             | Placeholder({index: placeholderIndex, contents}) =>
               // If this is the index we're looking for, add it to our list
               if (index == placeholderIndex) {
                 let placeholderText =
                   text(~index, placeholders) |> Option.value(~default="");
                 let len = String.length(placeholderText);
                 if (len > 0) {
                   // Add a byte range
                   let acc' = [
                     Range(
                       ByteRange.{
                         start: {
                           line: lineNumber,
                           byte: ByteIndex.ofInt(offset),
                         },
                         stop: {
                           line: lineNumber,
                           byte: ByteIndex.ofInt(offset + len - 1),
                         },
                       },
                     ),
                     ...acc,
                   ];
                   loop(lineNumber, acc', offset + len, tail);
                 } else {
                   // Add a byte position
                   let acc' = [
                     Position(
                       BytePosition.{
                         line: lineNumber,
                         byte: ByteIndex.ofInt(offset),
                       },
                     ),
                     ...acc,
                   ];
                   loop(lineNumber, acc', offset, tail);
                 };
               } else {
                 // Have to dive into this segment...
                 let (acc', offset') =
                   loop(lineNumber, acc, offset, contents);
                 loop(lineNumber, acc', offset', tail);
               }

             // TODO: Handle variable / choice
             | Variable(_)
             | Choice(_) => loop(lineNumber, acc, offset, tail)
             }
           };
         };

         snippet
         |> List.fold_left(
              (acc, curr) => {
                let (idx, currentPositions) = acc;
                let (newPositions, _offset) =
                  loop(
                    EditorCoreTypes.LineNumber.ofZeroBased(idx),
                    [],
                    0,
                    curr,
                  );
                let allPositions = currentPositions @ newPositions;
                (idx + 1, allPositions);
              },
              (0, []),
            )
         |> snd
         |> normalizePositions;
       });
  };

  let%test_module "Placeholder.positions" =
    (module
     {
       let make = placeholders =>
         placeholders
         |> List.fold_left(
              (acc, curr) => {
                let (index, segments) = curr;
                IntMap.add(index, segments, acc);
              },
              IntMap.empty,
            );

       let%test "simple empty placeholder" = {
         let placeholder1 = [];
         let placeholders = make([(1, placeholder1)]);

         let snippet = [
           [Text("abc"), Placeholder({index: 1, contents: placeholder1})],
         ];

         positions(~index=1, ~placeholders, snippet)
         == Some(
              Positions([
                BytePosition.{
                  line: EditorCoreTypes.LineNumber.zero,
                  byte: ByteIndex.(zero + 3),
                },
              ]),
            );
       };

       let%test "simple placeholder range" = {
         let placeholder1 = [Text("abc")];
         let placeholders = make([(1, placeholder1)]);

         let snippet = [
           [Text("abc"), Placeholder({index: 1, contents: placeholder1})],
         ];

         positions(~index=1, ~placeholders, snippet)
         == Some(
              Ranges([
                ByteRange.{
                  start: {
                    line: EditorCoreTypes.LineNumber.zero,
                    byte: ByteIndex.(zero + 3),
                  },
                  stop: {
                    line: EditorCoreTypes.LineNumber.zero,
                    byte: ByteIndex.(zero + 5),
                  },
                },
              ]),
            );
       };
     });

  let extractPlaceholders = (snippet: snippet) => {
    let rec populatePlaceholders = (map, segments) => {
      switch (segments) {
      | [] => map
      | [hd, ...tail] =>
        switch (hd) {
        // Move forward...
        | Text(_)
        | Variable(_) => populatePlaceholders(map, tail)

        // Bring in placeholders
        | Choice({index, choices}) =>
          let text = List.nth_opt(choices, 0) |> Option.value(~default="");
          let map' = IntMap.add(index, [Text(text)], map);
          populatePlaceholders(map', tail);

        | Placeholder({index, contents}) =>
          let map' = IntMap.add(index, contents, map);
          // Handle nested placeholders
          let map'' = populatePlaceholders(map', contents);
          // And then continue moving across the rest of hte line
          populatePlaceholders(map'', tail);
        }
      };
    };

    snippet
    |> List.fold_left(
         (acc, snippetLine) => {populatePlaceholders(acc, snippetLine)},
         IntMap.empty,
       );
  };

  let all = placeholders => {
    let candidates = placeholders |> IntMap.bindings |> List.map(fst);
    let sorted = candidates |> List.filter(idx => idx != 0);

    let hasZero = List.exists(idx => idx == 0, candidates);
    // Add zero back to the very end
    if (hasZero) {
      sorted @ [0];
    } else {
      sorted;
    };
  };
  let move = (~f, ~placeholder, placeholders) => {
    let sortedPlaceholders = all(placeholders);
    let len = List.length(sortedPlaceholders);

    let maybeMatch =
      sortedPlaceholders
      |> List.mapi((idx, item) => (idx, item))
      |> List.filter(((_idx, item)) => item == placeholder)
      |> (l => List.nth_opt(l, 0));

    maybeMatch
    |> Option.map(fst)
    |> OptionEx.flatMap(newIdx => {
         let destIdx = IntEx.clamp(~hi=len - 1, ~lo=0, f(newIdx));
         List.nth_opt(sortedPlaceholders, destIdx);
       })
    |> Option.value(~default=0);
  };

  let next = (~placeholder, placeholders) => {
    move(~f=idx => idx + 1, ~placeholder, placeholders);
  };

  let previous = (~placeholder, placeholders) => {
    move(~f=idx => idx - 1, ~placeholder, placeholders);
  };

  let final = placeholders => {
    let revPlaceholders = all(placeholders) |> List.rev;

    List.nth_opt(revPlaceholders, 0) |> Option.value(~default=0);
  };

  let initial = placeholders => {
    let nonZeroIndices =
      placeholders
      |> IntMap.bindings
      |> List.map(fst)
      |> List.filter(idx => idx != 0)
      |> List.sort(compare);

    let initialIndex =
      List.nth_opt(nonZeroIndices, 0) |> Option.value(~default=0);

    initialIndex;
  };
};

let placeholders = Placeholder.extractPlaceholders;

let resolve =
    (
      ~getVariable: string => option(string),
      ~prefix: string,
      ~postfix: string,
      ~indentationSettings: Oni_Core.IndentationSettings.t,
      snippet,
    ) => {
  let lines = List.length(snippet);
  let hasAnyPlaceholders = Placeholder.hasAny(snippet);

  // Keep track of the leading whitespace, so we can add it on subsequent lines
  let leadingWhitespace = StringEx.leadingWhitespace(prefix);

  let rec resolveVariables = segments =>
    switch (segments) {
    | [] => []
    | [Variable({name, default}), ...tail] =>
      getVariable(name)
      |> OptionEx.or_(default)
      |> Option.map(v => [Text(v), ...resolveVariables(tail)])
      |> Option.value(~default=resolveVariables(tail))
    | [nonVariable, ...tail] => [nonVariable, ...resolveVariables(tail)]
    };

  // Until we fully support choice w/ a contextual menu -
  // convert them into placeholders so we get a proper tab-through experience.
  let rec convertChoicesToPlaceholders = segments =>
    switch (segments) {
    | [] => []
    | [Choice({index, choices}), ...tail] =>
      let text = List.nth_opt(choices, 0) |> Option.value(~default="");
      [
        Placeholder({index, contents: [Text(text)]}),
        ...convertChoicesToPlaceholders(tail),
      ];
    | [nonChoice, ...tail] => [
        nonChoice,
        ...convertChoicesToPlaceholders(tail),
      ]
    };

  let normalizeWhitespace = segments =>
    switch (segments) {
    | [Text(text), ...tail] => [
        Text(
          IndentationSettings.normalizeTabs(
            ~indentation=indentationSettings,
            text,
          ),
        ),
        ...tail,
      ]
    | nonText => nonText
    };

  snippet
  // Iterate through the lines and resolve all variables like `$BLOCK_COMMENT_START`
  |> List.map(resolveVariables)
  |> List.map(convertChoicesToPlaceholders)
  |> List.mapi((idx, line) => {
       let isFirst = idx == 0;
       let isLast = idx == lines - 1;

       // Add prefix
       let line' =
         if (isFirst) {
           [Text(prefix), ...normalizeWhitespace(line)];
         } else if (leadingWhitespace != "") {
           [Text(leadingWhitespace), ...normalizeWhitespace(line)];
         } else {
           normalizeWhitespace(line);
         };

       // Add postfix
       let line'' =
         if (isLast) {
           let revLine' = line' |> List.rev;
           let finalPlaceholder =
             if (!hasAnyPlaceholders) {
               [Placeholder({index: 0, contents: []})];
             } else {
               [];
             };
           [Text(postfix)] @ finalPlaceholder @ revLine' |> List.rev;
         } else {
           line';
         };

       line'';
     });
};

let%test_module "resolve" =
  (module
   {
     let useTabs =
       IndentationSettings.(create(~mode=Tabs, ~size=4, ~tabSize=4, ()));
     let useSpaces2 =
       IndentationSettings.(create(~mode=Spaces, ~size=2, ~tabSize=2, ()));

     let%test "normalizes whitespace" = {
       let raw = parse("abc\n\tdef") |> Result.get_ok;

       let resolved =
         resolve(
           ~getVariable=_ => None,
           ~prefix="PREFIX",
           ~postfix="POSTFIX",
           ~indentationSettings=useSpaces2,
           raw,
         );

       resolved
       == [
            [Text("PREFIX"), Text("abc")],
            [
              // The tab should be replaced with spaces
              Text("  def"),
              Placeholder({index: 0, contents: []}),
              Text("POSTFIX"),
            ],
          ];
     };

     let%test "adds prefix and postfix in single line" = {
       let raw = parse("abc") |> Result.get_ok;

       let resolved =
         resolve(
           ~getVariable=_ => None,
           ~prefix="PREFIX",
           ~postfix="POSTFIX",
           ~indentationSettings=useTabs,
           raw,
         );

       resolved
       == [
            [
              Text("PREFIX"),
              Text("abc"),
              Placeholder({index: 0, contents: []}),
              Text("POSTFIX"),
            ],
          ];
     };

     let%test "adds prefix and postfix when multi-line" = {
       let raw = parse("abc\ndef") |> Result.get_ok;

       let resolved =
         resolve(
           ~getVariable=_ => None,
           ~prefix="PREFIX",
           ~postfix="POSTFIX",
           ~indentationSettings=useTabs,
           raw,
         );

       resolved
       == [
            [Text("PREFIX"), Text("abc")],
            [
              Text("def"),
              Placeholder({index: 0, contents: []}),
              Text("POSTFIX"),
            ],
          ];
     };
     let%test "persists whitespace across lines" = {
       let raw = parse("abc\ndef\nghi") |> Result.get_ok;

       let resolved =
         resolve(
           ~getVariable=_ => None,
           ~prefix="  PREFIX",
           ~postfix="POSTFIX",
           ~indentationSettings=useTabs,
           raw,
         );

       resolved
       == [
            [Text("  PREFIX"), Text("abc")],
            [Text("  "), Text("def")],
            [
              Text("  "),
              Text("ghi"),
              Placeholder({index: 0, contents: []}),
              Text("POSTFIX"),
            ],
          ];
     };
     let%test "variable resolution replaces variable with provided text" = {
       let raw = parse("$TESTVAR") |> Result.get_ok;

       let resolved =
         resolve(
           ~getVariable=_ => Some("!!Expanded"),
           ~prefix="pre",
           ~postfix="post",
           ~indentationSettings=useTabs,
           raw,
         );

       resolved
       == [
            [
              Text("pre"),
              Text("!!Expanded"),
              Placeholder({index: 0, contents: []}),
              Text("post"),
            ],
          ];
     };
     let%test "variable resolution uses default" = {
       let raw = parse("${TESTVAR:DEFAULT}") |> Result.get_ok;

       let resolved =
         resolve(
           ~getVariable=_ => None,
           ~prefix="pre",
           ~postfix="post",
           ~indentationSettings=useTabs,
           raw,
         );

       resolved
       == [
            [
              Text("pre"),
              Text("DEFAULT"),
              Placeholder({index: 0, contents: []}),
              Text("post"),
            ],
          ];
     };
   });

let updatePlaceholder = (~index: int, ~text: string, snippet: snippet) => {
  let rec map = segment =>
    switch (segment) {
    | Placeholder({index: placeholderIndex, contents}) =>
      if (index == placeholderIndex) {
        Placeholder({index, contents: [Text(text)]});
      } else {
        Placeholder({
          index: placeholderIndex,
          contents: contents |> List.map(map),
        });
      }
    | other => other
    };

  snippet |> List.map(segments => segments |> List.map(map));
};

let getFirstLineIndexWithPlaceholder = (~index: int, snippet: snippet) => {
  let rec hasPlaceholder = segments =>
    switch (segments) {
    | [] => false
    | [Placeholder({index: placeholderIndex, contents}), ...tail] =>
      if (placeholderIndex == index) {
        true;
      } else {
        hasPlaceholder(contents) || hasPlaceholder(tail);
      }
    | [_other, ...tail] => hasPlaceholder(tail)
    };

  let snippetsWithIndices =
    snippet
    |> List.mapi((idx, snippetLine) => (idx, snippetLine))
    |> List.filter(((_idx, snippetLine)) => hasPlaceholder(snippetLine));

  List.nth_opt(snippetsWithIndices, 0) |> Option.map(fst);
};

let getPlaceholderCountForLine = (~index: int, ~line: int, snippet: snippet) => {
  let rec countPlaceholders = (acc, segments) =>
    switch (segments) {
    | [] => acc
    | [Placeholder({index: placeholderIndex, contents}), ...tail] =>
      let acc' = placeholderIndex == index ? acc + 1 : acc;
      let placeholderCount = countPlaceholders(0, contents);
      countPlaceholders(acc' + placeholderCount, tail);
    | [_other, ...tail] => countPlaceholders(acc, tail)
    };

  List.nth_opt(snippet, line)
  |> Option.map(countPlaceholders(0))
  |> Option.value(~default=0);
};

let%test_module "updatePlaceholder" =
  (module
   {
     let%test "placeholder gets updated" = {
       let snippet = [[Placeholder({index: 1, contents: [Text("abc")]})]];

       let expected = [
         [Placeholder({index: 1, contents: [Text("def")]})],
       ];
       updatePlaceholder(~index=1, ~text="def", snippet) == expected;
     };
     let%test "nested placeholder gets updated" = {
       let snippet = [
         [
           Placeholder({
             index: 1,
             contents: [Placeholder({index: 2, contents: [Text("abc")]})],
           }),
         ],
       ];

       let expected = [
         [
           Placeholder({
             index: 1,
             contents: [Placeholder({index: 2, contents: [Text("def")]})],
           }),
         ],
       ];
       updatePlaceholder(~index=2, ~text="def", snippet) == expected;
     };
     let%test "nested placeholder removed if outer is updated" = {
       let snippet = [
         [
           Placeholder({
             index: 1,
             contents: [Placeholder({index: 2, contents: [Text("abc")]})],
           }),
         ],
       ];

       let expected = [
         [Placeholder({index: 1, contents: [Text("def")]})],
       ];
       updatePlaceholder(~index=1, ~text="def", snippet) == expected;
     };
   });

let toLines = (resolvedSnippet: snippet) => {
  let rec lineToString = (acc, line: list(Snippet.segment)) =>
    switch (line) {
    | [] => acc
    | [hd, ...tail] =>
      let acc' = acc ++ segmentToString(hd);
      lineToString(acc', tail);
    }
  and segmentToString = segment =>
    switch (segment) {
    | Text(str) => str
    | Placeholder({contents, _}) => lineToString("", contents)
    | Choice({choices, _}) =>
      List.nth_opt(choices, 0) |> Option.value(~default="")
    | Variable({name, default}) => default |> Option.value(~default=name)
    };

  resolvedSnippet |> List.map(lineToString("")) |> Array.of_list;
};

let lineCount = snippet => List.length(snippet);

type t = snippet;
