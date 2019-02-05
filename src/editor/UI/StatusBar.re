/*
 * Tabs.re
 *
 * Container for <Tab /> components
 */

open Revery.Core;
open Revery.UI;

open Oni_Core;

let noop = () => ();

type tabInfo = {
  title: string,
  active: bool,
  onClick: Tab.tabAction,
  onClose: Tab.tabAction,
};

let component = React.component("StatusBar");

let textStyle =
  Style.[color(Color.hex("#9da5b4")), fontFamily("Inter-UI-Regular.ttf"), fontSize(14)];

let toTab = (t: tabInfo) => {
  <Tab
    title={t.title}
    active={t.active}
    onClick={t.onClick}
    onClose={t.onClose}
  />;
};

let viewStyle = Style.[flexDirection(`Row), justifyContent(`Center), alignItems(`Center)];

let make = (~mode: State.Mode.t, ()) =>
  component((_slots: React.Hooks.empty) => {
    <View style=viewStyle><Text style=textStyle text={State.Mode.show(mode)} /></View>
  });

let createElement = (~children as _, ~mode, ()) =>
  React.element(make(~mode, ()));
