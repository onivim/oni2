open Oni_Core;
open Oni_Core.Types;

module CursorPosition = {
    type t = {
        line: Position.t,
        character: Position.t,
    };
}

module EditorState = {
    type t = {
        activeBuffer: Buffer.t,
        cursorPosition: CursorPosition.t,
    };
}
