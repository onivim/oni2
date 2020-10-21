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
  | MouseHovered({
      bytePosition: BytePosition.t,
      characterPosition: CharacterPosition.t,
    })
  | MouseMoved({
      bytePosition: BytePosition.t,
      characterPosition: CharacterPosition.t,
    });

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
      Editor.scrollToPixelY(~pixelY=newPixelScrollY, editor),
      Nothing,
    )
  | MinimapMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelY(
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
      Editor.scrollToPixelY(~pixelY=newPixelScrollY, editor),
      Nothing,
    )
  | EditorMouseWheel({deltaX, deltaY, shiftKey}) => (
      Editor.scrollDeltaPixelXY(
        ~pixelX=
          (shiftKey ? deltaY : deltaX) *. Constants.editorWheelMultiplier,
        ~pixelY=(shiftKey ? 0. : deltaY) *. Constants.editorWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | VerticalScrollbarMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelY(
        ~pixelY=deltaWheel *. Constants.scrollbarWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | HorizontalScrollbarBeforeTrackClicked({newPixelScrollX})
  | HorizontalScrollbarAfterTrackClicked({newPixelScrollX})
  | HorizontalScrollbarMouseDrag({newPixelScrollX}) => (
      Editor.scrollToPixelX(~pixelX=newPixelScrollX, editor),
      Nothing,
    )
  | HorizontalScrollbarMouseWheel({deltaWheel}) => (
      Editor.scrollDeltaPixelX(
        ~pixelX=deltaWheel *. Constants.scrollbarWheelMultiplier,
        editor,
      ),
      Nothing,
    )
  | HorizontalScrollbarMouseDown
  | HorizontalScrollbarMouseRelease
  | VerticalScrollbarMouseRelease
  | VerticalScrollbarMouseDown => (editor, Nothing)
  | MouseHovered({bytePosition}) => (
      editor,
      {
        Editor.byteToCharacter(bytePosition, editor)
        |> Option.map(characterPosition => {
             MouseHovered({bytePosition, characterPosition})
           })
        |> Option.value(~default=Nothing);
      },
    )
  | MouseMoved({bytePosition}) => (
      editor,
      {
        Editor.byteToCharacter(bytePosition, editor)
        |> Option.map(characterPosition => {
             MouseMoved({bytePosition, characterPosition})
           })
        |> Option.value(~default=Nothing);
      },
    )
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
  | MinimapEnabledConfigChanged(enabled) => (
      Editor.setMinimapEnabled(~enabled, editor),
      Nothing,
    )
  | LineHeightConfigChanged(lineHeight) => (
      Editor.setLineHeight(~lineHeight, editor),
      Nothing,
    )
  };
};

module Sub = {
  module MinimapEnabledSub =
    Oni_Core.Config.Sub.Make({
      type configValue = bool;
      let schema = EditorConfiguration.Minimap.enabled;
      type msg = Msg.t;
    });
  module LineHeightSub =
    Oni_Core.Config.Sub.Make({
      type configValue = LineHeight.t;
      let schema = EditorConfiguration.lineHeight;
      type msg = Msg.t;
    });
  let global = (~config) => {
    let minimapEnabledConfig =
      MinimapEnabledSub.create(
        ~config, ~name="Feature_Editor.Config.minimapEnabled", ~toMsg=enabled =>
        MinimapEnabledConfigChanged(enabled)
      );

    let lineHeightConfig =
      LineHeightSub.create(
        ~config,
        ~name="Feature_Editor.Config.lineHeightEnabled",
        ~toMsg=lineHeight =>
        LineHeightConfigChanged(lineHeight)
      );

    [minimapEnabledConfig, lineHeightConfig] |> Isolinear.Sub.batch;
  };
};
