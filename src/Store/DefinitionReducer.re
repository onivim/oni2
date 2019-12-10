/*
 * DefinitionReducer.re
 */

open Oni_Extensions;

open Oni_Model;

let reduce = (action: Actions.t, state: Definition.t) => {
  switch (action) {
  | EditorCursorMove(_) =>
    Definition.empty
  | DefinitionAvailable(bufferId, position, result) =>
    Definition.set(bufferId, position, result) 
  | _ => state
  };
};
