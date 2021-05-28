open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type model = {
  providers: list(provider),
  // buffer Id -> lines -> ranges
  bufferToHighlights: IntMap.t(IntMap.t(list(CharacterRange.t))),
};

let initial = {providers: [], bufferToHighlights: IntMap.empty};

[@deriving show]
type command =
  | ChangeAll;

[@deriving show]
type msg =
  | Command(command)
  | DocumentHighlighted({
      bufferId: int,
      ranges: list(CharacterRange.t),
    });
// TODO: kind?

module Commands = {
  open Feature_Commands.Schema;

  let changeAll =
    define(
      ~category="Editor",
      ~title="Change All Occurrences",
      "editor.action.changeAll",
      Command(ChangeAll),
    );
};

module Keybindings = {
  open Feature_Input.Schema;

  let changeAllWindows =
    bind(
      ~key="<S-F2>",
      ~command=Commands.changeAll.id,
      ~condition="!isMac && editorTextFocus" |> WhenExpr.parse,
    );

  let changeAllMac =
    bind(
      ~key="<D-F2>",
      ~command=Commands.changeAll.id,
      ~condition="isMac && editorTextFocus" |> WhenExpr.parse,
    );
};

let clear = (~bufferId, model) => {
  {
    ...model,
    bufferToHighlights:
      IntMap.add(bufferId, IntMap.empty, model.bufferToHighlights),
  };
};

let allHighlights = (~bufferId, model) => {
  model.bufferToHighlights
  |> IntMap.find_opt(bufferId)
  |> Option.value(~default=IntMap.empty)
  |> IntMap.bindings
  |> List.map(snd)
  |> List.flatten;
};

let cursorMoved = (~buffer, ~cursor, model) => {
  let bufferId = Oni_Core.Buffer.getId(buffer);
  let isCursorInHighlight =
    allHighlights(~bufferId, model)
    |> List.exists(range => CharacterRange.contains(cursor, range));

  if (!isCursorInHighlight) {
    clear(~bufferId, model);
  } else {
    model;
  };
};

let moveMarkers = (~buffer, ~markerUpdate, model) => {
  let bufferId = Oni_Core.Buffer.getId(buffer);

  let shiftLines = (~afterLine, ~delta, bufferToHighlights) => {
    let line = afterLine |> EditorCoreTypes.LineNumber.toZeroBased;
    bufferToHighlights
    |> IntMap.update(
         bufferId,
         Option.map(lineMap => {
           IntMap.shift(~startPos=line, ~endPos=line, ~delta, lineMap)
         }),
       );
  };

  let clearLine = (~line, bufferToHighlights) => {
    let lineIdx = line |> EditorCoreTypes.LineNumber.toZeroBased;
    bufferToHighlights
    |> IntMap.update(
         bufferId,
         Option.map(lineMap => {IntMap.remove(lineIdx, lineMap)}),
       );
  };

  let shiftCharacters =
      (
        ~line,
        ~afterByte as _,
        ~deltaBytes as _,
        ~afterCharacter,
        ~deltaCharacters,
        bufferToHighlights,
      ) => {
    bufferToHighlights
    |> IntMap.update(
         bufferId,
         Option.map(lineMap => {
           lineMap
           |> IntMap.update(
                EditorCoreTypes.LineNumber.toZeroBased(line),
                Option.map(ranges => {
                  ranges
                  |> List.map(
                       CharacterRange.shiftCharacters(
                         ~line,
                         ~afterCharacter,
                         ~delta=deltaCharacters,
                       ),
                     )
                }),
              )
         }),
       );
  };

  let bufferToHighlights' =
    MarkerUpdate.apply(
      ~clearLine,
      ~shiftLines,
      ~shiftCharacters,
      markerUpdate,
      model.bufferToHighlights,
    );
  {...model, bufferToHighlights: bufferToHighlights'};
};

let update = (~maybeBuffer, ~editorId, msg, model) => {
  switch (msg) {
  | DocumentHighlighted({bufferId, ranges}) =>
    let lineMap = ranges |> Utility.RangeEx.toCharacterLineMap;
    let bufferToHighlights =
      model.bufferToHighlights |> IntMap.add(bufferId, lineMap);
    ({...model, bufferToHighlights}, Outmsg.Nothing);

  | Command(ChangeAll) =>
    maybeBuffer
    |> Option.map(Oni_Core.Buffer.getId)
    |> Option.map(bufferId => {
         // Fix 'impedance mismatch' - the highlights return the last character as an 'exclusive' character,
         // but the selection treats the last character as inclusive.
         let allHighlights =
           allHighlights(~bufferId, model)
           |> List.map((range: CharacterRange.t) =>
                CharacterRange.{
                  start: {
                    line: range.start.line,
                    character: range.start.character,
                  },
                  stop: {
                    line: range.stop.line,
                    character: CharacterIndex.(range.stop.character - 1),
                  },
                }
              );
         (model, Outmsg.SetSelections({editorId, ranges: allHighlights}));
       })
    |> Option.value(~default=(model, Outmsg.Nothing))
  };
};

let register = (~handle: int, ~selector, model) => {
  ...model,
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle: int, model) => {
  ...model,
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

let getByLine = (~bufferId, ~line, model) => {
  model.bufferToHighlights
  |> IntMap.find_opt(bufferId)
  |> OptionEx.flatMap(IntMap.find_opt(line))
  |> Option.value(~default=[]);
};

let getLinesWithHighlight = (~bufferId, model) => {
  model.bufferToHighlights
  |> IntMap.find_opt(bufferId)
  |> Option.map(lineMap => IntMap.bindings(lineMap) |> List.map(fst))
  |> Option.value(~default=[]);
};

module Configuration = {
  open Config.Schema;
  let enabled = setting("editor.occurrencesHighlight", bool, ~default=true);
};

let configurationChanged = (~config, model) =>
  if (!Configuration.enabled.get(config)) {
    {...model, bufferToHighlights: IntMap.empty};
  } else {
    model;
  };

let sub = (~isInsertMode, ~config, ~buffer, ~location, ~client, model) =>
  if (!Configuration.enabled.get(config) || isInsertMode) {
    Isolinear.Sub.none;
  } else {
    let toMsg = (highlights: list(Exthost.DocumentHighlight.t)) => {
      let ranges =
        highlights
        |> List.map(({range, _}: Exthost.DocumentHighlight.t) => {
             Exthost.OneBasedRange.toRange(range)
           });

      DocumentHighlighted({bufferId: Oni_Core.Buffer.getId(buffer), ranges});
    };

    model.providers
    |> List.filter(({selector, _}) =>
         selector |> Exthost.DocumentSelector.matchesBuffer(~buffer)
       )
    |> List.map(({handle, _}) => {
         Service_Exthost.Sub.documentHighlights(
           ~handle,
           ~buffer,
           ~position=location,
           ~toMsg,
           client,
         )
       })
    |> Isolinear.Sub.batch;
  };

module Contributions = {
  let configuration = Configuration.[enabled.spec];

  let commands = Commands.[changeAll];

  let keybindings = Keybindings.[changeAllWindows, changeAllMac];
};
