open EditorCoreTypes;
open Feature_Snippets;

open Revery.Draw;

module Log = (
  val Oni_Core.Log.withNamespace("Feature_Editor.SnippetVisualizer")
);

let snippetPaint = Skia.Paint.make();

let draw = (~context: Draw.context, session: Feature_Snippets.Session.t) => {
  let color = Revery.Color.toSkia(Revery.Color.rgba(0., 0., 0., 0.1));
  Skia.Paint.setColor(snippetPaint, color);
  let width = float(context.width);
  let height = float(context.height);

  let (topPixel, _) =
    Editor.bufferBytePositionToPixel(
      ~position=
        BytePosition.{
          line: Session.startLine(session),
          byte: ByteIndex.zero,
        },
      context.editor,
    );

  CanvasContext.drawRectLtwh(
    ~left=0.,
    ~top=0.,
    ~height=topPixel.y,
    ~width,
    ~paint=snippetPaint,
    context.canvasContext,
  );

  Oni_Components.ScrollShadow.Shadow.render(
    ~color=Revery.Color.rgba(0., 0., 0., 0.1),
    ~opacity=1.0,
    ~direction=Up,
    ~x=0.,
    ~y=topPixel.y -. 5.,
    ~width,
    ~height=5.,
    ~context=Revery.Draw.CanvasContext.(context.canvasContext.canvas),
  );
  // CanvasContext.drawRectLtwh(
  //   ~left=0.,
  //   ~top=topPixel.y,
  //   ~height=2.,
  //   ~width,
  //   ~paint=snippetPaint,
  //   context.canvasContext,
  // );

  let (bottomPixel, _) =
    Editor.bufferBytePositionToPixel(
      ~position=
        BytePosition.{line: Session.stopLine(session), byte: ByteIndex.zero},
      context.editor,
    );
  CanvasContext.drawRectLtwh(
    ~left=0.,
    ~top=bottomPixel.y,
    ~height=height -. bottomPixel.y,
    ~width,
    ~paint=snippetPaint,
    context.canvasContext,
  );
  Oni_Components.ScrollShadow.Shadow.render(
    ~color=Revery.Color.rgba(0., 0., 0., 0.22),
    ~opacity=1.0,
    ~direction=Down,
    ~x=0.,
    ~y=bottomPixel.y,
    ~width,
    ~height=5.,
    ~context=Revery.Draw.CanvasContext.(context.canvasContext.canvas),
  );
};
