open EditorCoreTypes;

type lineEnding = Types.lineEnding;

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

  let isBetweenClosingPairs: (string, Index.t, t) => bool;

  let isBetweenDeletionPairs: (string, Index.t, t) => bool;
};

module Context: {
  type t = {
    autoClosingPairs: AutoClosingPairs.t,
    bufferId: int,
    width: int,
    height: int,
    leftColumn: int,
    topLine: int,
    cursors: list(Cursor.t),
    lineComment: option(string),
    tabSize: int,
    insertSpaces: bool,
  };

  let current: unit => t;
};

module Edit: {
  [@deriving show]
  type t = {
    range: Range.t,
    text: option(string),
  };

  type editResult = {
    oldStartLine: Index.t,
    oldEndLine: Index.t,
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
  let getLine: (t, Index.t) => string;

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
    (~start: Index.t=?, ~stop: Index.t=?, ~lines: array(string), t) => unit;

  let applyEdits: (~edits: list(Edit.t), t) => result(unit, string);

  /**
  [onEnter(f)] adds a listener [f] that is called whenever a new buffer is entered.

  This is more reliable than autocommands, as it will dispatch in any case the buffer
  is changed, even in cases where [BufEnter] would not be dispatched.

  Returns a function that can be called to unsubscribe.
  */
  let onEnter: Listeners.bufferListener => Event.unsubscribe;

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

module Goto: {
  type t =
    | Definition
    | Declaration
    | Hover;
};

module Effect: {
  type t =
    | Goto(Goto.t);
};

/**
[init] must be called prior to [input] or [command], and must only be called once.

[init] initializes and sets up initial global state for Vim.
*/
let init: unit => unit;

/**
[input(s)] sends a single keystroke to Vim.

The value [s] may be of the following form:
- A single ASCII character, ie ["a"] or [":"]
- A Vim key, ie ["<cr>"] or ["<bs>"]
- A Vim key with modifiers, ie ["<C-a>"]

The keystroke is processed synchronously.
*/
let input: (~context: Context.t=?, string) => Context.t;

/**
[command(cmd)] executes [cmd] as an Ex command.

For example, [command("edit! buf.txt")] would run the [edit!] command and open [buf.txt].

You may use any valid Ex command, although you must omit the leading semicolon.

The command [cmd] is processed synchronously.
*/
let command: string => Context.t;

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
module CommandLine = CommandLine;
module Cursor = Cursor;
module Event = Event;
module Mode = Mode;
module Options = Options;
module Search = Search;
module Types = Types;
module Visual = Visual;
module VisualRange = VisualRange;
module Window = Window;
module Yank = Yank;
