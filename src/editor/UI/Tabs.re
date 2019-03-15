/*
 * Tabs.re
 *
 * Container for <Tab /> components
 */

open Revery.UI;

open Oni_Core;

let noop = () => ();

type tabInfo = {
  title: string,
  active: bool,
  modified: bool,
  onClick: Tab.tabAction,
  onClose: Tab.tabAction,
};

let component = React.component("Tabs");

let toTab = (theme, mode, uiFont, t: tabInfo) =>
  <Tab
    theme
    title={t.title}
    active={t.active}
    modified={t.modified}
    uiFont
    mode
    onClick={t.onClick}
    onClose={t.onClose}
  />;

let viewStyle = Style.[flexDirection(`Row)];

let createElement =
    (~children as _, ~theme, ~tabs: list(tabInfo), ~mode: Types.Mode.t, ~uiFont, ()) =>
  component(hooks => {
    let tabComponents = List.map(toTab(theme, mode, uiFont), tabs);
    (hooks, <View style=viewStyle> ...tabComponents </View>);
  });
