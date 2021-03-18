open EditorCoreTypes;
open Oni_Core;
open Utility;

module Constants = {
  let firstCharacterScore = 100.;
};

let score =
    (
      line: string,
      item: CompletionItem.t,
      meetPosition: CharacterPosition.t,
      cursor: CharacterPosition.t,
    ) => {
  let lineByteLength = line |> String.length;
  let itemText = item.filterText;
  let itemByteLength = itemText |> String.length;

  let {start: startCharacter, stop: stopCharacter}: CharacterSpan.t =
    CompletionItem.replaceSpan(
      ~activeCursor=cursor,
      ~insertLocation=meetPosition,
      item,
    );

  let startLineByte =
    line |> StringEx.characterToByte(~index=startCharacter) |> ByteIndex.toInt;

  let stopLineByte =
    line |> StringEx.characterToByte(~index=stopCharacter) |> ByteIndex.toInt;

  let rec loop = (score, lineByte, itemByte, nextBonus) =>
    if (lineByte >= lineByteLength
        || lineByte >= stopLineByte
        || itemByte >= itemByteLength) {
      score;
    } else {
      let (lineUchar, nextLineByte) = Zed_utf8.extract_next(line, lineByte);
      let (byteUchar, nextItemByte) =
        Zed_utf8.extract_next(itemText, itemByte);

      // Got a match
      if (Uchar.equal(lineUchar, byteUchar)) {
        loop(score +. nextBonus, nextLineByte, nextItemByte, nextBonus /. 2.);
      } else {
        loop(score, lineByte, nextItemByte, nextBonus /. 2.);
      };
    };

  loop(0., startLineByte, 0, Constants.firstCharacterScore);
};
