open Oni_Core;
open Oni_Model;

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Notification = {
  open Notification;

  module Styles = {
    open Style;

    let container = [
      flexDirection(`Row),
      alignItems(`Center),
      paddingHorizontal(10),
      paddingVertical(5),
    ];

    let text = (font: UiFont.t) => [
      fontFamily(font.fontFile),
      fontSize(11),
      textWrap(TextWrapping.NoWrap),
      marginLeft(6),
    ];
  };

  let colorFor = (item, ~theme: Theme.t) =>
    switch (item.kind) {
    | Success => theme.notificationSuccessBackground
    | Warning => theme.notificationWarningBackground
    | Error => theme.notificationErrorBackground
    | Info => theme.notificationInfoBackground
    };

  let iconFor = item =>
    switch (item.kind) {
    | Success => FontAwesome.checkCircle
    | Warning => FontAwesome.exclamationTriangle
    | Error => FontAwesome.exclamationCircle
    | Info => FontAwesome.infoCircle
    };

  let make = (~item, ~theme, ~font, ()) => {
    let icon = () =>
      <FontIcon
        icon={iconFor(item)}
        fontSize=16
        backgroundColor=Colors.transparentWhite
        color={colorFor(item, ~theme)}
      />;

    <View style=Styles.container>
      <icon />
      <Text style={Styles.text(font)} text={item.message} />
    </View>;
  };
};

module Styles = {
  open Style;

  let pane = [flexGrow(1), flexDirection(`Row)];

  let noResultsContainer = [
    flexGrow(1),
    alignItems(`Center),
    justifyContent(`Center),
  ];

  let container = [
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
  ];

  let title = (~theme: Theme.t, ~font: UiFont.t) => [
    fontFamily(font.fontFile),
    fontSize(font.fontSize),
    color(theme.editorForeground),
    margin(8),
  ];
};

let make = (~state: State.t, ()) => {
  let {theme, uiFont as font, _}: State.t = state;

  let items =
    state.notifications |> List.map(item => <Notification item theme font />);

  let innerElement =
    if (items == []) {
      <View style=Styles.noResultsContainer>
        <Text
          style={Styles.title(~theme, ~font=uiFont)}
          text="No notifications, yet!"
        />
      </View>;
    } else {
      <ScrollView style=Styles.container>
        {items |> React.listToElement}
      </ScrollView>;
    };

  <View style=Styles.pane> innerElement </View>;
};
