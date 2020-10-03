
open EditorCoreTypes;
open Oni_Core;

// MODEL

type model;

let empty: model;

let add: (Buffer.t, model) => model;

let get: (int, model) => option(Buffer.t);

let modified: (model) => list(Buffer.t);

let anyModified: model => bool;

let isModifiedByPath: (model, string) => bool;

let all: (model) => list(Buffer.t);

let map: (Buffer.t => Buffer.t, model) => model;

let filter: (Buffer.t => bool, model) => list(Buffer.t);

// MSG

[@deriving show]
type msg =
  | SyntaxHighlightingDisabled(int)
  | Entered({
      id: int,
      fileType: Oni_Core.Buffer.FileType.t,
      lineEndings: [@opaque] option(Vim.lineEnding),
      filePath: option(string),
      isModified: bool,
      version: int,
      font: Font.t,
      // TODO: This duplication-of-truth is really awkward,
      // but I want to remove it shortly
      buffer: [@opaque] Buffer.t,
    })
  | FileTypeChanged({
      id: int,
      fileType: Oni_Core.Buffer.FileType.t,
    })
  | FilenameChanged({
      id: int,
      newFilePath: option(string),
      newFileType: Oni_Core.Buffer.FileType.t,
      version: int,
      isModified: bool,
    })
  | Update({
      update: [@opaque] BufferUpdate.t,
      oldBuffer: [@opaque] Buffer.t,
      newBuffer: [@opaque] Buffer.t,
      triggerKey: option(string),
    })
  | LineEndingsChanged({
      id: int,
      lineEndings: [@opaque] Vim.lineEnding,
    })
  | Saved(int)
  | IndentationSet(int, [@opaque] IndentationSettings.t)
  | ModifiedSet(int, bool);

type outmsg;

// UPDATE

let update: (msg, model) => model;

// EFFECTS

module Effects: {
    let openInEditor: (
        ~split: ([`Current | `Horizontal | `Vertical | `NewTab])=?,
        ~position: option(CharacterPosition.t)=?,
        ~grabFocus: bool=?,
        string
    ) => Isolinear.Effect.t(msg);

    let load: (
        string
    ) => Isolinear.Effect.t(msg);
}
