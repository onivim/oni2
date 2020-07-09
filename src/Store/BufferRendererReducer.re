/*
 * BufferRendererReducer
 */

open Oni_Model;

let reduce = (state: BufferRenderers.t, action) => {
  switch (action) {
  | Actions.BufferRenderer(BufferRenderer.RendererAvailable(id, renderer)) =>
    BufferRenderers.setById(id, renderer, state)
  | action =>
    let reduceForBuffer = (id, renderer) => {
      BufferRenderer.(
        switch (renderer, action) {
        | (Welcome, BufferUpdate(bu)) when bu.update.id == id => Editor
        | (Version, _) => Version
        | (Editor, _) => Editor
        | (Terminal(state), Terminal(msg)) =>
          Terminal(Feature_Terminal.bufferRendererReducer(state, msg))
        | _ => renderer
        }
      );
    };
    BufferRenderers.mapi(reduceForBuffer, state);
  };
};
