open Revery_UI;
open Oni_Model;

let component = React.component("MessagesView");
let messagesStyles =
  Style.[position(`Relative), height(100), left(0), right(0)];
let messageHeight = 20;

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks =>
    (
      hooks,
      List.length(state.messages) > 0
        ? <View style=messagesStyles>
            <FlatList
              rowHeight=messageHeight
              height=100
              width=500
              count={List.length(state.messages)}
              render={index => {
                let message = List.nth(state.messages, index);
                <Text
                  text={message.content}
                  style=Style.[
                    fontFamily(state.uiFont.fontFile),
                    fontSize(20),
                  ]
                />;
              }}
            />
          </View>
        : React.empty,
    )
  );
