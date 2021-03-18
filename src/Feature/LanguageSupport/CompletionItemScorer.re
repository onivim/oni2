open EditorCoreTypes;
open Oni_Core;
open Utility;

module Constants = {
  let firstCharacterScore = 100.;
};

let traverse =
    (
      ~f: (~isMatch: bool, ~byte: int, 'acc) => 'acc,
      ~initial,
      ~selector: CompletionItem.t => string,
      line: string,
      item: CompletionItem.t,
      cursor: CharacterPosition.t,
    ) => {
  let lineByteLength = line |> String.length;
  let itemText = selector(item);
  let itemByteLength = itemText |> String.length;

  let {start: startCharacter, stop: stopCharacter}: CharacterSpan.t =
    CompletionItem.replaceSpan(~activeCursor=cursor, item);

  let startLineByte =
    line |> StringEx.characterToByte(~index=startCharacter) |> ByteIndex.toInt;

  let stopLineByte =
    line |> StringEx.characterToByte(~index=stopCharacter) |> ByteIndex.toInt;

  let rec loop = (acc, lineByte, itemByte) =>
    if (lineByte >= lineByteLength
        || lineByte >= stopLineByte
        || itemByte >= itemByteLength) {
      acc;
    } else {
      let (lineUchar, nextLineByte) = Zed_utf8.extract_next(line, lineByte);
      let (byteUchar, nextItemByte) =
        Zed_utf8.extract_next(itemText, itemByte);

      // Got a match
      if (Uchar.equal(lineUchar, byteUchar)) {
        let acc' = f(~isMatch=true, ~byte=itemByte, acc);
        loop(acc', nextLineByte, nextItemByte);
      } else {
        let acc' = f(~isMatch=false, ~byte=itemByte, acc);
        loop(acc', lineByte, nextItemByte);
      };
    };

  loop(initial, startLineByte, 0);
};

let score = {
  let initial = (0., Constants.firstCharacterScore);

  let f = (~isMatch, ~byte as _, acc) => {
    let (score, nextBonus) = acc;

    if (isMatch) {
      (score +. nextBonus, nextBonus /. 2.);
    } else {
      (score, nextBonus /. 2.);
    };
  };

  let selector = (item: CompletionItem.t) => item.filterText;

  (line, item, cursor) => {
    let (score, _) = traverse(~f, ~initial, ~selector, line, item, cursor);
    score;
  };
};

let highlights = {
  let buildHighlightsMap = (line, item: CompletionItem.t, cursor) => {
    // Map of uchar -> occurrence count
    let initial = IntMap.empty;

    let f = (~isMatch, ~byte, acc) =>
      if (isMatch) {
        let uchar = Zed_utf8.extract(item.filterText, byte);
        IntMap.update(
          Uchar.to_int(uchar),
          fun
          | None => Some(1)
          | Some(v) => Some(v + 1),
          acc,
        );
      } else {
        acc;
      };

    let selector = (item: CompletionItem.t) => item.filterText;
    traverse(~f, ~initial, ~selector, line, item, cursor);
  };

  (line, item: CompletionItem.t, cursor) => {
    let highlightsMap = buildHighlightsMap(line, item, cursor);

    let initial = (false, []);

    let f = (~isMatch, ~byte, acc) => {
      let (isInHighlight, highlights) = acc;

      // Not in a highlight, not in a match... return what we have
      if (!isMatch) {
        (false, highlights);
      } else if (isInHighlight) {
        // In a highlight, and a match, extend current range

        let highlights' =
          switch (highlights) {
          | [] => [(byte, byte)]
          | [(low, _), ...tail] => [(low, byte), ...tail]
          };
        (true, highlights');
      } else {
        (
          // Start new range
          true,
          [(byte, byte), ...highlights],
        );
      };
    };

    let len = String.length(item.label);
    let rec loop = (highlightsMap, acc, byteIdx) =>
      if (byteIdx >= len) {
        acc;
      } else {
        let (uchar, nextByte) = Zed_utf8.extract_next(item.label, byteIdx);
        let ucharCode = Uchar.to_int(uchar);
        let isIndexHighlighted =
          IntMap.find_opt(ucharCode, highlightsMap)
          |> Option.map(count => count > 0)
          |> Option.value(~default=false);

        let highlightsMap' =
          IntMap.update(
            ucharCode,
            fun
            | None => None
            | Some(count) => Some(count - 1),
            highlightsMap,
          );

        let acc' = f(~isMatch=isIndexHighlighted, ~byte=byteIdx, acc);
        loop(highlightsMap', acc', nextByte);
      };

    let (_, highlights) = loop(highlightsMap, initial, 0);
    highlights |> List.rev;
  };
};
