/*
 * Tabs.re
 *
 * Container for <Tab /> components
 */

open Revery.UI;
open Rench;

let noop = () => ();

type tabInfo = {
  title: string,
  active: bool,
  modified: bool,
  onClick: Tab.tabAction,
  onClose: Tab.tabAction,
};

let component = React.component("Tabs");

let toTab = (theme, mode, uiFont, numberOfTabs, active, index, t: tabInfo) =>
  <Tab
    theme
    tabPosition={index + 1}
    numberOfTabs
    title={Path.filename(t.title)}
    active={t.active}
    showHighlight=active
    modified={t.modified}
    uiFont
    mode
    onClick={t.onClick}
    onClose={t.onClose}
  />;

let measureOverflow: option(node) => int = fun
  | Some(outer) => {
      let inner = outer#firstChild();
      max(0, inner#measurements().width - outer#measurements().width);
    }
  | _ => 0

let createElement =
    (
      ~children as _,
      ~theme,
      ~tabs: list(tabInfo),
      ~mode: Vim.Mode.t,
      ~uiFont,
      ~active,
      (),
    ) =>
  component(hooks => {
    let (actualScrollLeft, setScrollLeft, hooks) =
      Hooks.state(0, hooks);
    let (outerRef: option(Revery_UI.node), setOuterRef, hooks) =
      Hooks.state(None, hooks);

    let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      let maxOffset =
        measureOverflow(outerRef);

      let newScrollLeft =
        actualScrollLeft - int_of_float(wheelEvent.deltaY *. 25.);

      let clampedScrollLeft =
        newScrollLeft
        |> max(0)
        |> min(maxOffset);

      setScrollLeft(clampedScrollLeft);
    };

    let tabCount = List.length(tabs);
    let tabComponents =
      List.mapi(toTab(theme, mode, uiFont, tabCount, active), tabs);

    let outerStyle =
      Style.[
        flexDirection(`Row),
        overflow(`Scroll)
      ];
    
    let innerViewTransform =
      Transform.[
        TranslateX((-1.) *. float_of_int(actualScrollLeft)),
      ];

    let innerStyle =
      Style.[
        flexDirection(`Row),
        transform(innerViewTransform),
      ];
      
    (
      hooks,
      <View
        onMouseWheel=scroll
        ref={r => setOuterRef(Some(r))}
        style=outerStyle>
        <View style=innerStyle>
          ...tabComponents
        </View>
      </View>
    );
  });
