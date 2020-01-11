open Oni_Core;

type t = {
  nextId: int,
  idToKeyword: IntMap.t(string),
  keywordToId: StringMap.t(int),
  bufferToLineToWords: IntMap.t(IntMap.t(list(int))),
};

let empty = {
  nextId: 1,
  idToKeyword: IntMap.empty,
  keywordToId: StringMap.empty,
  bufferToLineToWords: IntMap.empty,
};

module Internal = {
  let getOrAddWord = (word: string, keyMap: t) => {
    switch (StringMap.find_opt(word, keyMap.keywordToId)) {
    | Some(id) => (id, keyMap)
    | None =>
      let {nextId, idToKeyword, keywordToId, _}: t = keyMap;
      let id = nextId;
      let nextId = id + 1;
      let idToKeyword = IntMap.add(id, word, idToKeyword);
      let keywordToId = StringMap.add(word, id, keywordToId);
      (id, {...keyMap, nextId, idToKeyword, keywordToId});
    };
  };

  let update = (~bufferId, ~line, wordIds: list(int), keyMap) => {
    let updateLine =
      IntMap.update(
        line,
        fun
        | None => Some(wordIds)
        | Some(_) => Some(wordIds),
      );

    IntMap.update(
      bufferId,
      fun
      | None => Some(updateLine(IntMap.empty))
      | Some(v) => Some(updateLine(v)),
      keyMap.bufferToLineToWords,
    );
  };
};

let add = (~bufferId, ~line: int, ~words: list(string), keyMap) => {
  // Update all the words
  let (ids, keyMap) =
    List.fold_left(
      (acc, curr) => {
        let (currentWords, currentMap) = acc;
        let (wordId, newKeyMap) = Internal.getOrAddWord(curr, currentMap);
        ([wordId, ...currentWords], newKeyMap);
      },
      ([], keyMap),
      words,
    );

  keyMap;
};

let setBufferScope = (~bufferId, ~scope, keyMap) => keyMap;

let getKeywords = (~scope, keyMap) => ["KeyMapHello"];
