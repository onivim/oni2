open Oni_Core;
open Revery;
open Revery.UI;

open Revery.UI.Components;

module Colors = Feature_Theme.Colors;
module Sneakable = Feature_Sneak.View.Sneakable;

module Styles = {
  open Style;
  let text = (~theme) => [
    color(Colors.SideBar.foreground.from(theme)),
    marginLeft(10),
    marginVertical(2),
    textWrap(TextWrapping.NoWrap),
    textOverflow(`Ellipsis),
  ];
  let input = [flexGrow(1), margin(12)];

  let button = (~backgroundColor) => [
    Style.backgroundColor(backgroundColor),
    borderRadius(4.),
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
  ];
  let innerButton = [
    margin(2)
  ];
};

module ActionButton = {

  let make = (~font: UiFont.t, ~title: string, ~backgroundColor, ~color, ~onAction, ()) => {
    <Sneakable
      style=Styles.button(backgroundColor)
      onClick=onAction
    >
      <View style=Styles.innerButton>
      <Text fontFamily={font.family} fontSize={11.} text=title />
      </View>
    </Sneakable>
  };
  
//          <ActionButton font title="A" backgroundColor=Revery.Colors.red color=Revery.Colors.white onAction={() => {
//            prerr_endline("Clicked!")
//          }}/>
};

let make =
    (~iconPath, ~theme, ~displayName, ~author, ~version, ~font: UiFont.t, ()) => {
  let icon =
    switch (iconPath) {
    | None => <Container color=Revery.Colors.darkGray width=32 height=32 />
    | Some(iconPath) => <Image src={`File(iconPath)} width=32 height=32 />
    };

  <View
    style=Style.[
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
      flexGrow(1),
      height(45),
    ]>
    icon
    <View style=Style.[flexDirection(`Column), flexGrow(1)]>
      <Text
        style={Styles.text(~theme)}
        fontFamily={font.family}
        fontSize={font.size}
        text=displayName
      />
      <View style=Style.[flexDirection(`Row), flexGrow(1)]>
        <View
          style=Style.[
            flexDirection(`Column),
            flexGrow(1),
            overflow(`Hidden),
          ]>
          <Text
            style={Styles.text(~theme)}
            fontFamily={font.family}
            fontSize={font.size}
            text=author
          />
        </View>
        <View
          style=Style.[
            flexDirection(`Column),
            flexGrow(1),
            overflow(`Hidden),
          ]>
          <Text
            style={Styles.text(~theme)}
            fontFamily={font.family}
            fontSize={font.size}
            text=version
          />
        </View>
        <View
          style=Style.[
            flexDirection(`Column),
            flexGrow(0),
            overflow(`Hidden),
          ]>
        </View>
      </View>
    </View>
  </View>;
};
