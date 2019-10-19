/*
 * TitleStoreConnector
 *
 * This implements an updater (reducer + side effects) for the window title
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;

let start = () => {

  let _lastTitle = ref("");

  let updateTitleEffect = (state) => 
    Isolinear.Effect.createWithDispatch(
    ~name="title.update", (dispatch) => {
 ();
    });

  let updater = (state: Model.State.t, action: Actions.t) => {
    switch (action) {
    | _ => (state, updateTitleEffect(state))
    };
  };
  updater;
};
