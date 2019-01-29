/*
 * Tabs.re
 *
 * Container for <Tab /> components
 */

open Revery.UI;

let noop = () => ();

type tabInfo = {
  title: string,
  active: bool,
  onClick: Tab.tabAction,
  onClose: Tab.tabAction,
};

let component = React.component("Tabs");

let toTab = (t: tabInfo) => {
  <Tab
    title={t.title}
    active={t.active}
    onClick={t.onClick}
    onClose={t.onClose}
  />;
};

let viewStyle = Style.[flexDirection(`Row)];

let make = (~tabs: list(tabInfo), ()) =>
  component((_slots: React.Hooks.empty) => {
    let tabComponents = List.map(toTab, tabs);
    <View style=viewStyle> ...tabComponents </View>;
  });

let createElement = (~children as _, ~tabs, ()) =>
  React.element(make(~tabs, ()));
