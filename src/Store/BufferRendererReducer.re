/*
 * BufferRendererReducer
 */

open Oni_Model;

let reduce = (state: BufferRenderers.t, action: Actions.t) => {
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
