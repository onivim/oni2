open Oni_Core;
open Revery.UI;
open Revery.UI.Components;
open Oni_Components;

open Exthost.Extension;
open Model;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;
  let container = [flexDirection(`Column), flexGrow(1), overflow(`Hidden)];

  let row = [
    flexDirection(`Row),
    flexGrow(1),
    justifyContent(`Center),
    alignItems(`Center),
  ];

  let column = [
    flexDirection(`Column),
    flexGrow(1),
    justifyContent(`Center),
    alignItems(`Center),
  ];
};

let installButton = (~font, ~extensionId, ~dispatch, ()) => {
  <ItemView.ActionButton
    font
    title="Install"
    backgroundColor=Revery.Colors.green
    color=Revery.Colors.white
    onAction={() =>
      dispatch(Model.InstallExtensionClicked({extensionId: extensionId}))
    }
  />;
};

let row = (~children, ()) => {
  <View style=Styles.row />;
};

let column = (~children, ()) => {
  <View style=Styles.column />;
};

let header = (~font: UiFont.t, ~maybeLogo, ~displayName, ~description, ()) => {
  let logo =
    switch (maybeLogo) {
    | Some(src) => <Image width=128 height=128 src={`File(src)} />
    // TODO: Replace with real logo
    | None => <Container color=Revery.Colors.gray height=128 width=128 />
    };

  <row>
    <column> logo </column>
    <column>
      <row>
        <Text
          fontFamily={font.family}
          fontSize=24.
          fontWeight=Revery.Font.Weight.Bold
          text=displayName
        />
      </row>
      <row>
        <Text fontFamily={font.family} fontSize=18. text=description />
      </row>
    </column>
  </row>;
};

let make = (~model: Model.model, ~theme, ~font: UiFont.t, ~dispatch, ()) => {
  switch (model.selected) {
  | None => <View />
  | Some(selected) =>
    let maybeLogo = selected |> Selected.logo;
    let displayName = selected |> Selected.displayName;
    let description =
      selected |> Selected.description |> Option.value(~default="");

    <View style=Styles.container>
      <header font maybeLogo displayName description />
    </View>;
  };
};
