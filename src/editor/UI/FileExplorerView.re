open Oni_Model;
open Revery_UI;
open Revery.UI.Components;

module Core = Oni_Core;

let component = React.component("FileExplorer");

let dummyFiles =
  Tree.(
    Node(
      {data: "root", id: 1, status: Open},
      [
        Node(
          {data: "subfolder 1", id: 2, status: Open},
          [
            Node(
              {data: "subdirectory 1", id: 3, status: Closed},
              [Empty, Empty],
            ),
          ],
        ),
        Node(
          {data: "home", id: 4, status: Open},
          [
            Node({status: Closed, id: 5, data: "downloads"}, [Empty, Empty]),
            Node(
              {data: "desktop", id: 6, status: Open},
              [
                Node(
                  {status: Open, id: 7, data: "subfolder 2"},
                  [
                    Node(
                      {status: Open, id: 8, data: "pictures"},
                      [
                        Node({status: Closed, id: 12, data: "Images"}, []),
                        Node(
                          {status: Closed, id: 10, data: "holiday 2018"},
                          [],
                        ),
                        Node(
                          {status: Closed, id: 11, data: "Graduation 2017"},
                          [],
                        ),
                      ],
                    ),
                    Empty,
                  ],
                ),
                Node(
                  {data: "subfolder 3", id: 9, status: Closed},
                  [Empty, Empty],
                ),
              ],
            ),
          ],
        ),
      ],
    )
  );

let itemStyles = Style.[flexDirection(`Row), marginVertical(5)];
let itemRenderer =
    (~indent, font, itemFontSize, {data, status, _}: Tree.content(string)) => {
  open Tree;
  let isOpen =
    switch (status) {
    | Open => true
    | Closed => false
    };
  open Style;
  let textStyles = [fontSize(itemFontSize), color(Revery.Colors.white)];
  let indentStr = String.make(indent * 2, ' ');
  let arrow = isOpen ? {||} : {||};
  <Clickable>
    <View style=itemStyles>
      <Text
        text={indentStr ++ arrow ++ " "}
        style=[fontFamily("FontAwesome5FreeSolid.otf"), ...textStyles]
      />
      <Text text=data style=[fontFamily(font), ...textStyles] />
    </View>
  </Clickable>;
};

let treeContainer = (_theme: Core.Theme.t) => Style.[padding(20)];
let title = (theme: Core.Theme.t, font) =>
  Style.[padding(5), fontSize(14), fontFamily(font)];

let createElement = (~children, ~state: State.t, ()) =>
  component(hooks => {
    let font = state.uiFont.fontFile;
    let {State.theme} = state;
    let itemFontSize = 12;

    (
      hooks,
      <View>
        <View
          style=Style.[
            flexDirection(`Row),
            justifyContent(`Center),
            alignItems(`Center),
            backgroundColor(theme.colors.editorMenuItemSelected),
          ]>
          <Text text="File Explorer" style={title(theme, font)} />
        </View>
        <View style={treeContainer(theme)}>
          <Tree
            tree=dummyFiles
            nodeRenderer={itemRenderer(font, itemFontSize)}
          />
        </View>
      </View>,
    );
  });
