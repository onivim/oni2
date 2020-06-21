open Revery.UI;
open Oni_Core;
open Utility;
open Oni_Components;
open Model;
open Msg;

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

  let render: t => element;
};

let make =
    (
      ~provider as module ContentModel: ContentModel,
      ~showTabs=true,
      ~uiFont,
      ~theme,
      ~isActive,
      ~model: Group.t,
      ~dispatch,
      (),
    ) => {
  let isSelected = item => ContentModel.id(item) == model.selectedId;
  let children = {
    let editorContainer =
      switch (List.find_opt(isSelected, model.items)) {
      | Some(item) => ContentModel.render(item)
      | None => React.empty
      };

    if (showTabs) {
      let items = model.items |> List.rev;
      let tabs =
        <Tabs
          items
          style=Style.[
            backgroundColor(
              Colors.EditorGroupHeader.tabsBackground.from(theme),
            ),
          ]
          selectedIndex={ListEx.findIndex(isSelected, items)}>
          ...{item => {
            <EditorTab
              uiFont
              theme
              title={ContentModel.title(item)}
              isGroupFocused=isActive
              isActive={isSelected(item)}
              isModified={ContentModel.isModified(item)}
              icon={ContentModel.icon(item)}
              onClick={() =>
                dispatch(GroupTabClicked(ContentModel.id(item)))
              }
              onClose={() =>
                dispatch(EditorCloseButtonClicked(ContentModel.id(item)))
              }
            />
          }}
        </Tabs>;

      <View style=Styles.editorContainer> tabs editorContainer </View>;
    } else {
      editorContainer;
    };
  };

  let onMouseDown = _ => dispatch(GroupSelected(model.id));

  <View onMouseDown style={Styles.container(theme)}> children </View>;
};
