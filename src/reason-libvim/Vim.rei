open EditorCoreTypes;

type lineEnding = Types.lineEnding;
module Event = Event;
module Types = Types;

module AutoClosingPairs: {
  module AutoPair: {
    type t = {
      opening: string,
      closing: string,
    };
  };

  type t;

  let empty: t;

  let create:
    (
      ~allowBefore: list(string)=?,
      ~passThrough: list(string)=?,
      ~deletionPairs: list(AutoPair.t)=?,
      list(AutoPair.t)
    ) =>
    t;

  let isBetweenClosingPairs: (string, ByteIndex.t, t) => bool;

  let isBetweenDeletionPairs: (string, ByteIndex.t, t) => bool;
};

module AutoIndent: {
  type action =
    | IncreaseIndent
    | KeepIndent
    | DecreaseIndent;
};

module ColorScheme: {
  module Provider: {
    type t = string => array(string);
    let default: t;
  };
};

module ViewLineMotion: {
  type t =
    | MotionH
    | MotionM
    | MotionL;
};

module Registers: {let get: (~register: char) => option(array(string));};

module Operator: {
  type operation =
    | NoPending
    | Delete
    | Yank
    | Change
    | LeftShift
    | RightShift
    | Filter
    | SwitchCase
    | Indent
    | Format
    | Colon
    | MakeUpperCase
    | MakeLowerCase
    | Join
    | JoinNS
    | Rot13
    | Replace
    | Insert
    | Append
    | Fold
    | FoldOpen
    | FoldOpenRecursive
    | FoldClose
    | FoldCloseRecursive
    | FoldDelete
    | FoldDeleteRecursive
    | Format2
    | Function
    | NumberAdd
    | NumberSubtract
    | Comment;

  type pending = {
    operation,
    register: int,
    count: int,
  };

  let get: unit => option(pending);

  let toString: pending => string;
};

module Mode: {
  type t =
    | Normal({cursor: BytePosition.t})
    | Insert({cursors: list(BytePosition.t)})
    | CommandLine({
        text: string,
        commandCursor: ByteIndex.t,
        commandType: Types.cmdlineType,
        cursor: BytePosition.t,
      })
    | Replace({cursor: BytePosition.t})
    | Visual(VisualRange.t)
    | Operator({
        cursor: BytePosition.t,
        pending: Operator.pending,
      })
    | Select({ranges: list(VisualRange.t)});

  let current: unit => t;

  let isCommandLine: t => bool;
  let isInsert: t => bool;
  let isInsertOrSelect: t => bool;
  let isNormal: t => bool;
  let isVisual: t => bool;
  let isSelect: t => bool;
  let isReplace: t => bool;
  let isOperatorPending: t => bool;

  let cursors: t => list(BytePosition.t);

  let show: t => string;
};

module Split: {
  type t =
    | NewHorizontal
    | Horizontal({filePath: option(string)})
    | NewVertical
    | Vertical({filePath: option(string)})
    | NewTabPage
    | TabPage({filePath: option(string)});
};

module SubMode: {
  type t =
    | None
    | InsertLiteral;
};

module Functions: {
  module GetChar: {
    type mode =
      | Wait // getchar()
      | Immediate // getchar(0)
      | Peek; // getchar(1)

    type t = mode => char;
  };
};

module Context: {
  type t = {
    autoClosingPairs: AutoClosingPairs.t,
    autoIndent:
      (~previousLine: string, ~beforePreviousLine: option(string)) =>
      AutoIndent.action,
    viewLineMotion:
      (~motion: ViewLineMotion.t, ~count: int, ~startLine: LineNumber.t) =>
      LineNumber.t,
    screenCursorMotion:
      (
        ~direction: [ | `Up | `Down],
        ~count: int,
        ~line: LineNumber.t,
        ~currentByte: ByteIndex.t,
        ~wantByte: ByteIndex.t
      ) =>
      BytePosition.t,
    toggleComments: array(string) => array(string),
    bufferId: int,
    colorSchemeProvider: ColorScheme.Provider.t,
    width: int,
    height: int,
    leftColumn: int,
    topLine: int,
    mode: Mode.t,
    subMode: SubMode.t,
    tabSize: int,
    insertSpaces: bool,
    functionGetChar: Functions.GetChar.t,
  };

  let current: unit => t;
};

module CommandLine: {
  type t = Types.cmdline;

  let getCompletions: (~context: Context.t=?, unit) => array(string);

  let getPosition: unit => int;

  let onEnter: (Event.handler(t), unit) => unit;
  let onLeave: (Event.handler(unit), unit) => unit;
  let onUpdate: (Event.handler(t), unit) => unit;
};

module Edit: {
  [@deriving show]
  type t = {
    range: CharacterRange.t,
    text: array(string),
  };

  type editResult = {
    oldStartLine: EditorCoreTypes.LineNumber.t,
    oldEndLine: EditorCoreTypes.LineNumber.t,
    newLines: array(string),
  };

  let applyEdit:
    (~provider: int => option(string), t) => result(editResult, string);

  // [sort(edits)] returns the edits in a list in order to be applied,
  // edits later in the document first (reverse range order).
  let sort: list(t) => list(t);
};

module Buffer: {
  type t = Native.buffer;

  /**
  [openFile(path)] opens a file, sets it as the active buffer, and returns a handle to the buffer.
  */
  let openFile: string => t;

  let make: unit => t;

  /**
  [loadFile(path)] opens a file and returns a handle to the buffer.
  */
  let loadFile: string => t;

  /**
  [getFileName(buffer)] returns the full file path of the buffer [buffer]
  */
  let getFilename: t => option(string);

  /**
  [getFiletype(buffer)] returns the filetype of the buffer [buffer]
  */
  let getFiletype: t => option(string);

  let getLineEndings: t => option(lineEnding);

  let setLineEndings: (t, lineEnding) => unit;

  /**
  [getVersion(buffer)] returns the latest changedtick of the buffer [buffer].

  The changedtick gets updated with each modification to the buffer
  */
  let getVersion: t => int;

  /**
  [isModified(buffer)] returns [true] if the buffer has been modified since the last save, [false] otherwise.
  */
  let isModified: t => bool;

  /**
  [getLineCount(buffer)] returns the number of lines in the buffer.
  */
  let getLineCount: t => int;

  /**
  [getline(buffer, line)] returns the text content at the one-based line number [line] for buffer [buffer].
  */
  let getLine: (t, LineNumber.t) => string;

  let getLines: t => array(string);

  /**
  [getId(buffer)] returns the id of buffer [buffer];
  */
  let getId: t => int;

  /**
  [getById(id)] returns a buffer if one is available with the specified id if it exists, otherwise [None].
  */
  let getById: int => option(t);

  /**
  [getCurrent()] returns the currently active buffer.
  */
  let getCurrent: unit => t;

  let isReadOnly: t => bool;
  let setReadOnly: (~readOnly: bool, t) => unit;

  let isModifiable: t => bool;
  let setModifiable: (~modifiable: bool, t) => unit;

  /**
  [setCurrent(buffer)] sets the active buffer to [buffer].

  This will trigger dispatching of autocommands, like [BufEnter].
  */
  let setCurrent: t => unit;

  /**
  [setLines(~start, ~stop, ~lines, buffer)] sets lines between [start] (inclusive) and [stop] (exclusive).

  - If [start] is not specified, or [None], the lines will be added at the beginning at the before.
  - If [stop] is not specified, or [None], the buffer after [start] will be replaced with [lines]
  - If neither [start] or [stop] are specified, the lines in the buffer will be replaced with [lines]
  */
  let setLines:
    (
      ~undoable: bool=?,
      ~start: LineNumber.t=?,
      ~stop: LineNumber.t=?,
      ~lines: array(string),
      t
    ) =>
    unit;

  let applyEdits: (~edits: list(Edit.t), t) => result(unit, string);

  let onLineEndingsChanged:
    Listeners.bufferLineEndingsChangedListener => Event.unsubscribe;

  /**
  [onModifiedChanged(f)] adds a listener [f] that is called when a buffer's modified
  state _changes._ [f(id, modified)] is called where [id] is the [id] of the affected
  buffer, and [modified] is the new state of the modified flag.
  */
  let onModifiedChanged:
    Listeners.bufferModifiedChangedListener => Event.unsubscribe;

  /**
  [onUpdate(f)] adds a listener [f] that is called whenever a buffer is modified.

  Returns a function that can be called to unsubscribe.
  */
  let onUpdate: Listeners.bufferUpdateListener => Event.unsubscribe;

  /**
  [onFileNameChanged(f)] adds a listener [f] that is called whenever the filename
  associated with a buffer changes. This could happen via a `:sav` command.

  Returns a function that can be called to unsubscribe.
  */
  let onFilenameChanged:
    Listeners.bufferMetadataChangedListener => Event.unsubscribe;
  /**
  [onFiletypeChanged(f)] adds a listener [f] that is called whenever the filetype
  associated with a buffer changes. This could happen via a `:set ft` command.

  Returns a function that can be called to unsubscribe.
  */
  let onFiletypeChanged:
    Listeners.bufferMetadataChangedListener => Event.unsubscribe;

  /**
  [onWrite(f)] adds a listener [f] that is called whenever the buffer is written
  to disk, whether partially or fully. This could happen with `:w`.

  Returns a function that can be called to unsubscribe.
  */
  let onWrite: Listeners.bufferWriteListener => Event.unsubscribe;
};

module Clear: {
  type target =
    | Messages;

  type t = {
    target,
    count: int,
  };
};

module Goto: {
  type effect =
    | Definition
    | Declaration
    | Hover
    | Outline
    | Messages;
};

module TabPage: {
  [@deriving show]
  type effect =
    | Goto(int)
    | GotoRelative(int)
    | Move(int)
    | MoveRelative(int)
    | Close(int)
    | CloseRelative(int)
    | Only(int)
    | OnlyRelative(int);
};

module Format: {
  type formatType =
    | Indentation
    | Formatting;

  type effect =
    | Buffer({
        formatType,
        bufferId: int,
        adjustCursor: bool,
      })
    | Range({
        formatType,
        bufferId: int,
        startLine: LineNumber.t,
        endLine: LineNumber.t,
        adjustCursor: bool,
      });
};

module Setting: {
  [@deriving show]
  type value =
    | String(string)
    | Int(int);

  [@deriving show]
  type t = {
    fullName: string,
    shortName: option(string),
    value,
  };
};

module Scroll: {
  [@deriving show]
  type direction =
    | CursorCenterVertically // zz
    | CursorCenterHorizontally
    | CursorTop // zt
    | CursorBottom // zb
    | CursorLeft
    | CursorRight
    | LineUp
    | LineDown
    | HalfPageDown
    | HalfPageUp
    | PageDown
    | PageUp
    | HalfPageLeft
    | HalfPageRight
    | ColumnLeft
    | ColumnRight;
};

module Mapping: {
  [@deriving show]
  type mode =
    | Insert // imap, inoremap
    | Language // lmap
    | CommandLine // cmap
    | Normal // nmap, nnoremap
    | VisualAndSelect // vmap, vnoremap
    | Visual // xmap, xnoremap
    | Select // smap, snoremap
    | Operator // omap, onoremap
    | Terminal // tmap, tnoremap
    | InsertAndCommandLine // :map!
    | NormalAndVisualAndSelectAndOperator; // :map;

  module ScriptId: {
    [@deriving show]
    type t;
    let default: t;
    let toInt: t => int;
  };

  [@deriving show]
  type t = {
    mode,
    fromKeys: string, // mapped from, lhs
    toValue: string, // mapped to, rhs
    expression: bool,
    recursive: bool,
    silent: bool,
    scriptId: ScriptId.t,
  };
};

module Effect: {
  type t =
    | Goto(Goto.effect)
    | TabPage(TabPage.effect)
    | Format(Format.effect)
    | SettingChanged(Setting.t)
    | ColorSchemeChanged(option(string))
    | MacroRecordingStarted({register: char})
    | MacroRecordingStopped({
        register: char,
        value: option(string),
      })
    | Scroll({
        count: int,
        direction: Scroll.direction,
      })
    | SearchStringChanged(option(string))
    | SearchClearHighlights
    | Map(Mapping.t)
    | Unmap({
        mode: Mapping.mode,
        keys: option(string),
      })
    | Clear(Clear.t)
    | Output({
        cmd: string,
        output: option(string),
      })
    | WindowSplit(Split.t);
};

/**
[init] must be called prior to [input] or [command], and must only be called once.

[init] initializes and sets up initial global state for Vim.
*/
let init: unit => unit;

/**
[input(s)] sends a string of text to Vim

The value [s] must be a string of UTF-8 characters.
- A string of
- A Vim key, ie ["<cr>"] or ["<bs>"]
- A Vim key with modifiers, ie ["<C-a>"]

The keystroke is processed synchronously.
*/
let input: (~context: Context.t=?, string) => (Context.t, list(Effect.t));

/**
[key(s)] sends a single keystroke.

The value [s] must be a valid Vim key, such as:
- A Vim key, ie ["<cr>"] or ["<bs>"]
- A Vim key with modifiers, ie ["<C-a>"]
*/

// TODO: Strongly type these keys...
let key: (~context: Context.t=?, string) => (Context.t, list(Effect.t));

let eval: (~context: Context.t=?, string) => result(string, string);

/**
[command(cmd)] executes [cmd] as an Ex command.

For example, [command("edit! buf.txt")] would run the [edit!] command and open [buf.txt].

You may use any valid Ex command, although you must omit the leading semicolon.

The command [cmd] is processed synchronously.
*/
let command: (~context: Context.t=?, string) => (Context.t, list(Effect.t));

/**
[onDirectoryChanged(f)] registers a directory changed listener [f].

[f] is called whenever the active directory is changed, for example,
via a [command("cd some-new-directory")].
*/
let onDirectoryChanged:
  Listeners.directoryChangedListener => Event.unsubscribe;

/**
[onMessage(f)] registers a message listener [f].
[f] is called whenever a message is emitted from Vim.
*/
let onMessage: Listeners.messageListener => Event.unsubscribe;

/**
[onQuit(f)] registers a quit listener [f].

[f] is called whenever a quit is requested, for example,
by [command(":q")] or [ZZ].
*/
let onQuit: Listeners.quitListener => Event.unsubscribe;

/**
[onTerminal(f)] registers a handler for the :term command.
*/
let onTerminal: (Types.terminalRequest => unit) => Event.unsubscribe;

/**
[onUnhandledEscape(f)] registers an unhandled escape listener [f].

[f] is called whenver an _unhandled escape_ occurs. This happens, for example,
if the user presses <esc> while in normal mode, but there is no pending operator.

The default Vim behavior was to 'beep', but UIs might want to handle this differently.
*/
let onUnhandledEscape: Listeners.noopListener => Event.unsubscribe;

/**
[onVersionCallback(f)] registers a handler when the :version command is used
*/
let onVersion: Listeners.noopListener => Event.unsubscribe;

/**
[onIntroCallback(f)] registers a handler when the :intro command is used
*/
let onIntro: Listeners.noopListener => Event.unsubscribe;

/**
[onYank(f)] registers a yank listener [f]

[f] is called whenever a value is 'yanked' to a register -
this could happen as a result of a yank or a delete command.
*/
let onYank: Listeners.yankListener => Event.unsubscribe;

/**
[onWriteFailure(f)] registers a write failure listener [f]

[f] is called whenever a buffer fails to write to disk.
*/
let onWriteFailure: Listeners.writeFailureListener => Event.unsubscribe;

let onEffect: (Effect.t => unit) => Event.unsubscribe;

module AutoCommands = AutoCommands;
module BufferMetadata = BufferMetadata;
module BufferUpdate = BufferUpdate;
module Clipboard = Clipboard;
module Cursor = Cursor;
module Options = Options;
module Search = Search;
module Visual = Visual;
module VisualRange = VisualRange;
module Window = Window;
module Yank = Yank;

module Testing: {module Undo: {let saveRegion: (int, int) => unit;};};
