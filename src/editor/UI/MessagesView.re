open Revery_UI;
open Revery.UI.Components;
open Oni_Model;

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
                       item =>
                         <Text
                           text=item
                           style=Style.[
                             fontFamily(state.uiFont.fontFile),
                             fontSize(20),
                           ]
                         />,
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
