/*
 * EditorVerticalScrollbar.re
 */

open EditorCoreTypes;
open Revery.UI;

open Oni_Core;

module BufferHighlights = Oni_Syntax.BufferHighlights;
module Diagnostic = Feature_LanguageSupport.Diagnostic;

module Styles = {
  open Style;

  let absolute = [
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
  ];

  let background = color => [backgroundColor(color), ...absolute];

  let container = (~color) => {
    [
      backgroundColor(color),
      cursor(Revery.MouseCursors.pointer),
      ...absolute,
    ];
  };

  let verticalThumb =
      (~width, ~scrollMetrics: Editor.scrollbarMetrics, ~color) => [
    position(`Absolute),
    top(scrollMetrics.thumbOffset),
    left(0),
    Style.width(width),
    height(scrollMetrics.thumbSize),
    backgroundColor(color),
  ];

  let cursor = (~cursorLine, ~totalWidth, ~colors: Colors.t) => {
    [
      position(`Absolute),
      top(cursorLine),
      left(0),
      width(totalWidth),
      height(Constants.scrollBarCursorSize),
      backgroundColor(colors.editorForeground),
    ];
  };
};

module Common = {
  type captureState = {
    bbox: Revery.Math.BoundingBox2d.t,
    // The cursor position relative to the thumb
    offset: float,
  };

  let%component make =
                (
                  ~background,
                  ~hoverBackground,
                  ~isVertical,
                  ~beforeTrackClicked,
                  ~afterTrackClicked,
                  ~dragStart,
                  ~dragMove,
                  ~dragStop,
                  ~wheel,
                  ~thumbPosition: float,
                  ~thumbSize: float,
                  ~render,
                  (),
                ) => {
    let getRelevantMousePosition = (mouseX, mouseY) => {
      isVertical ? mouseY : mouseX;
    };

    let getRelevantBoxDimensions = bbox => {
      let (x, y, width, height) = Revery.Math.BoundingBox2d.getBounds(bbox);

      if (isVertical) {
        (y, height);
      } else {
        (x, width);
      };
    };

    let windowSpaceToScrollSpace = (bbox, mouseX, mouseY) => {
      let (pos, _size) = getRelevantBoxDimensions(bbox);
      let mousePos = getRelevantMousePosition(mouseX, mouseY);
      mousePos -. pos;
    };

    let%hook (isHovering, setHovering) = Hooks.state(false);
    //let opacity = isHovering ? 1.0 : 0.8;

    let%hook (captureMouse, _captureState) =
      Hooks.mouseCapture(
        ~onMouseMove=
          (state, evt: NodeEvents.mouseMoveEventParams) => {
            let mousePosition =
              windowSpaceToScrollSpace(state.bbox, evt.mouseX, evt.mouseY);
            dragMove(mousePosition -. state.offset);
            Some(state);
          },
        ~onMouseUp=
          (_state, _evt) => {
            dragStop();
            None;
          },
        (),
      );

    let%hook (maybeBbox, setBbox) = Hooks.state(None);

    let onMouseOver = _ => {
      setHovering(_ => true);
    };

    let onMouseLeave = _ => {
      setHovering(_ => false);
    };

    let onMouseWheel = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      wheel(wheelEvent.deltaY);
    };

    let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
      maybeBbox
      |> Option.iter(bbox => {
           let mousePosition =
             windowSpaceToScrollSpace(bbox, evt.mouseX, evt.mouseY);
           if (mousePosition < thumbPosition) {
             beforeTrackClicked(mousePosition);
           } else if (mousePosition > thumbPosition +. thumbSize) {
             afterTrackClicked(mousePosition);
           } else {
             dragStart();
             captureMouse({bbox, offset: mousePosition -. thumbPosition});
           };
         });
    };

    let color = isHovering ? hoverBackground : background;

    <View
      style={Styles.container(~color)}
      onMouseDown
      onMouseOver
      onMouseLeave
      onMouseWheel
      onBoundingBoxChanged={bbox => setBbox(_ => Some(bbox))}>
      {render()}
    </View>;
  };
};

module Vertical = {
  let diagnosticMarkers =
      (~diagnostics, ~totalHeight, ~editor, ~colors: Colors.t, ()) => {
    IntMap.bindings(diagnostics)
    |> List.map(binding => {
         let (key, _) = binding;
         key;
       })
    |> List.map(line => {
         let diagTop =
           Editor.projectLine(~line, ~pixelHeight=totalHeight, editor)
           |> int_of_float;

         let diagnosticStyle =
           Style.[
             position(`Absolute),
             top(diagTop),
             right(0),
             width(Constants.scrollBarThickness / 3),
             height(Constants.scrollBarCursorSize),
             backgroundColor(colors.errorForeground),
           ];
         <View style=diagnosticStyle />;
       })
    |> React.listToElement;
  };

  let matchingPairMarkers =
      (~bufferHighlights, ~totalHeight, ~editor, ~colors: Colors.t, ()) => {
    let matchingPairStyle = t =>
      Style.[
        position(`Absolute),
        top(t - 3),
        left(4),
        right(4),
        height(8),
        backgroundColor(colors.overviewRulerBracketMatchForeground),
      ];

    BufferHighlights.getMatchingPair(
      Editor.getBufferId(editor),
      bufferHighlights,
    )
    |> Option.map(mp => {
         open Location;
         let (startPos, endPos) = mp;

         let topLine =
           Editor.projectLine(
             ~line=Index.toZeroBased(startPos.line),
             ~pixelHeight=totalHeight,
             editor,
           )
           |> int_of_float;

         let botLine =
           Editor.projectLine(
             ~line=Index.toZeroBased(endPos.line),
             ~pixelHeight=totalHeight,
             editor,
           )
           |> int_of_float;

         React.listToElement([
           <View style={matchingPairStyle(topLine)} />,
           <View style={matchingPairStyle(botLine)} />,
         ]);
       })
    |> Option.value(~default=React.empty);
  };

  let searchMarkers =
      (~bufferHighlights, ~totalHeight, ~editor, ~colors: Colors.t, ()) => {
    let searchMatches = t =>
      Style.[
        position(`Absolute),
        top(t - 3),
        left(4),
        right(4),
        height(8),
        backgroundColor(colors.findMatchBackground),
      ];

    let searchHighlightToElement = line => {
      let line = Index.toZeroBased(line);
      let position =
        Editor.projectLine(~line, ~pixelHeight=totalHeight, editor)
        |> int_of_float;
      <View style={searchMatches(position)} />;
    };

    BufferHighlights.getHighlights(
      ~bufferId=Editor.getBufferId(editor),
      bufferHighlights,
    )
    |> List.map(searchHighlightToElement)
    |> React.listToElement;
  };

  let selectionMarkers = (~totalHeight, ~editor, ~colors: Colors.t, ()) => {
    let selectionStyle = (t, bot) => {
      Style.[
        position(`Absolute),
        top(t),
        left(0),
        right(0),
        height(bot - t),
        backgroundColor(
          Revery.Color.multiplyAlpha(0.5, colors.selectionBackground),
        ),
      ];
    };
    let getSelectionElements = (selection: VisualRange.t) => {
      switch (selection.mode) {
      | Vim.Types.None => []
      | _ =>
        let topLine =
          Editor.projectLine(
            ~line=Index.toZeroBased(selection.range.start.line),
            ~pixelHeight=totalHeight,
            editor,
          )
          |> int_of_float;
        let botLine =
          Editor.projectLine(
            ~line=Index.toZeroBased(selection.range.stop.line) + 1,
            ~pixelHeight=totalHeight,
            editor,
          )
          |> int_of_float;
        [<View style={selectionStyle(topLine, botLine)} />];
      };
    };

    getSelectionElements(editor.selection) |> React.listToElement;
  };

  let make =
      (
        ~dispatch: Msg.t => unit,
        ~editor: Editor.t,
        ~cursorPosition: Location.t,
        ~height as totalHeight,
        ~width as totalWidth,
        ~diagnostics: IntMap.t(list(Diagnostic.t)),
        ~colors: Colors.t,
        ~bufferHighlights,
        (),
      ) => {
    let scrollMetrics =
      Editor.getVerticalScrollbarMetrics(editor, totalHeight);

    let cursorLine =
      Editor.projectLine(
        ~line=Index.toZeroBased(Location.(cursorPosition.line)),
        ~pixelHeight=totalHeight,
        editor,
      )
      |> int_of_float;

    let scrollbarPixelToEditorPixel = pixelPosition => {
      let (_editorPixelX, editorPixelY) =
        Editor.unprojectToPixel(
          ~pixelX=0.,
          ~pixelY=pixelPosition,
          ~pixelWidth=1,
          ~pixelHeight=totalHeight - scrollMetrics.thumbSize,
          editor,
        );
      editorPixelY;
    };

    let beforeTrackClicked = pos => {
      let newPixelScrollY = pos |> scrollbarPixelToEditorPixel;
      dispatch(
        Msg.VerticalScrollbarBeforeTrackClicked({
          newPixelScrollY: newPixelScrollY,
        }),
      );
    };
    let afterTrackClicked = pos => {
      let newPixelScrollY = pos |> scrollbarPixelToEditorPixel;

      dispatch(
        Msg.VerticalScrollbarBeforeTrackClicked({
          newPixelScrollY: newPixelScrollY,
        }),
      );
    };
    let dragStart = () => dispatch(Msg.VerticalScrollbarMouseDown);
    let dragMove = pos => {
      let newPixelScrollY = pos |> scrollbarPixelToEditorPixel;
      dispatch(
        Msg.VerticalScrollbarMouseDrag({newPixelScrollY: newPixelScrollY}),
      );
    };
    let dragStop = () => dispatch(Msg.VerticalScrollbarMouseRelease);
    let wheel = deltaWheel =>
      dispatch(
        Msg.VerticalScrollbarMouseWheel({deltaWheel: (-1.0) *. deltaWheel}),
      );

    <Common
      background={colors.scrollbarSliderBackground}
      hoverBackground={colors.scrollbarSliderHoverBackground}
      isVertical=true
      thumbPosition={scrollMetrics.thumbOffset |> float_of_int}
      thumbSize={scrollMetrics.thumbSize |> float_of_int}
      beforeTrackClicked
      afterTrackClicked
      dragStart
      dragMove
      dragStop
      wheel
      render={() => {
        <View style=Styles.absolute>
          <View
            style={Styles.verticalThumb(
              ~width=totalWidth,
              ~scrollMetrics,
              ~color=colors.scrollbarSliderBackground,
            )}
          />
          <View style={Styles.cursor(~cursorLine, ~totalWidth, ~colors)} />
          <View style=Styles.absolute>
            <selectionMarkers totalHeight editor colors />
            <diagnosticMarkers totalHeight editor diagnostics colors />
            <matchingPairMarkers bufferHighlights editor totalHeight colors />
            <searchMarkers bufferHighlights editor totalHeight colors />
          </View>
        </View>
      }}
    />;
  };
};
