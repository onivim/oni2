open Oni_Core;

module CursorPosition = {
    type t = {
        line: Position.t,
        character: Position.t,
    };
}

type EditorState = {
    activeBuffer: Buffer.t,
    cursorPosition: CursorPosition.t,
};
