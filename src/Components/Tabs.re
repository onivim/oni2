open Revery.UI;

let measureWidth: option(node) => int =
  fun
  | Some(outer) => outer#measurements().width
  | None => 0;

let measureOverflow: option(node) => int =
  fun
  | Some(outer) => {
      let inner = outer#firstChild();
      max(0, inner#measurements().width - outer#measurements().width);
    }
  | None => 0;

let measureChildOffset: (int, option(node)) => option((int, int)) =
  index =>
    fun
    | Some(outer) => {
        let rec loop = (i, offset) => (
          fun
          | [] => None

          | [child, ..._] when i == index => {
              let childNode: node = child;
              Some((offset, childNode#measurements().width));
            }

          | [child, ...rest] => {
              let childNode: node = child;
              loop(i + 1, offset + childNode#measurements().width, rest);
            }
        );

        loop(0, 0, outer#firstChild()#getChildren());
      }
    | None => None;

let isPendingRender: option(node) => bool =
  fun
  | Some(outer) => outer#firstChild()#firstChild()#measurements().width < 0
  | None => true;

let postRenderQueue = ref([]);

let postRender = _ => {
  List.iter(f => f(), postRenderQueue^);
  postRenderQueue := [];
};

let schedulePostRender = f => postRenderQueue := [f, ...postRenderQueue^];

let component = React.Expert.component("Tabs");
let make =
    (
      ~children as render: (~isSelected: bool, ~index: int, 'a) => element,
      ~items: list('a),
      ~selectedIndex: option(int),
      ~style,
      (),
    ) =>
  component(hooks => {
    let ((actualScrollLeft, setScrollLeft), hooks) = Hooks.state(0, hooks);
    let ((outerRef: option(Revery_UI.node), setOuterRef), hooks) =
      Hooks.state(None, hooks);

    let selectedChanged = () => {
      switch (selectedIndex) {
      | Some(index) =>
        let f = () => {
          switch (measureChildOffset(index, outerRef)) {
          | Some((offset, width)) =>
            let viewportWidth = measureWidth(outerRef);
            if (offset < actualScrollLeft) {
              // out of view to the left, so align with left edge
              setScrollLeft(_ =>
                offset
              );
            } else if (offset + width > actualScrollLeft + viewportWidth) {
              // out of view to the right, so align with right edge
              setScrollLeft(_ =>
                offset - viewportWidth + width
              );
            };
          | None => ()
          };
        };
        isPendingRender(outerRef) ? schedulePostRender(f) : f();
      | None => ()
      };
      None;
    };

    let ((), hooks) =
      Hooks.effect(If((!=), selectedIndex), selectedChanged, hooks);

    let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) => {
      let maxOffset = measureOverflow(outerRef);

      setScrollLeft(actualScrollLeft => {
        let newScrollLeft =
          actualScrollLeft
          - int_of_float((-. wheelEvent.deltaX +. wheelEvent.deltaY) *. 25.);

        newScrollLeft |> max(0) |> min(maxOffset);
      });
    };

    let outerStyle =
      Style.[flexDirection(`Row), overflow(`Scroll), ...style];

    let innerViewTransform =
      Transform.[TranslateX((-1.) *. float(actualScrollLeft))];

    let innerStyle =
      Style.[flexDirection(`Row), transform(innerViewTransform)];

    (
      <View
        onMouseWheel=scroll
        ref={r => setOuterRef(_ => Some(r))}
        style=outerStyle>
        <View onDimensionsChanged=postRender style=innerStyle>
          {List.mapi(
             index =>
               render(~isSelected=Some(index) == selectedIndex, ~index),
             items,
           )
           |> React.listToElement}
        </View>
      </View>,
      hooks,
    );
  });
