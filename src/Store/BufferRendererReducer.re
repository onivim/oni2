/*
 * BufferRendererReducer
 */

open Oni_Model;

let reduce = (state: BufferRenderers.t, action) => {
  switch (action) {
  | Actions.BufferRenderer(BufferRenderer.RendererAvailable(id, renderer)) =>
    BufferRenderers.setById(id, renderer, state)
  | action =>
    let reduceForBuffer = renderer => {
      BufferRenderer.(
        switch (renderer, action) {
        | (Terminal(state), Terminal(msg)) =>
          Terminal(Feature_Terminal.bufferRendererReducer(state, msg))
        | _ => renderer
        }
      );
    };
    BufferRenderers.map(reduceForBuffer, state);
  };
};
