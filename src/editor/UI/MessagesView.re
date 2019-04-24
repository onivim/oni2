open Revery_UI;
open Oni_Model;
open Revery.Colors;
open Revery.UI.Components;

module Core = Oni_Core;

let component = React.component("MessagesView");
let messagesStyles =
  Style.[
    position(`Relative),
    height(100),
    left(0),
    right(0),
    padding(10),
  ];
let messageHeight = 20;

let renderMessage =
    (message, messageKind: Core.Types.messageKind, state: State.t) => {
  let font = state.uiFont.fontFile;
  switch (messageKind) {
  | ReturnPrompt =>
    <Text
      text=message
      style=Style.[fontFamily(font), fontSize(20), backgroundColor(red)]
    />
  | _ => <Text text=message style=Style.[fontFamily(font), fontSize(20)] />
  };
};

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks =>
    (
      hooks,
      List.length(state.messages) > 0
        ? <ScrollView style=messagesStyles>
            ...{
                 List.map(
                   (message: Core.Types.message) =>
                     List.map(
                       item => renderMessage(item, message.kind, state),
                       message.content,
                     ),
                   state.messages,
                 )
                 |> List.flatten
               }
          </ScrollView>
        : React.empty,
    )
  );
