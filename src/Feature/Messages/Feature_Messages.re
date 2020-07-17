open Oni_Core;
open Oni_Core.Utility;

[@deriving show]
type msg =
  | NotificationReceived({
      severity: Exthost.Message.severity,
      message: string,
    })
  | MessageReceived({
      severity: Exthost.Message.severity,
      message: string,
      commands: list(Exthost.Message.Command.t),
      resolver: [@opaque] Lwt.u(option(Exthost.Message.handle)),
    })
  | CommandClicked({
      messageId: int,
      handle: Exthost.Message.handle,
    });

module Msg = {
  let exthost = (~dispatch, msg) => {
    let (msg, promise) =
      Exthost.Msg.MessageService.(
        switch (msg) {
        | ShowMessage({severity, message, commands, _}) =>
          // No commands, just a notification...
          if (commands == []) {
            let msg = NotificationReceived({severity, message});
            (msg, Lwt.return(None));
          } else {
            let (promise, resolver) = Lwt.task();

            let msg =
              MessageReceived({severity, message, commands, resolver});

            (msg, promise);
          }
        }
      );

    dispatch(msg);
    promise;
  };
};

type message = {
  messageId: int,
  severity: Exthost.Message.severity,
  message: string,
  commands: list(Exthost.Message.Command.t),
  resolver: Lwt.u(option(Exthost.Message.handle)),
};

type model = {
  nextId: int,
  messages: list(message),
};

let initial = {nextId: 0, messages: []};

module Internal = {
  let severityToKind =
    Feature_Notification.(
      fun
      | Exthost.Message.Ignore => Info
      | Exthost.Message.Info => Info
      | Exthost.Message.Warning => Warning
      | Exthost.Message.Error => Error
    );

  let removeMessage = (~messageId, model) => {
    ...model,
    messages:
      List.filter(message => message.messageId != messageId, model.messages),
  };
};

type outmsg =
  | Nothing
  | Notification({
      kind: Feature_Notification.kind,
      message: string,
    })
  | Effect(Isolinear.Effect.t(msg));

module Effect = {
  let resolve = (~resolver, handle) =>
    Isolinear.Effect.create(~name="Feature_Messages.Effect.resolve", () => {
      Lwt.wakeup(resolver, Some(handle))
    });
};

let update = (msg, model) => {
  switch (msg) {
  | NotificationReceived({severity, message}) => (
      model,
      Notification({kind: Internal.severityToKind(severity), message}),
    )
  | MessageReceived({severity, message, commands, resolver}) =>
    let id = model.nextId;

    (
      {
        nextId: id + 1,
        messages: [
          {messageId: id, severity, message, commands, resolver},
          ...model.messages,
        ],
      },
      Nothing,
    );
  | CommandClicked({messageId, handle}) =>
    let maybeMessage =
      model.messages
      |> List.filter((message: message) => message.messageId == messageId)
      |> (list => List.nth_opt(list, 0));

    maybeMessage
    |> Option.map(message => {
         let eff = Effect.resolve(~resolver=message.resolver, handle);

         let model = Internal.removeMessage(~messageId, model);

         (model, Effect(eff));
       })
    |> Option.value(~default=(model, Nothing));
  };
};

module View = {
  open Revery;
  open Revery.UI;
  open Revery.UI.Components;

  module Sneakable = Feature_Sneak.View.Sneakable;

  module Styles = {
    open Style;

    let container = [
      position(`Absolute),
      backgroundColor(Colors.black),
      bottom(100),
      right(100),
      width(360),
      height(120),
      pointerEvents(`Allow),
      //      flexDirection(`Column),
      //      alignItems(`Center),
      //      justifyContent(`SpaceBetween),
    ];

    let text = (~theme) => [flexGrow(1)];

    let buttons = [flexDirection(`Row)];

    let button = (~theme) => [
      border(~width=1, ~color=Colors.red),
      margin(8),
    ];
  };

  let command =
      (
        ~messageId,
        ~font: UiFont.t,
        ~theme,
        ~command: Exthost.Message.Command.t,
        ~dispatch,
        (),
      ) => {
    let onClick = () =>
      dispatch(CommandClicked({messageId, handle: command.handle}));

    <Clickable style={Styles.button(~theme)} onClick>
      <Text fontFamily={font.family} fontSize=12. text={command.title} />
    </Clickable>;
  };

  let messageItem =
      (
        ~messageId,
        ~font: UiFont.t,
        ~theme,
        ~title,
        ~commands: list(Exthost.Message.Command.t),
        ~dispatch,
        (),
      ) => {
    <View style=Styles.container>
      <View style={Styles.text(~theme)}>
        <Text fontFamily={font.family} fontSize=12. text=title />
      </View>
      <View style=Styles.buttons>
        {commands
         |> List.map(commandInfo => {
              <command messageId command=commandInfo font theme dispatch />
            })
         |> React.listToElement}
      </View>
    </View>;
  };

  let make =
      (
        ~theme: ColorTheme.Colors.t,
        ~model: model,
        ~font: UiFont.t,
        ~dispatch: msg => unit,
        (),
      ) => {
    let {messages, _} = model;

    if (messages == []) {
      React.empty;
    } else {
      messages
      |> List.map(({messageId, message, commands, severity, resolver}) => {
           <messageItem messageId font theme title=message commands dispatch />
         })
      |> React.listToElement;
    };
  };
};
