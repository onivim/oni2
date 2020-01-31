/*
 * Minimap.re
 *
 * Component that handles Minimap rendering
 */

open EditorCoreTypes;
open Oni_Core;
open Revery;
open Revery.Draw;
open Revery.UI;

let cachedPaint = Skia.Paint.make();

let drawRect = (~x, ~y, ~width, ~height, ~color, canvasContext) => {
  Skia.Paint.setColor(cachedPaint, Color.toSkia(color));
  let rect = Skia.Rect.makeLtrb(x, y, x +. width, y +. height);
  CanvasContext.drawRect(~rect, ~paint=cachedPaint, canvasContext);
};
