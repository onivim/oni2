/*
  [<Sneakable />] is analagous to [<Clickable />], but also supports
  registering as a provider for sneak mode.

  When sneak-mode is activated, the element will be measured, and
  if visible and available, allowed to participate in the sneak session.
 */

open Revery.UI;
open Revery.UI.Components;

module Utility = Oni_Core_Utility;

let%component make =
              (
                ~style=[],
                ~onClick=() => (),
                ~onRightClick=() => (),
                ~onAnyClick=_ => (),
                ~onSneak=?,
                ~onBlur=?,
                ~onFocus=?,
                ~tabindex=?,
                ~onKeyDown=?,
                ~onKeyUp=?,
                ~onTextEdit=?,
                ~onTextInput=?,
                ~children,
                (),
              ) => {
  let%hook (holder: ref(option(Revery.UI.node)), _) =
    Hooks.state(ref(None));

  let componentRef = (node: Revery.UI.node) => {
    holder := Some(node);
  };

  let%hook () =
    Hooks.effect(
      OnMount,
      () => {
        switch (onSneak) {
        | Some(cb) => SneakRegistry.register(holder, cb)
        | None => SneakRegistry.register(holder, onClick)
        };

        Some(
          () => {
            holder := None;
            SneakRegistry.unregister(holder);
          },
        );
      },
    );

  <Clickable
    style
    onClick
    onRightClick
    onAnyClick
    componentRef
    ?onBlur
    ?onFocus
    ?tabindex
    ?onKeyDown
    ?onKeyUp
    ?onTextEdit
    ?onTextInput>
    children
  </Clickable>;
};
