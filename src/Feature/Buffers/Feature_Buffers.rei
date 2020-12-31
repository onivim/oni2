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
  | BufferSaved(Oni_Core.Buffer.t)
  | CreateEditor({
      buffer: Oni_Core.Buffer.t,
      split: [ | `Current | `Horizontal | `Vertical | `NewTab],
      position: option(BytePosition.t),
      grabFocus: bool,
      preview: bool,
    })
  | BufferModifiedSet(int, bool);

// UPDATE

let update:
  (~activeBufferId: int, ~config: Config.fileTypeResolver, msg, model) =>
  (model, outmsg);

// EFFECTS

module Effects: {
  let loadFile:
    (
      ~font: Service_Font.font,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~filePath: string,
      ~toMsg: array(string) => 'msg,
      model
    ) =>
    Isolinear.Effect.t('msg);

  let openNewBuffer:
    (
      ~font: Service_Font.font,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~split: [ | `Current | `Horizontal | `Vertical | `NewTab],
      model
    ) =>
    Isolinear.Effect.t(msg);

  let openFileInEditor:
    (
      ~font: Service_Font.font,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~split: [ | `Current | `Horizontal | `Vertical | `NewTab]=?,
      ~position: option(CharacterPosition.t)=?,
      ~grabFocus: bool=?,
      ~filePath: string,
      ~preview: bool=?,
      model
    ) =>
    Isolinear.Effect.t(msg);

  let openBufferInEditor:
    (
      ~font: Service_Font.font,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~split: [ | `Current | `Horizontal | `Vertical | `NewTab],
      ~bufferId: int,
      model
    ) =>
    Isolinear.Effect.t(msg);
};

module Contributions: {
  let commands: Command.Lookup.t(msg);
  let configuration: list(Config.Schema.spec);
};
