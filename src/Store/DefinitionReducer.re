/*
 * DefinitionReducer.re
 */

open Oni_Model;

let reduce = (action: Actions.t, state: Definition.t) => {
  switch (action) {
  | EditorCursorMove(_) => Definition.empty
  | DefinitionAvailable(bufferId, position, result) =>
    Definition.create(bufferId, position, result)
  | _ => state
  };
};
