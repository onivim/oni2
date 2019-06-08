/*
 * Tabs.re
 *
 * Container for <Tab /> components
 */

open Revery.UI;
open Rench;

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

let toTab = (theme, mode, uiFont, numberOfTabs, index, t: tabInfo) =>
  <Tab
    theme
    tabPosition={index + 1}
    numberOfTabs
    title={Path.filename(t.title)}
    active={t.active}
    modified={t.modified}
    uiFont
    mode
    onClick={t.onClick}
    onClose={t.onClose}
  />;

let viewStyle = Style.[flexDirection(`Row)];

let createElement =
    (
      ~children as _,
      ~theme,
      ~tabs: list(tabInfo),
      ~mode: Vim.Mode.t,
      ~uiFont,
      (),
    ) =>
  component(hooks => {
    let tabCount = List.length(tabs);
    let tabComponents =
      List.mapi(toTab(theme, mode, uiFont, tabCount), tabs);
    (hooks, <View style=viewStyle> ...tabComponents </View>);
  });
