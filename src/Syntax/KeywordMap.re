open Oni_Core;

module KeywordCount = {
  type t = IntMap.t(int);

  //let empty = IntMap.empty;

  let ofIds = (ids: list(int)) => {
    List.fold_left((acc, curr) => {
      IntMap.update(curr,
      fun
      | Some(v) => Some(v + 1)
      | None => Some(1),
      acc);
    }, IntMap.empty, ids);
  };

  let getIds = (map: t) => {
    map
    |> IntMap.bindings
    |> List.filter(((_, count)) => count > 0)
    |> List.map(((word, _)) => word);
  };
};

type t = {
  nextId: int,
  idToKeyword: IntMap.t(string),
  keywordToId: StringMap.t(int),
  bufferToLineToWords: IntMap.t(IntMap.t(list(int))),
  fileTypeToCount: StringMap.t(KeywordCount.t),
};

let empty = {
  nextId: 1,
  idToKeyword: IntMap.empty,
  keywordToId: StringMap.empty,
  bufferToLineToWords: IntMap.empty,
  fileTypeToCount: StringMap.empty,
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

  let idsToWords = (~map: IntMap.t(string), ids: list(int)) => {
    ids
    |> List.map((id) => IntMap.find_opt(id, map))
    |> Utility.List.filter_map(Utility.identity);
  };

  let _update = (~bufferId, ~line, wordIds: list(int), keyMap) => {
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

  /*let setBufferScope = (
    ~bufferId, 
    ~scope, 
    keyMap) => 
    keyMap;*/

};

let set = (~bufferId, ~scope, ~line: int, ~words: list(string), keyMap) => {

  ignore(bufferId);
  ignore(scope);
  ignore(line);
  ignore(words);
  // TODO:
  //let keyMap = Internal.setBufferScope(~bufferId, ~scope, keyMap);

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

  // Naively set words for buffer
  let fileTypeToCount = StringMap.add(scope, 
  KeywordCount.ofIds(ids),
  keyMap.fileTypeToCount);

  { ...keyMap, fileTypeToCount };
};

let get = (~scope, keyMap) =>  {
  keyMap.fileTypeToCount
  |> StringMap.find_opt(scope)
  |> Utility.Option.map(KeywordCount.getIds)
  |> Utility.Option.map(Internal.idsToWords(~map=keyMap.idToKeyword))
  |> Utility.Option.value(~default=[]);
 }
