module Model = {
  type session = {
    id: int,
    resolver: Lwt.u(Exthost.Reply.t),
  };

  type t = option(session);
  let start = (~id, ~resolver, current) =>
    switch (current) {
    | Some({id: previousId, _}) when previousId == id => current
    | Some({id: previousId, _}) when previousId != id => Some({id, resolver})
    | None => Some({id, resolver})
    | _ => failwith("Never hit this")
    };

  let reset = (_: t) => None;

  let initial = None;
};

type model = Model.t;

let initial = Model.initial;

exception UserCancelled;

[@deriving show]
type msg =
  | Noop
  | Accepted({
      text: string,
      resolver: [@opaque] Lwt.u(Exthost.Reply.t),
    })
  | Cancelled({resolver: [@opaque] Lwt.u(Exthost.Reply.t)})
  | MenuCancelled
  | MenuItemSelected({item: Exthost.QuickOpen.Item.t})
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
      | SetItems({instance, items}) =>
        ShowQuickPick({instance, items, resolver})
      | Show({instance, _}) => ShowQuickPick({instance, items: [], resolver})
      | CreateOrUpdate({params}) =>
        Exthost.QuickOpen.(
          {
            switch (params) {
            | QuickInput(input) =>
              ShowInput({
                options: {
                  ignoreFocusOut: false,
                  password: false,
                  placeHolder: input.placeholder,
                  prompt: input.prompt,
                  value: input.value,
                },
                resolver,
              })
            | QuickPick({id, items, _}) =>
              ShowQuickPick({instance: id, items, resolver})
            };
          }
        )
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

  let selectItem =
      (~session, ~resolver, ~item: Exthost.QuickOpen.Item.t, client) =>
    Isolinear.Effect.create(~name="Feature_QuickOpen.select", () => {
      Exthost.Request.QuickOpen.onDidChangeActive(
        ~session,
        ~handles=[item.handle],
        client,
      );

      Exthost.Request.QuickOpen.onDidAccept(~session, client);

      Lwt.wakeup(resolver, Exthost.Reply.okJson(`Int(item.handle)));
    });
};

let update = (~client, msg, model) =>
  switch (msg) {
  | Accepted({resolver, text}) => (
      model |> Model.reset,
      Effect(Effects.accept(~resolver, text)),
    )

  | Cancelled({resolver}) => (
      model |> Model.reset,
      Effect(Effects.cancel(~resolver)),
    )

  | MenuItemSelected({item}) =>
    let eff =
      switch (model) {
      | None => Isolinear.Effect.none
      | Some({resolver, id}) =>
        Effects.selectItem(~session=id, ~resolver, ~item, client)
      };

    (model |> Model.reset, Effect(eff));

  | MenuCancelled =>
    let eff =
      switch (model) {
      | None => Isolinear.Effect.none
      | Some({resolver, _}) => Effects.cancel(~resolver)
      };
    (model |> Model.reset, Effect(eff));

  | ShowQuickPick({instance, items, resolver}) =>
    let placeholderText = "";
    let menu =
      Feature_Quickmenu.Schema.(
        menu(
          ~onItemFocused=_item => Noop,
          ~onAccepted=
            (~text as _, ~item) => {
              switch (item) {
              | Some(item) => MenuItemSelected({item: item})
              | None => MenuCancelled
              }
            },
          ~onCancelled=_item => {Cancelled({resolver: resolver})},
          ~placeholderText,
          ~itemRenderer=Renderer.default,
          ~toString=
            (item: Exthost.QuickOpen.Item.t) =>
              Exthost.QuickOpen.Item.(item.label),
          items,
        )
      );
    (model |> Model.start(~id=instance, ~resolver), ShowMenu(menu));

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
    (model |> Model.reset, ShowMenu(menu));

  | Noop => (model, Nothing)
  };
