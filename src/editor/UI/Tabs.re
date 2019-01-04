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

include (
          val component((render, ~tabs:list(tabInfo), ~children, ()) =>
                render(
                  () => {

                    let toTab = (t: tabInfo) => {
                        <Tab title={t.title} active={t.active} onClick={t.onClick} onClose={t.onClose} />   
                    };
                    let tabComponents = List.map(toTab, tabs);

                    <view
                      style={Style.make(
                        ~flexDirection=LayoutTypes.Row,
                        (),
                      )}>
                      ...tabComponents
                    </view>;
                  },
                  ~children,
                )
              )
        );
