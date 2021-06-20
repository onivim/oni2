open Oni_Core;

module Constants = {
  let maxLinesToConsider = 10000;
  let maxLineLengthToConsider = 10000;
};

module Internal = {
  let characterListToString =
    fun
    | []
    | [_]
    | [_, _] => None
    | wordCharacters => Some(wordCharacters |> Zed_utf8.rev_implode);

  let extractKeywordsFromLine =
      (~wordMap=StringMap.empty, ~isWordCharacter, str) => {
    let (remainingWord, outputWordMap) =
      Zed_utf8.fold(
        (uchar, (prevCharacters, wordMap)) => {
          let isOnWord = isWordCharacter(uchar);

          if (isOnWord) {
            (
              // Add the letter to a candidate word
              [uchar, ...prevCharacters],
              wordMap,
            );
          } else {
            switch (characterListToString(prevCharacters)) {
            // If we're empty, 1-letter, or 2-letter, just skip and continue
            | None => ([], wordMap)
            // Found a word that's at least 3 letters!
            | Some(word) =>
              let wordMap' =
                StringMap.update(
                  word,
                  fun
                  | None => Some(1)
                  | Some(count) => Some(count + 1),
                  wordMap,
                );
              ([], wordMap');
            };
          };
        },
        str,
        ([], wordMap),
      );

    // If there was a word at the end, add it too
    remainingWord
    |> characterListToString
    |> Option.map(word =>
         StringMap.update(
           word,
           fun
           | None => Some(1)
           | Some(count) => Some(count + 1),
           outputWordMap,
         )
       )
    |> Option.value(~default=outputWordMap);
  };

  let%test_module "extractKeywordsFromLine" =
    (module
     {
       let isWordCharacter = uchar =>
         LanguageConfiguration.isWordCharacter(
           uchar,
           LanguageConfiguration.default,
         );
       let%test "simple extraction" = {
         let keywords =
           extractKeywordsFromLine(
             ~wordMap=StringMap.empty,
             ~isWordCharacter,
             "abc def",
           )
           |> StringMap.bindings
           |> List.map(fst);

         keywords == ["abc", "def"];
       };

       let%test "short words ignored" = {
         let keywords =
           extractKeywordsFromLine(
             ~wordMap=StringMap.empty,
             ~isWordCharacter,
             "abc a ab def",
           )
           |> StringMap.bindings
           |> List.map(fst);

         keywords == ["abc", "def"];
       };

       let%test "utf-8" = {
         let keywords =
           extractKeywordsFromLine(
             ~wordMap=StringMap.empty,
             ~isWordCharacter,
             "abc κόσμε def",
           )
           |> StringMap.bindings
           |> List.map(fst);

         keywords == ["abc", "def", "κόσμε"];
       };
     });

  let extractKeywordsFromLines =
      (~wordMap=StringMap.empty, ~isWordCharacter, getLine) => {
    let rec loop = (wordMap, lineNumber) => {
      switch (getLine(lineNumber)) {
      | None => wordMap
      | Some(lineString) =>
        if (String.length(lineString) < Constants.maxLineLengthToConsider) {
          let wordMap' =
            extractKeywordsFromLine(~isWordCharacter, ~wordMap, lineString);
          loop(wordMap', lineNumber + 1);
        } else {
          loop(wordMap, lineNumber + 1);
        }
      };
    };
    loop(wordMap, 0)
    |> StringMap.bindings
    // Sort by occurrences
    |> List.fast_sort((a, b) => snd(b) - snd(a))
    // And return all items
    |> List.map(fst);
  };
  let%test_module "extractKeywordsFromLines" =
    (module
     {
       let isWordCharacter = uchar =>
         LanguageConfiguration.isWordCharacter(
           uchar,
           LanguageConfiguration.default,
         );
       let getLine =
         fun
         | 0 => Some("abc def def ghi")
         | 1 => Some("ghi ghi ghi ghi")
         | _ => None;

       let%test "sorted by frequency" = {
         let keywords =
           extractKeywordsFromLines(
             ~wordMap=StringMap.empty,
             ~isWordCharacter,
             getLine,
           );

         keywords == ["ghi", "def", "abc"];
       };
     });
};

let keywords = (~languageConfiguration, ~buffer) => {
  let wordMap = StringMap.empty;
  let isWordCharacter = uchar =>
    LanguageConfiguration.isWordCharacter(uchar, languageConfiguration);
  let bufferLineCount = Buffer.getNumberOfLines(buffer);

  let getLine = lineNumber =>
    if (lineNumber >= bufferLineCount
        || lineNumber >= Constants.maxLinesToConsider) {
      None;
    } else {
      let bufferLine = Buffer.getLine(lineNumber, buffer);
      let lineString = BufferLine.raw(bufferLine);
      Some(lineString);
    };

  Internal.extractKeywordsFromLines(~wordMap, ~isWordCharacter, getLine);
};
