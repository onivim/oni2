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

  let changeAll =
    bind(
      ~key="<F2>",
      ~command=Commands.changeAll.id,
      ~condition="editorTextFocus" |> WhenExpr.parse,
    );
};

let clear = (~bufferId, model) => {
  {
    ...model,
    bufferToHighlights:
      IntMap.add(bufferId, IntMap.empty, model.bufferToHighlights),
  };
};

let cursorMoved = (~buffer, ~cursor, model) => {
  let isCursorInHighlight = (highlights: IntMap.t(list(CharacterRange.t))) => {
    highlights
    |> IntMap.bindings
    |> List.map(snd)
    |> List.flatten
    |> List.exists(range => CharacterRange.contains(cursor, range));
  };

  let bufferId = Oni_Core.Buffer.getId(buffer);
  let currentHighlights =
    model.bufferToHighlights
    |> IntMap.find_opt(bufferId)
    |> Option.value(~default=IntMap.empty);

  if (!isCursorInHighlight(currentHighlights)) {
    clear(~bufferId, model);
  } else {
    model;
  };
};

let allHighlights = (~bufferId, model) => {
  [];
};

let update = (~maybeBuffer, ~editorId, msg, model) => {
  switch (msg) {
  | DocumentHighlighted({bufferId, ranges}) =>
    let lineMap = ranges |> Utility.RangeEx.toCharacterLineMap;
    let bufferToHighlights =
      model.bufferToHighlights |> IntMap.add(bufferId, lineMap);
    ({...model, bufferToHighlights}, Outmsg.Nothing);

  | Command(ChangeAll) =>
    let _model =
      maybeBuffer
      |> Option.map(Oni_Core.Buffer.getId)
      |> Option.map(bufferId => {
           let _allHighlights = allHighlights(~bufferId, model);
           ();
         });
    open EditorCoreTypes;
    let rangeForLine = idx =>
      CharacterRange.{
        start:
          CharacterPosition.{
            line: LineNumber.ofZeroBased(idx),
            character: CharacterIndex.zero,
          },
        stop:
          BytePosition.{
            line: LineNumber.ofZeroBased(idx),
            character: CharacterIndex.(zero + 1),
          },
      };

    (
      model,
      Outmsg.SetSelections({
        editorId,
        ranges:
          CharacterRange.[
            rangeForLine(0),
            rangeForLine(1),
            rangeForLine(2),
          ],
      }),
    );
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

  let keybindings = Keybindings.[changeAll];
};
