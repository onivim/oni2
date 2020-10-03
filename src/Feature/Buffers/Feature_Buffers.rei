open EditorCoreTypes;
open Oni_Core;

// MODEL

type model;

let empty: model;

let add: (Buffer.t, model) => model;

let get: (int, model) => option(Buffer.t);

let modified: model => list(Buffer.t);

let anyModified: model => bool;

let isModifiedByPath: (model, string) => bool;

let all: model => list(Buffer.t);

let map: (Buffer.t => Buffer.t, model) => model;

let filter: (Buffer.t => bool, model) => list(Buffer.t);

// MSG

[@deriving show]
type msg;

module Msg: {
  let saved: (~bufferId: int) => msg;
  let modified: (~bufferId: int, ~isModified: bool) => msg;
  let fileNameChanged:
    (
      ~bufferId: int,
      ~newFilePath: option(string),
      ~newFileType: Oni_Core.Buffer.FileType.t,
      ~version: int,
      ~isModified: bool
    ) =>
    msg;
  let fileTypeChanged:
    (~bufferId: int, ~fileType: Oni_Core.Buffer.FileType.t) => msg;
  let lineEndingsChanged:
    (~bufferId: int, ~lineEndings: Vim.Types.lineEnding) => msg;

  let indentationSet:
    (~bufferId: int, ~indentation: IndentationSettings.t) => msg;

  let updated:
    (
      ~update: BufferUpdate.t,
      ~newBuffer: Buffer.t,
      ~oldBuffer: Buffer.t,
      ~triggerKey: option(string)
    ) =>
    msg;
};

type outmsg =
  | Nothing
  | BufferUpdated({
      update: Oni_Core.BufferUpdate.t,
      newBuffer: Oni_Core.Buffer.t,
      oldBuffer: Oni_Core.Buffer.t,
      triggerKey: option(string),
    })
  //| BufferSaved()
  | CreateEditor({
      buffer: Oni_Core.Buffer.t,
      split: [ | `Current | `Horizontal | `Vertical | `NewTab],
      position: option(BytePosition.t),
      grabFocus: bool,
    });

// UPDATE

let update: (msg, model) => (model, outmsg);

// EFFECTS

module Effects: {
  let openInEditor:
    (
      ~font: Service_Font.font,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~split: [ | `Current | `Horizontal | `Vertical | `NewTab]=?,
      ~position: option(CharacterPosition.t)=?,
      ~grabFocus: bool=?,
      ~filePath: string,
      model
    ) =>
    Isolinear.Effect.t(msg);
};
