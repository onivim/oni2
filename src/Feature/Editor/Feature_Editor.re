open EditorCoreTypes;

module BracketMatch = BracketMatch;
module BufferLineColorizer = BufferLineColorizer;
module BufferViewTokenizer = BufferViewTokenizer;
module Selection = Selection;

module Editor = Editor;
module EditorBuffer = EditorBuffer;
module EditorId = EditorId;
module EditorLayout = EditorLayout;
module EditorSurface = EditorSurface;

module EditorDiffMarkers = EditorDiffMarkers;

module Wrapping = Wrapping;

module Configuration = EditorConfiguration;

module Contributions = {
  let configuration = EditorConfiguration.contributions;
};

open Msg;

[@deriving show({with_path: false})]
type msg = Msg.t;

type outmsg =
  | Nothing
  | MouseHovered(option(CharacterPosition.t))
  | MouseMoved(option(CharacterPosition.t));

type model = Editor.t;

module Constants = {
  let editorWheelMultiplier = 50.;
  let minimapWheelMultiplier = 150.;
  let scrollbarWheelMultiplier = 300.;
};

let update = (editor, msg) => {
  switch (msg) {
  | VerticalScrollbarAfterTrackClicked({newPixelScrollY})
  | VerticalScrollbarBeforeTrackClicked({newPixelScrollY})
  | VerticalScrollbarMouseDrag({newPixelScrollY}) => (
      Editor.scrollToPixelY(~animated=false, ~pixelY=newPixelScrollY, editor),
      Nothing,
    )
  | MinimapMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelY(
        ~animated=false,
        ~pixelY=deltaWheel *. Constants.minimapWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | MinimapClicked({viewLine}) => (
      Editor.scrollToLine(~line=viewLine, editor),
      Nothing,
    )
  | MinimapDragged({newPixelScrollY}) => (
      Editor.scrollToPixelY(~animated=false, ~pixelY=newPixelScrollY, editor),
      Nothing,
    )
  | EditorMouseWheel({deltaX, deltaY, shiftKey}) => (
      Editor.scrollDeltaPixelXY(
        ~animated=false,
        ~pixelX=
          (shiftKey ? deltaY : deltaX) *. Constants.editorWheelMultiplier,
        ~pixelY=(shiftKey ? 0. : deltaY) *. Constants.editorWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | VerticalScrollbarMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelY(
        ~animated=false,
        ~pixelY=deltaWheel *. Constants.scrollbarWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | HorizontalScrollbarBeforeTrackClicked({newPixelScrollX})
  | HorizontalScrollbarAfterTrackClicked({newPixelScrollX})
  | HorizontalScrollbarMouseDrag({newPixelScrollX}) => (
      Editor.scrollToPixelX(~animated=false, ~pixelX=newPixelScrollX, editor),
      Nothing,
    )
  | HorizontalScrollbarMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelX(
        ~animated=false,
        ~pixelX=deltaWheel *. Constants.scrollbarWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | HorizontalScrollbarMouseDown
  | HorizontalScrollbarMouseRelease
  | VerticalScrollbarMouseRelease
  | VerticalScrollbarMouseDown => (editor, Nothing)
  | EditorMouseDown({time, pixelX, pixelY}) => (
      editor |> Editor.mouseDown(~time, ~pixelX, ~pixelY),
      Nothing,
    )
  | EditorMouseUp({time, pixelX, pixelY}) => (
      editor |> Editor.mouseUp(~time, ~pixelX, ~pixelY),
      Nothing,
    )
  | InlineElementSizeChanged({key, uniqueId, height}) => (
      Editor.setInlineElementSize(~key, ~uniqueId, ~height, editor),
      Nothing,
    )
  | Internal(msg) => (Editor.update(msg, editor), Nothing)
  | EditorMouseMoved({time, pixelX, pixelY}) =>
    let editor' = editor |> Editor.mouseMove(~time, ~pixelX, ~pixelY);

    let maybeCharacter = Editor.getCharacterUnderMouse(editor');
    let eff = MouseMoved(maybeCharacter);
    (editor', eff);
  | EditorMouseLeave => (editor |> Editor.mouseLeave, Nothing)
  | EditorMouseEnter => (editor |> Editor.mouseEnter, Nothing)
  | MouseHovered =>
    let maybeCharacter = Editor.getCharacterUnderMouse(editor);
    (editor, MouseHovered(maybeCharacter));
  //  | MouseMoved({bytePosition}) => (
  //      editor,
  //      {
  //        Editor.byteToCharacter(bytePosition, editor)
  //        |> Option.map(characterPosition => {
  //             MouseMoved({bytePosition, characterPosition})
  //           })
  //        |> Option.value(~default=Nothing);
  //      },
  //    )
  | ModeChanged({mode, effects}) =>
    let handleScrollEffect = (~count, ~direction, editor) => {
      let count = max(count, 1);
      Vim.Scroll.(
        switch (direction) {
        | CursorCenterVertically =>
          Editor.scrollCenterCursorVertically(editor)
        | CursorTop => Editor.scrollCursorTop(editor)
        | CursorBottom => Editor.scrollCursorBottom(editor)
        | LineUp => Editor.scrollLines(~count=count * (-1), editor)
        | LineDown => Editor.scrollLines(~count, editor)
        | HalfPageUp => Editor.scrollHalfPage(~count=count * (-1), editor)
        | HalfPageDown => Editor.scrollHalfPage(~count, editor)
        | PageUp => Editor.scrollPage(~count=count * (-1), editor)
        | PageDown => Editor.scrollPage(~count, editor)
        | _ => editor
        }
      );
    };

    let editor' = Editor.setMode(mode, editor);
    let editor'' =
      effects
      |> List.fold_left(
           (acc, effect) => {
             switch (effect) {
             | Vim.Effect.Scroll({count, direction}) =>
               handleScrollEffect(~count, ~direction, acc)
             | _ => acc
             }
           },
           editor',
         );
    (editor'', Nothing);
  };
};

module Sub = {
  let editor = (~config, editor: Editor.t) => {
    let hoverEnabled = EditorConfiguration.Hover.enabled.get(config);
    let hoverSub =
      switch (Editor.lastMouseMoveTime(editor)) {
      | Some(time) when hoverEnabled && !Editor.isMouseDown(editor) =>
        let delay = EditorConfiguration.Hover.delay.get(config);
        Service_Time.Sub.once(
          ~uniqueId={
            string_of_int(Editor.getId(editor))
            ++ string_of_float(Revery.Time.toFloatSeconds(time));
          },
          ~delay,
          ~msg=(~current as _) =>
          Msg.MouseHovered
        );
      | Some(_) => Isolinear.Sub.none
      | None => Isolinear.Sub.none
      };

    let internalSub =
      Editor.sub(editor) |> Isolinear.Sub.map(msg => Msg.Internal(msg));
    Isolinear.Sub.batch([hoverSub, internalSub]);
  };
};
