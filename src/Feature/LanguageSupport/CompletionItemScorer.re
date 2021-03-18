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

  let selector = (item: CompletionItem.t) => item.label;

  (line, item, cursor) => {
    let (_, highlights) =
      traverse(~f, ~initial, ~selector, line, item, cursor);
    highlights |> List.rev;
  };
};
