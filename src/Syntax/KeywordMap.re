open Oni_Core;

module KeywordCount = {
  type t = IntMap.t(int);

  type diff = IntMap.t(int);

  let empty = IntMap.empty;

  let ofIds = (ids: list(int)) => {
    List.fold_left(
      (acc, curr) => {
        IntMap.update(
          curr,
          fun
          | Some(v) => Some(v + 1)
          | None => Some(1),
          acc,
        )
      },
      IntMap.empty,
      ids,
    );
  };

  let getIds = (map: t) => {
    map
    |> IntMap.bindings
    |> List.filter(((_, count)) => count > 0)
    |> List.map(((word, _)) => word);
  };

  let createDiff: (t, t) => diff =
    (oldMap, newMap) => {
      IntMap.fold(
        (key, v, acc) => {
          IntMap.update(
            key,
            fun
            | None => Some((-1) * v)
            | Some(newCount) => Some(newCount - v),
            acc,
          )
        },
        newMap,
        oldMap,
      );
    };

  let applyDiff: (diff, t) => t =
    (diff, map) => {
      IntMap.fold(
        (key, diff, acc) => {
          IntMap.update(
            key,
            fun
            | None => Some(diff)
            | Some(oldVal) => Some(oldVal + diff),
            acc,
          )
        },
        map,
        diff,
      );
    };
};

type t = {
  nextId: int,
  idToKeyword: IntMap.t(string),
  keywordToId: StringMap.t(int),
  bufferToLineToWords: IntMap.t(IntMap.t(KeywordCount.t)),
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
    |> List.map(id => IntMap.find_opt(id, map))
    |> Utility.List.filter_map(Utility.identity);
  };

  let getKeyCount = (~bufferId, ~line, keyMap: t) => {
    keyMap.bufferToLineToWords
    |> IntMap.find_opt(bufferId)
    |> Utility.Option.bind(IntMap.find_opt(line))
    |> Utility.Option.value(~default=KeywordCount.empty);
  };

  let update =
      (
        ~bufferId,
        ~line,
        ~scope,
        ~newKeywords: KeywordCount.t,
        ~diff: KeywordCount.diff,
        keyMap,
      ) => {
    // First, save new keywords

    let updateLine = (line, prev) => {
      IntMap.add(line, newKeywords, prev);
    };

    let updateBuffer = (bufferId, prev) => {
      IntMap.update(
        bufferId,
        fun
        | None => Some(updateLine(line, IntMap.empty))
        | Some(v) => Some(updateLine(line, v)),
        prev,
      );
    };

    let bufferToLineToWords =
      updateBuffer(bufferId, keyMap.bufferToLineToWords);

    // Apply diff to fileTypeToCount
    let fileTypeToCount =
      StringMap.update(
        scope,
        fun
        | None => Some(newKeywords)
        | Some(old) => Some(KeywordCount.applyDiff(diff, old)),
        keyMap.fileTypeToCount,
      );

    {...keyMap, bufferToLineToWords, fileTypeToCount};
  };
};

let set = (~bufferId, ~scope, ~line: int, ~words: list(string), keyMap) => {
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

  let newKeyCount = KeywordCount.ofIds(ids);

  let oldKeyCount = Internal.getKeyCount(~bufferId, ~line, keyMap);

  let diff = KeywordCount.createDiff(oldKeyCount, newKeyCount);

  Internal.update(
    ~bufferId,
    ~scope,
    ~line,
    ~newKeywords=newKeyCount,
    ~diff,
    keyMap,
  );
};

let get = (~scope, keyMap) => {
  keyMap.fileTypeToCount
  |> StringMap.find_opt(scope)
  |> Utility.Option.map(KeywordCount.getIds)
  |> Utility.Option.map(Internal.idsToWords(~map=keyMap.idToKeyword))
  |> Utility.Option.value(~default=[]);
};
