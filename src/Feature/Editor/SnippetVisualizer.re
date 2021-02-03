open EditorCoreTypes;
open Feature_Snippets;

open Revery.Draw;

module Log = (
  val Oni_Core.Log.withNamespace("Feature_Editor.SnippetVisualizer")
);

module Constants = {
  let snippetMargin = 3.;
};

let snippetPaint = Skia.Paint.make();

let draw =
    (~config, ~context: Draw.context, session: Feature_Snippets.Session.t) => {
  let isShadowEnabled =
    Feature_Configuration.GlobalConfiguration.shadows.get(config);

  if (!isShadowEnabled) {
    ();
  } else {
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

    let topY = topPixel.y -. Constants.snippetMargin;

    CanvasContext.drawRectLtwh(
      ~left=0.,
      ~top=0.,
      ~height=topY,
      ~width,
      ~paint=snippetPaint,
      context.canvasContext,
    );

    Oni_Components.ScrollShadow.Shadow.render(
      ~color=Revery.Color.rgba(0., 0., 0., 0.1),
      ~opacity=1.0,
      ~direction=Up,
      ~x=0.,
      ~y=topY -. 5.,
      ~width,
      ~height=5.,
      ~context=Revery.Draw.CanvasContext.(context.canvasContext.canvas),
    );

    let (bottomPixel, _) =
      Editor.bufferBytePositionToPixel(
        ~position=
          BytePosition.{
            line: Session.stopLine(session),
            byte: ByteIndex.zero,
          },
        context.editor,
      );

    let bottomY = bottomPixel.y +. Constants.snippetMargin;

    CanvasContext.drawRectLtwh(
      ~left=0.,
      ~top=bottomY,
      ~height=height -. bottomY,
      ~width,
      ~paint=snippetPaint,
      context.canvasContext,
    );
    Oni_Components.ScrollShadow.Shadow.render(
      ~color=Revery.Color.rgba(0., 0., 0., 0.22),
      ~opacity=1.0,
      ~direction=Down,
      ~x=0.,
      ~y=bottomY,
      ~width,
      ~height=5.,
      ~context=Revery.Draw.CanvasContext.(context.canvasContext.canvas),
    );
  };
};
