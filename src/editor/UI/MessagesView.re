open Revery_UI;
open Oni_Model;

let component = React.component("MessagesView");

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks =>
    (
      hooks,
      List.length(state.messages) > 0
        ? <View style=Style.[flexGrow(0)]>
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
