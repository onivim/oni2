open Oni_Core;
open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Components;

open Exthost.Extension;

module Colors = Feature_Theme.Colors;

module Constants = {
  let itemHeight = 72;
};

module Styles = {
  let text = (~theme) =>
    Style.[
      color(Colors.SideBar.foreground.from(theme)),
      //marginLeft(10),
      marginVertical(2),
      textWrap(TextWrapping.NoWrap),
      textOverflow(`Ellipsis),
      width(250),
    ];
  //overflow(`Hidden),
};

module Extension = {
  let make =
      (
        ~width as itemWidth,
        ~theme,
        ~font: UiFont.t,
        ~extension: Scanner.ScanResult.t,
        (),
      ) => {
    let icon =
      switch (extension.manifest.icon) {
      | None => <Container color=Revery.Colors.darkGray width=32 height=32 />
      | Some(iconPath) => <Image src={`File(iconPath)} width=50 height=50 />
      };

    <View
      style=Style.[
        flexDirection(`Row),
        alignItems(`Center),
        width(itemWidth),
        height(Constants.itemHeight),
        overflow(`Hidden),
        flexGrow(0),
        position(`Relative),
      ]>
      <View
        style=Style.[
          width(64),
          height(Constants.itemHeight),
          flexGrow(0),
          flexDirection(`Column),
          justifyContent(`Center),
          alignItems(`Center),
        ]>
        icon
      </View>
      <View style=Style.[flexDirection(`Column), width(300)]>
        <Text
          style={Styles.text(~theme)}
          fontFamily={font.family}
          fontSize={font.size}
          text={Manifest.getDisplayName(extension.manifest)}
        />
        <Text
          style={Styles.text(~theme)}
          fontFamily={font.family}
          fontSize={font.size}
          text={extension.manifest.author}
        />
        <Text
          style={Styles.text(~theme)}
          fontFamily={font.family}
          fontSize={font.size}
          text={extension.manifest.version}
        />
      </View>
    </View>;
  };
};

type state = {
  installedExpanded: bool,
  bundledExpanded: bool,
  width: int,
};

let default = {installedExpanded: true, bundledExpanded: false, width: 225};

type msg =
  | InstalledTitleClicked
  | BundledTitleClicked
  | WidthChanged(int);

let reduce = (msg, model) =>
  switch (msg) {
  | InstalledTitleClicked => {
      ...model,
      installedExpanded: !model.installedExpanded,
    }
  | BundledTitleClicked => {...model, bundledExpanded: !model.bundledExpanded}
  | WidthChanged(width) => {...model, width}
  };

let%component make = (~model, ~theme, ~font: UiFont.t, ()) => {
  let%hook (state, dispatch) = Hooks.reducer(~initialState=default, reduce);

  let renderItem = (extensions: array(Scanner.ScanResult.t), idx) => {
    <Extension
      width={state.width}
      theme
      font
      extension={Array.get(extensions, idx)}
    />;
  };

  let bundledExtensions =
    Model.getExtensions(~category=Scanner.Bundled, model) |> Array.of_list;

  let userExtensions =
    Model.getExtensions(~category=Scanner.User, model) |> Array.of_list;

  <View
    style=Style.[flexDirection(`Column), flexGrow(1), overflow(`Hidden)]
    onDimensionsChanged={({width, _}) => dispatch(WidthChanged(width))}>
    <Accordion
      title="Installed"
      expanded={state.installedExpanded}
      uiFont=font
      renderItem={renderItem(userExtensions)}
      rowHeight=Constants.itemHeight
      count={Array.length(userExtensions)}
      focused=None
      theme
      onClick={_ => dispatch(InstalledTitleClicked)}
    />
    <Accordion
      title="Bundled"
      expanded={state.bundledExpanded}
      uiFont=font
      renderItem={renderItem(bundledExtensions)}
      rowHeight=Constants.itemHeight
      count={Array.length(bundledExtensions)}
      focused=None
      theme
      onClick={_ => dispatch(BundledTitleClicked)}
    />
  </View>;
};
