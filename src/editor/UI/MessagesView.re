open Revery_UI;
open Oni_Model;

let component = React.component("MessagesView");
let messagesStyles = Style.[height(100), left(0), right(0)];

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks =>
    (
      hooks,
      List.length(state.messages) > 0
        ? <View style=messagesStyles>
            ...{List.map(
              (message: Oni_Core.Types.message) =>
                <Text
                  text={message.content}
                  style=Style.[
                    fontFamily(state.uiFont.fontFile),
                    fontSize(20),
                  ]
                />,
              state.messages,
            )}
          </View>
        : React.empty,
    )
  );
