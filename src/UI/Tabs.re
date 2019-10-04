/*
 * Tabs.re
 *
 * Container for <Tab /> components
 */

open Revery.UI;
open Rench;

let noop = () => ();

type tabInfo = {
  editorId: int,
  title: string,
  modified: bool,
};

let component = React.component("Tabs");

let toTab = (theme, mode, uiFont, numberOfTabs, active, activeEditorId, index, t: tabInfo) =>
  <Tab
    theme
    tabPosition={index + 1}
    numberOfTabs
    title={Path.filename(t.title)}
    active={Some(t.editorId) == activeEditorId}
    showHighlight=active
    modified={t.modified}
    uiFont
    mode
    onClick={() => GlobalContext.current().openEditorById(t.editorId)}
    onClose={() => GlobalContext.current().closeEditorById(t.editorId)}
  />;

let measureWidth: option(node) => int = fun
  | Some(outer) => outer#measurements().width
  | None => 0;

let measureOverflow: option(node) => int = fun
  | Some(outer) => {
      let inner = outer#firstChild();
      max(0, inner#measurements().width - outer#measurements().width);
    }
  | None => 0;

let measureChildOffset: int => option(node) => option((int, int)) = index => fun
  | Some(outer) => {
      let rec loop = (i, offset) => fun
        | [] =>
          None

        | [(child: node), ..._] when i == index =>
          Some((offset, child#measurements().width))

        | [(child: node), ...rest] =>
          loop(i + 1, offset + child#measurements().width, rest);

      loop(0, 0, outer#firstChild()#getChildren());
    }
  | None => None;

let findIndex = (predicate, list) => {
  let rec loop = i => fun
    | [] => None
    | [head, ..._] when predicate(head) => Some(i)
    | [_, ...tail] => loop(i + 1, tail);
  loop(0, list);
};

let isPendingRender: option(node) => bool = fun
  | Some(outer) => outer#firstChild()#firstChild()#measurements().width < 0
  | None => true;

let postRenderQueue = ref([]);

let postRender = _ => {
  List.iter(f => f(), postRenderQueue^);
  postRenderQueue := [];
};

let schedulePostRender = f =>
  postRenderQueue := [f, ...postRenderQueue^];

let createElement =
    (
      ~children as _,
      ~theme,
      ~tabs: list(tabInfo),
      ~activeEditorId: option(int),
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

    let activeEditorChanged = () => {
      switch (findIndex(t => Some(t.editorId) == activeEditorId, tabs)) {
        | Some(index) =>
          let f = () => {
            switch (measureChildOffset(index, outerRef)) {
              | Some((offset, width)) =>
                let viewportWidth = measureWidth(outerRef);
                if (offset < actualScrollLeft) {
                  // out of view to the left, so align with left edge
                  setScrollLeft(offset);
                } else if (offset + width > actualScrollLeft + viewportWidth) {
                  // out of view to the right, so align with right edge
                  setScrollLeft(offset - viewportWidth + width);
                }
              | None => ()
            }
          };
          isPendingRender(outerRef) ? schedulePostRender(f) : f();
        | None => ()
      };
      None
    };

    let hooks =
      Hooks.effect(If((!=), activeEditorId), activeEditorChanged, hooks);

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
      List.mapi(toTab(theme, mode, uiFont, tabCount, active, activeEditorId), tabs);

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
        <View
          onDimensionsChanged=postRender
          style=innerStyle>
          ...tabComponents
        </View>
      </View>
    );
  });
