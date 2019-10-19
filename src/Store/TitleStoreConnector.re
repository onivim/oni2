/*
 * TitleStoreConnector
 *
 * This implements an updater (reducer + side effects) for the window title
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module Menu = Model.Menu;
module MenuJob = Model.MenuJob;

let start = () => {

  let lastTitle = ref("");

  let updateTitleEffect = (state) => 
    Isolinear.Effect.createWithDispatch(
    ~name="title.update", (dispatch) => {

    });

  let updater = (state: Model.State.t, action: Actions.t) => {
    switch (action) {
    | _ => (state, Isolinear.Effect.none)
    };
  };
  updater;
};
