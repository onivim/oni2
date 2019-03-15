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
  modified: bool,
  onClick: Tab.tabAction,
  onClose: Tab.tabAction,
};

let component = React.component("Tabs");

let toTab = (theme, uiFont, t: tabInfo) =>
  <Tab
    theme
    title={t.title}
    active={t.active}
    modified={t.modified}
    uiFont
    onClick={t.onClick}
    onClose={t.onClose}
  />;

let viewStyle = Style.[flexDirection(`Row)];

let createElement = (~children as _, ~theme, ~tabs: list(tabInfo), ~uiFont, ()) =>
  component(hooks => {
    let tabComponents = List.map(toTab(theme, uiFont), tabs);
    (hooks, <View style=viewStyle> ...tabComponents </View>);
  });
