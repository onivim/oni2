/*
 * BufferRendererReducer
 */

open Oni_Model;

let terminalReducer:
  (BufferRenderer.terminal, Actions.t) => BufferRenderer.terminal =
  (terminal, action) => {
    switch (action) {
    | Actions.Terminal(
        Feature_Terminal.Service(
          Service_Terminal.ProcessTitleChanged({id, title, _}),
        ),
      )
        when terminal.id == id => {
        ...terminal,
        id,
        title,
      }
    | Command("terminal.normalMode") => {...terminal, insertMode: false}
    | Command("terminal.insertMode") => {...terminal, insertMode: true}

    | _ => terminal
    };
  };

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
        | Terminal(term) => Terminal(terminalReducer(term, action))
        }
      );
    };
    BufferRenderers.mapi(reduceForBuffer, state);
  };
};
