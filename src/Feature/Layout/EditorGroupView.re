open Revery.UI;
open Oni_Core;
open Oni_Components;
open Model;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let container = theme => [
    backgroundColor(Colors.Editor.background.from(theme)),
    color(Colors.foreground.from(theme)),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
  ];

  let editorContainer = [flexGrow(1), flexDirection(`Column)];
};

module type ContentModel = {
  type t = Feature_Editor.Editor.t;

  let id: t => int;
  let title: t => string;
  let icon: t => option(IconTheme.IconDefinition.t);
  let isModified: t => bool;
  let isActive: t => bool;

  let render: t => element;
};

let make =
    (
      ~provider as module ContentModel: ContentModel,
      ~showTabs=true,
      ~uiFont,
      ~theme,
      ~isActive,
      ~model: group,
      ~dispatch,
      (),
    ) => {
  let children = {
    let editorContainer =
      switch (
        List.find_opt(
          item => ContentModel.id(item) == model.selected,
          model.items,
        )
      ) {
      | Some(item) => ContentModel.render(item)
      | None => React.empty
      };

    if (showTabs) {
      let tabs =
        <Tabs
          items={model.items |> List.rev}
          style=Style.[
            backgroundColor(
              Colors.EditorGroupHeader.tabsBackground.from(theme),
            ),
          ]
          isSelected=ContentModel.isActive>
          ...{item => {
            <EditorTab
              uiFont
              theme
              title={ContentModel.title(item)}
              isGroupFocused=isActive
              isActive={ContentModel.isActive(item)}
              isModified={ContentModel.isModified(item)}
              icon={ContentModel.icon(item)}
              onClick={() =>
                dispatch(Model.GroupTabClicked(ContentModel.id(item)))
              }
              onClose={() => ()}
              // dispatch(Model.Actions.ViewCloseEditor(tabInfo.editorId))
            />
          }}
        </Tabs>;

      <View style=Styles.editorContainer> tabs editorContainer </View>;
    } else {
      editorContainer;
    };
  };

  let onMouseDown = _ => (); //dispatch(EditorGroupSelected(editorGroup.editorGroupId));

  <View onMouseDown style={Styles.container(theme)}> children </View>;
};
