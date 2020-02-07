/*
 * BufferRenderers.re
 */

open Oni_Core;

type t = {
  rendererById: IntMap.t(BufferRenderer.t),
  // TODO: Probably get rid of this and simplify types / interface
  rendererByFiletype: StringMap.t(BufferRenderer.t),
};

let initial = {
  rendererById: IntMap.empty,
  rendererByFiletype: StringMap.empty,
};

let map = (f: BufferRenderer.t => BufferRenderer.t, renderers: t) => {
  ...renderers,
  rendererById: IntMap.map(f, renderers.rendererById),
};

let mapi = (f: (int, BufferRenderer.t) => BufferRenderer.t, renderers: t) => {
  ...renderers,
  rendererById: IntMap.mapi(f, renderers.rendererById),
};

let getById = (bufferId: int, {rendererById, _}: t) =>
  IntMap.find_opt(bufferId, rendererById)
  |> Option.value(~default=BufferRenderer.Editor);

let setById = (bufferId, renderer, renderers: t) => {
  ...renderers,
  rendererById: IntMap.add(bufferId, renderer, renderers.rendererById),
};

let setByFiletype = (filetype, renderer, {rendererByFiletype, _}: t) =>
  StringMap.add(filetype, renderer, rendererByFiletype);
