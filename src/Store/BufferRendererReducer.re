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
        switch (renderer) {
        | Welcome =>
          switch (action) {
          | Actions.BufferUpdate(bu) when bu.update.id == id => Editor
          | _ => Welcome
          }
        | Editor => Editor
        | Terminal(_) as term => term
        }
      );
    };
    BufferRenderers.mapi(reduceForBuffer, state);
  };
};
