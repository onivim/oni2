open Oni_Core;

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
  | CancelClicked({messageId: int})
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

exception MessageCancelled;

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

  let cancel = (~resolver) =>
    Isolinear.Effect.create(~name="Feature_Messages.Effect.resolve", () => {
      Lwt.wakeup_exn(resolver, MessageCancelled)
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
  | CancelClicked({messageId}) =>
    let maybeMessage =
      model.messages
      |> List.filter((message: message) => message.messageId == messageId)
      |> (list => List.nth_opt(list, 0));

    maybeMessage
    |> Option.map(message => {
         let eff = Effect.cancel(~resolver=message.resolver);

         let model = Internal.removeMessage(~messageId, model);

         (model, Effect(eff));
       })
    |> Option.value(~default=(model, Nothing));
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

module Constants = {
  let totalHeight = 150;
  let totalWidth = 360;
};

module View = {
  open Revery;
  open Revery.UI;

  module Colors = Feature_Theme.Colors;
  module Sneakable = Feature_Sneak.View.Sneakable;

  module Styles = {
    open Style;

    let color = Color.rgba(0., 0., 0., 0.75);

    let container = (~theme) => [
      position(`Absolute),
      backgroundColor(Colors.Notifications.background.from(theme)),
      bottom(100),
      right(100),
      width(Constants.totalWidth),
      height(Constants.totalHeight),
      boxShadow(
        ~xOffset=4.,
        ~yOffset=4.,
        ~blurRadius=12.,
        ~spreadRadius=0.,
        ~color,
      ),
      border(~color=Colors.Notifications.border.from(theme), ~width=1),
      flexDirection(`Column),
      pointerEvents(`Allow),
      flexGrow(0),
      overflow(`Hidden),
    ];

    let title = (~theme) => [
      backgroundColor(Colors.Notifications.headerBackground.from(theme)),
      height(25),
      width(Constants.totalWidth - 2),
      flexGrow(0),
      flexShrink(0),
      flexDirection(`Row),
      borderBottom(
        ~color=Colors.Notifications.headerBorder.from(theme),
        ~width=1,
      ),
    ];

    let titleText = [flexGrow(1), flexShrink(0), padding(8)];

    let contents = [
      width(Constants.totalWidth),
      flexDirection(`Row),
      flexGrow(1),
    ];

    let iconContainer = [
      flexGrow(0),
      flexShrink(0),
      width(25),
      height(25),
      alignItems(`Center),
      justifyContent(`Center),
    ];

    let text = [width(Constants.totalWidth - 8 - 25), padding(8)];

    let buttons = [
      flexDirection(`Row),
      flexShrink(0),
      pointerEvents(`Allow),
      justifyContent(`FlexStart),
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

    <Oni_Components.Button font theme onClick label={command.title} />;
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
    <View style={Styles.container(~theme)}>
      <View style={Styles.title(~theme)}>
        <View style=Styles.titleText>
          <Text
            fontFamily={font.family}
            fontWeight=Revery.Font.Weight.Bold
            fontSize=12.
            text="NOTIFICATIONS"
            style=[
              Style.color(Colors.Notifications.headerForeground.from(theme)),
            ]
          />
        </View>
        <View style=Styles.iconContainer>
          <Sneakable
            sneakId={string_of_int(messageId)}
            onClick={() => dispatch(CancelClicked({messageId: messageId}))}>
            <Codicon
              color={Colors.Notifications.foreground.from(theme)}
              icon=Codicon.close
            />
          </Sneakable>
        </View>
      </View>
      <Revery.UI.Components.ScrollView style=Styles.contents>
        <View style=Styles.text>
          <Text
            fontFamily={font.family}
            fontSize=12.
            text=title
            style=[Style.color(Colors.Notifications.foreground.from(theme))]
          />
        </View>
      </Revery.UI.Components.ScrollView>
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
      |> List.map(({messageId, message, commands, _}) => {
           <messageItem messageId font theme title=message commands dispatch />
         })
      |> React.listToElement;
    };
  };
};
