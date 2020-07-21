/*
 * DefinitionReducer.re
 */

open Oni_Model;

module Definition = Feature_LanguageSupport.Definition;

let reduce = (action: Actions.t, state: Definition.t) => {
  switch (action) {
  | Editor({msg: CursorsChanged(_), _}) => Definition.empty
  | DefinitionAvailable(bufferId, position, result) =>
    Definition.create(bufferId, position, result)
  | _ => state
  };
};
