/*
 * Editor.re
 *
 * Editor component - an 'Editor' encapsulates the following:
 * - a 'tabbar'
 * - an editor surface - usually a textual buffer view
 */

open Revery.UI;
open Oni_Model;
module Model = Oni_Model;

let component = React.component("Editor");

let editorViewStyle = (background, foreground) =>
  Style.[
    backgroundColor(background),
    color(foreground),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
    flexDirection(`Column),
  ];

/**
   We wrap each split component as we have to have a type signature
   that matches unit => React.syntheticElement this is because
   in the WindowManager module we cannot pass a reference of state
   in the type signature e.g. State.t => React.syntheticElement because
   this would cause a circular reference.

   Alternatives are type parameters but this invloves a lot of unrelated
   type params being added everywhere. ?Functors is another route
 */
let splitFactory = (fn, ()) => {
  let state = GlobalContext.current().getState();
  fn(state);
};

let createElement = (~state: State.t, ~children as _, ()) =>
  component(hooks => {
    let theme = state.theme;
    let hooks =
      React.Hooks.effect(
        OnMount,
        () => {
          open WindowManager;

          let dispatch = GlobalContext.current().dispatch;
          let dock =
            registerDock(
              ~order=1,
              ~width=50,
              ~id=MainDock,
              ~component=splitFactory(state => <Dock state />),
              (),
            );

          let editorGroupId = state.editorGroups.activeId;

          let editor = createSplit(~direction=Vertical, ~editorGroupId, ());

          let explorer =
            registerDock(
              ~order=2,
              ~width=225,
              ~id=ExplorerDock,
              ~component=splitFactory(state => <FileExplorerView state />),
              (),
            );

          dispatch(RegisterDockItem(dock));
          dispatch(RegisterDockItem(explorer));
          dispatch(AddSplit(editor));
          None;
        },
        hooks,
      );

    let style =
      editorViewStyle(theme.colors.background, theme.colors.foreground);

    (hooks, <View style> <EditorLayoutView state /> </View>);
  });
