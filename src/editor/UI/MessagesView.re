open Revery_UI;
open Oni_Model;
open Revery.UI.Components;
open Oni_Core.Types;

let component = React.component("MessagesView");
let messagesStyles = Style.[height(150), left(0), right(0), padding(10)];
let messageFontSize = 15;

let renderMessage = (message, messageKind: messageKind, font) => {
  open Style;
  open Revery.Colors;

  let baseStyles = [fontFamily(font), fontSize(messageFontSize)];
  let styles =
    switch (messageKind) {
    | ReturnPrompt => [color(gold), ...baseStyles]
    | Echoerr
    | Emsg => [color(red), ...baseStyles]
    | _ => baseStyles
    };
  <Text text=message style=styles />;
};

let header = bg => Style.[padding(8), backgroundColor(bg)];
let headerText = font =>
  Style.[fontFamily(font), fontSize(messageFontSize)];

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks => {
    let font = state.uiFont.fontFile;
    let headerBgColor = state.theme.colors.editorWhitespaceForeground;
    (
      hooks,
      List.length(state.messages) > 0
        ? <View>
            <View style={header(headerBgColor)}>
              <Text text="Messages" style={headerText(font)} />
            </View>
            <ScrollView style=messagesStyles>
              ...{
                   List.map(
                     ({kind, content}) =>
                       List.map(
                         item => renderMessage(item, kind, font),
                         content,
                       ),
                     state.messages,
                   )
                   |> List.flatten
                 }
            </ScrollView>
          </View>
        : React.empty,
    );
  });
