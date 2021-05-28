open Oni_Core;

open Oni_Model;
open Oni_Model.Actions;

let start = () => {
  let togglePathEffect = name =>
    Isolinear.Effect.create(
      ~name,
      () => {
        let _ =
          Oni_Core.NodeTask.run(
            ~setup=Oni_Core.Setup.init(),
            "add-to-path.js",
          );
        ();
      },
    );

  let openChangelogEffect = _ =>
    Isolinear.Effect.createWithDispatch(~name="oni.changelog", dispatch => {
      dispatch(
        OpenFileByPath(BufferPath.changelog, SplitDirection.Current, None),
      )
    });

  let commands = [
    ("system.addToPath", _ => togglePathEffect),
    ("system.removeFromPath", _ => togglePathEffect),
    ("oni.changelog", _ => openChangelogEffect),
  ];

  let commandMap =
    List.fold_left(
      (prev, curr) => {
        let (command, handler) = curr;
        StringMap.add(command, handler, prev);
      },
      StringMap.empty,
      commands,
    );

  let updater = (state: State.t, action) => {
    switch (action) {
    | CommandInvoked({command: cmd, _}) =>
      switch (StringMap.find_opt(cmd, commandMap)) {
      | Some(v) => (state, v(state, cmd))
      | None => (state, Isolinear.Effect.none)
      }
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
