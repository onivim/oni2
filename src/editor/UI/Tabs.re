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

let toTab = (theme, t: tabInfo) => {
  <Tab
    theme
    title={t.title}
    active={t.active}
    onClick={t.onClick}
    onClose={t.onClose}
  />;
};

let viewStyle = Style.[flexDirection(`Row)];

let createElement = (~children as _, ~theme, ~tabs: list(tabInfo), ()) =>
  component((hooks) => {
    let tabComponents = List.map(toTab(theme), tabs);
    (hooks, <View style=viewStyle> ...tabComponents </View>);
  });
