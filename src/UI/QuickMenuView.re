
open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Oni_Model;

module Constants = {
  let menuWidth = 400;
  let menuHeight = 320;
};

type items =
  | Array(array(Actions.menuCommand))
  | Job(MenuJob.t);

let component = React.component("Menu");

let loseFocusOnClose = isOpen =>
  /**
   TODO: revery-ui/revery#412 if the menu is hidden abruptly the element is not automatically unfocused
   as revery is unaware the element is no longer in focus
 */
  (
    switch (Focus.focused, isOpen) {
    | ({contents: Some(_)}, false) => Focus.loseFocus()
    | (_, _) => ()
    }
  );


let getLabel = (command: Actions.menuCommand) => {
  switch (command.category) {
  | Some(v) => v ++ ": " ++ command.name
  | None => command.name
  };
};

module Styles = {
  let container = (theme: Theme.t) =>
    Style.[
      backgroundColor(theme.editorMenuBackground),
      color(theme.editorMenuForeground),
    ];

  let input = font =>
    Style.[
      border(~width=2, ~color=Color.rgba(0., 0., 0., 0.1)),
      backgroundColor(Color.rgba(0., 0., 0., 0.3)),
      width(Constants.menuWidth - 10),
      color(Colors.white),
      fontFamily(font),
    ];

  let menuItem =
    Style.[
      fontSize(14),
      width(Constants.menuWidth - 50),
      cursor(Revery.MouseCursors.pointer),
    ];
};

let createElement =
    (
      ~children as _,
      ~font: Types.UiFont.t,
      ~theme: Theme.t,
      ~configuration: Configuration.t,
      ~autofocus: bool=true,
      ~prefix: option(string)=?,
      ~placeholder: string="type here to search the menu",
      ~loadingAnimation: Animation.t=Animation.create(~isActive=false, ()),
      ~text: string,
      ~cursorPosition: int,
      ~items: items,
      ~selected: option(int),
      ~onInput: (string, int) => unit,
      ~onSelectedChange: int => unit,
      ~onSelect: int => unit,
      (),
    ) =>
  component(hooks => {
    let handleKeyDown = (event: NodeEvents.keyEventParams) =>
      switch (event) {
      | {key: Revery.Key.KEY_DOWN, _} =>
        GlobalContext.current().dispatch(MenuNextItem)
      | {key: Revery.Key.KEY_UP, _} =>
        GlobalContext.current().dispatch(MenuPreviousItem)
      | _ => ()
      };

    let (items, jobProgress) =
      switch (items) {
        | Array(items) =>
          (items, 1.)
        | Job(job) =>
          (Job.getCompletedWork(job).uiFiltered, Job.getProgress(job))
      };

    let time = Time.getTime() |> Time.to_float_seconds;

    let loadingOpacityAnimation = Animation.getValue(loadingAnimation);
    let loadingSpinner =
      Animation.isActive(loadingAnimation)
        ? <View style=Style.[height(40), width(Constants.menuWidth)]>
            <Center>
              <View
                style=Style.[
                  transform(
                    Transform.[RotateY(Math.Angle.Radians(time *. 2.))],
                  ),
                ]>
                <Opacity opacity=loadingOpacityAnimation>
                  <Container
                    width=10
                    height=10
                    color={theme.oniNormalModeBackground}
                  />
                </Opacity>
              </View>
            </Center>
          </View>
        : <Opacity opacity=0.3>
            <View style=Style.[height(2), width(Constants.menuWidth)]>
              <View
                style=Style.[
                  height(2),
                  width(
                    1
                    + (
                      int_of_float(float_of_int(Constants.menuWidth) *. jobProgress)
                      - 1
                    ),
                  ),
                  backgroundColor(theme.oniNormalModeBackground),
                ]
              />
            </View>
          </Opacity>;

    (
      hooks,
      <AllowPointer>
        <OniBoxShadow configuration theme>
          <View style={Styles.container(theme)}>
            <View style=Style.[width(Constants.menuWidth), padding(5)]>
              <OniInput
                autofocus
                placeholder
                ?prefix
                cursorColor=Colors.white
                style={Styles.input(font.fontFile)}
                onChange=onInput
                onKeyDown=handleKeyDown
                text
                cursorPosition
              />
            </View>
            <View>
              <FlatList
                rowHeight=40
                height=Constants.menuHeight
                width=Constants.menuWidth
                count={Array.length(items)}
                selected
                render={index => {
                  let item = items[index];
                  <MenuItem
                    onClick={() => onSelect(index)}
                    theme
                    style=Styles.menuItem
                    label=getLabel(item)
                    icon=item.icon
                    onMouseOver={() => onSelectedChange(index)}
                    selected={Some(index) == selected}
                  />;
                }}
              />
              loadingSpinner
            </View>
          </View>
        </OniBoxShadow>
      </AllowPointer>
    );
  });
