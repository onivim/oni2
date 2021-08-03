type model = unit;

let initial = ();

exception UserCancelled;

[@deriving show]
type msg =
  | Noop
  | Accepted({
      text: string,
      resolver: [@opaque] Lwt.u(Exthost.Reply.t),
    })
  | Cancelled({resolver: [@opaque] Lwt.u(Exthost.Reply.t)})
  | MenuItemSelected({resolver: [@opaque] Lwt.u(Exthost.Reply.t), item: Exthost.QuickOpen.Item.t})
  | ShowInput({
      options: Exthost.InputBoxOptions.t,
      resolver: [@opaque] Lwt.u(Exthost.Reply.t),
    })
  | ShowQuickPick({ 
    instance: int,
    items: list(Exthost.QuickOpen.Item.t),
    resolver: [@opaque] Lwt.u(Exthost.Reply.t),
  });

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | ShowMenu(Feature_Quickmenu.Schema.menu(msg));

module Msg = {
  let exthost = (~resolver, msg) =>
    Exthost.Msg.QuickOpen.(
      switch (msg) {
      | Input({options, _}) => ShowInput({options, resolver})
      | SetItems({instance, items}) => ShowQuickPick({ instance, items, resolver })
      | Show({instance, _}) => ShowQuickPick({ instance, items: [], resolver})
      | _ => Noop
      }
    );
};

module Effects = {
  let cancel = (~resolver) =>
    Isolinear.Effect.create(~name="Feature_QuickOpen.cancel", () => {
      Lwt.wakeup_exn(resolver, UserCancelled)
    });

  let accept = (~resolver, text) =>
    Isolinear.Effect.create(~name="Feature_QuickOpen.accept", () => {
      Lwt.wakeup(resolver, Exthost.Reply.okJson(`String(text)))
    });

  let selectItem = (~resolver, ~item: Exthost.QuickOpen.Item.t) =>
    Isolinear.Effect.create(~name="Feature_QuickOpen.select", () => {
      prerr_endline ("SELECTING:  "++ string_of_int(item.handle));
      Lwt.wakeup(resolver, Exthost.Reply.okJson(`Int(item.handle)));
    });

  let ok = (~resolver) =>
    Isolinear.Effect.create(~name="Feature_QuickOpen.replyOk", () => {
      Lwt.wakeup(resolver, Exthost.Reply.okEmpty)
    });
};

let update = (msg, model) =>
  switch (msg) {
  | Accepted({resolver, text}) => (
      model,
      Effect(Effects.accept(~resolver, text)),
    )

  | Cancelled({resolver}) => (model, Effect(Effects.cancel(~resolver)))

  | MenuItemSelected({item, resolver}) => (
    model,
    Effect(Effects.selectItem(~resolver, ~item))
  )

  | ShowQuickPick({items, resolver}) =>
    let placeholderText = "";
    let menu =
      Feature_Quickmenu.Schema.(
        menu(
          ~onItemFocused=_item => Noop,
          ~onAccepted=(~text, ~item) => {
            switch (item) {
          | Some(item) => MenuItemSelected({item, resolver})
          | None => Cancelled({resolver: resolver})
          }
          },
          ~onCancelled=_item => {Cancelled({resolver: resolver})},
          ~placeholderText,
          ~itemRenderer=Renderer.default,
          ~toString={(item: Exthost.QuickOpen.Item.t) => Exthost.QuickOpen.Item.(item.label)},
          items,
        )
      );
    (model, ShowMenu(menu));

  | ShowInput({options, resolver}) =>
    let placeholderText =
      Exthost.InputBoxOptions.(options.placeHolder)
      |> Option.value(~default="");
    let menu =
      Feature_Quickmenu.Schema.(
        menu(
          ~onItemFocused=_item => Noop,
          ~onAccepted=(~text, ~item as _) => {Accepted({text, resolver})},
          ~onCancelled=_item => {Cancelled({resolver: resolver})},
          ~placeholderText,
          ~itemRenderer=Renderer.default,
          ~toString=Fun.id,
          [],
        )
      );
    (model, ShowMenu(menu));

  | Noop => (model, Nothing)
  };
