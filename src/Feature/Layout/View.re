module Local = {
  module Layout = Layout;
  module Configuration = Configuration;
};
open Revery;
open Revery.UI;
open Oni_Core;
open Utility;
open Oni_Components;
open Model;
open Msg;

module Colors = Feature_Theme.Colors;

module type ContentModel = {
  type t = Feature_Editor.Editor.t;

  let id: t => int;
  let title: t => string;
  let preview: t => bool;
  let tooltip: t => string;
  let icon: t => option(Oni_Core.IconTheme.IconDefinition.t);
  let isModified: t => bool;

  let render: (~isActive: bool, t) => Revery.UI.element;
};

module Tab = {
  module FontAwesome = Oni_Components.FontAwesome;
  module FontIcon = Oni_Components.FontIcon;
  module Sneakable = Feature_Sneak.View.Sneakable;

  module Theme = Feature_Theme;

  module Constants = {
    include Constants;

    let minWidth = 125;
  };

  module Colors = Theme.Colors.Tab;

  let proportion = factor =>
    float(Constants.minWidth) *. factor |> int_of_float;

  module Styles = {
    open Style;

    let container =
        (~isGroupFocused, ~isActive, ~isHovered, ~isModified, ~theme) => {
      let background = {
        let unhovered =
          switch (isActive, isGroupFocused) {
          | (false, _) => Colors.inactiveBackground
          | (true, false) => Colors.unfocusedActiveBackground
          | (true, true) => Colors.activeBackground
          };

        if (isHovered) {
          let color =
            isGroupFocused
              ? Colors.unfocusedHoverBackground : Colors.hoverBackground;

          color.tryFrom(theme)
          |> Option.value(~default=unhovered.from(theme));
        } else {
          unhovered.from(theme);
        };
      };

      let borderTop = {
        let color =
          if (isActive) {
            background;
          } else {
            let color =
              isGroupFocused
                ? Colors.activeBorderTop : Colors.unfocusedActiveBorderTop;

            color.tryFrom(theme) |> Option.value(~default=background);
          };

        borderTop(~color, ~width=2);
      };

      let borderBottom = {
        let color = {
          let unhovered =
            (
              switch (isActive, isGroupFocused, isModified) {
              | (false, _, false) => Colors.border
              | (false, false, true) => Colors.unfocusedInactiveModifiedBorder
              | (false, true, true) => Colors.inactiveModifiedBorder
              | (true, false, true) => Colors.unfocusedActiveModifiedBorder
              | (true, true, true) => Colors.activeModifiedBorder
              | (true, false, false) => Colors.unfocusedActiveBorder
              | (true, true, false) => Colors.activeBorder
              }
            ).
              tryFrom(
              theme,
            )
            |> Option.value(~default=background);

          if (isHovered) {
            let color =
              isGroupFocused
                ? Colors.unfocusedHoverBorder : Colors.hoverBorder;

            color.tryFrom(theme) |> Option.value(~default=unhovered);
          } else {
            unhovered;
          };
        };

        borderBottom(~color, ~width=1);
      };

      [
        overflow(`Hidden),
        paddingHorizontal(5),
        backgroundColor(background),
        borderTop,
        borderBottom,
        height(Constants.tabHeight),
        minWidth(Constants.minWidth),
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
      ];
    };

    let text = (~isGroupFocused, ~isActive, ~theme) => {
      let foreground =
        switch (isActive, isGroupFocused) {
        | (false, false) => Colors.unfocusedInactiveForeground
        | (false, true) => Colors.inactiveForeground
        | (true, false) => Colors.unfocusedActiveForeground
        | (true, true) => Colors.activeForeground
        };
      [
        width(proportion(0.80) - 10),
        textOverflow(`Ellipsis),
        color(foreground.from(theme)),
        justifyContent(`Center),
        alignItems(`Center),
      ];
    };

    let icon = [
      width(32),
      height(Constants.tabHeight),
      alignItems(`Center),
      justifyContent(`Center),
    ];
  };

  let%component make =
                (
                  ~title,
                  ~tooltip,
                  ~isGroupFocused,
                  ~isActive,
                  ~isModified,
                  ~onClick,
                  ~onDoubleClick=() => (),
                  ~onClose,
                  ~theme: ColorTheme.Colors.t,
                  ~uiFont: UiFont.t,
                  ~icon,
                  ~isPreview: bool=false,
                  (),
                ) => {
    let%hook (isHovered, setHovered) = Hooks.state(false);

    let fileIconView =
      switch (icon) {
      | Some(icon: IconTheme.IconDefinition.t) =>
        <FontIcon
          fontFamily={Revery.Font.Family.fromFile("seti.ttf")}
          icon={icon.fontCharacter}
          color={icon.fontColor}
          /* TODO: Use 'weight' value from IconTheme font */
          fontSize={uiFont.size *. 1.5}
        />
      | None => React.empty
      };

    let onAnyClick = (evt: NodeEvents.mouseButtonEventParams) => {
      switch (evt.button) {
      | Revery.MouseButton.BUTTON_MIDDLE => onClose()
      | Revery.MouseButton.BUTTON_LEFT => onClick()
      | _ => ()
      };
    };

    <View
      onMouseOver={_ => setHovered(_ => true)}
      onMouseOut={_ => setHovered(_ => false)}
      style={Styles.container(
        ~isGroupFocused,
        ~isActive,
        ~isHovered,
        ~isModified,
        ~theme,
      )}>
      <Sneakable
        sneakId=title
        onSneak=onClick
        onAnyClick
        onDoubleClick
        style=Style.[
          width(proportion(0.80)),
          flexGrow(1),
          flexDirection(`Row),
          alignItems(`Center),
          justifyContent(`Center),
        ]>
        <View style=Styles.icon> fileIconView </View>
        <Tooltip text=tooltip>
          <Text
            style={Styles.text(~isGroupFocused, ~isActive, ~theme)}
            fontFamily={uiFont.family}
            italic=isPreview
            fontSize={uiFont.size}
            text=title
          />
        </Tooltip>
      </Sneakable>
      <Sneakable onClick=onClose style=Styles.icon sneakId={title ++ ".close"}>
        <FontIcon
          icon={isModified ? FontAwesome.circle : FontAwesome.times}
          color={
            isActive
              ? Colors.activeForeground.from(theme)
              : Colors.inactiveForeground.from(theme)
          }
          fontSize={isModified ? 10. : 12.}
        />
      </Sneakable>
    </View>;
  };
};

module EditorGroupView = {
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
    let preview: t => bool;
    let tooltip: t => string;
    let icon: t => option(IconTheme.IconDefinition.t);
    let isModified: t => bool;

    let render: (~isActive: bool, t) => element;
  };

  let make =
      (
        ~provider as module ContentModel: ContentModel,
        ~config,
        ~showTabs,
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
        switch (List.find_opt(isSelected, model.editors)) {
        | Some(item) => ContentModel.render(~isActive, item)
        | None => React.empty
        };

      if (showTabs && !Local.Configuration.singleTabMode.get(config)) {
        let editors = model.editors |> List.rev;
        let tabs =
          <Tabs
            items=editors
            style=Style.[
              backgroundColor(
                Colors.EditorGroupHeader.tabsBackground.from(theme),
              ),
            ]
            selectedIndex={ListEx.findIndex(isSelected, editors)}>
            ...{(~isSelected, ~index as _, item) => {
              <Tab
                uiFont
                theme
                title={ContentModel.title(item)}
                tooltip={ContentModel.tooltip({item})}
                isGroupFocused=isActive
                isActive=isSelected
                isModified={ContentModel.isModified(item)}
                isPreview={ContentModel.preview(item)}
                icon={ContentModel.icon(item)}
                onClick={() =>
                  dispatch(
                    EditorTabClicked({
                      groupId: model.id,
                      editorId: ContentModel.id(item),
                    }),
                  )
                }
                onDoubleClick={() =>
                  dispatch(
                    EditorTabDoubleClicked({
                      groupId: model.id,
                      editorId: ContentModel.id(item),
                    }),
                  )
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
};

module Layout = {
  module Constants = {
    let handleSize = 10;
  };

  module Styles = {
    open Style;

    let container = [flexGrow(1), flexDirection(`Row)];

    let verticalHandle = (node: Positioned.t(_)) => [
      cursor(MouseCursors.horizontalResize),
      position(`Absolute),
      left(node.meta.x + node.meta.width - Constants.handleSize / 2),
      top(node.meta.y),
      width(Constants.handleSize),
      height(node.meta.height),
    ];

    let horizontalHandle = (node: Positioned.t(_)) => [
      cursor(MouseCursors.verticalResize),
      position(`Absolute),
      left(node.meta.x),
      top(node.meta.y + node.meta.height - Constants.handleSize / 2),
      width(node.meta.width),
      height(Constants.handleSize),
    ];
  };

  let component = React.Expert.component("handleView");
  let handleView =
      (~direction, ~node: Positioned.t(_), ~onDrag, ~onDragComplete, ()) =>
    component(hooks => {
      let ((captureMouse, _state), hooks) =
        Hooks.mouseCapture(
          ~onMouseMove=
            ((originX, originY), evt) => {
              let delta =
                switch (direction) {
                | `Vertical => evt.mouseX -. originX
                | `Horizontal => evt.mouseY -. originY
                };

              onDrag(delta);
              Some((originX, originY));
            },
          ~onMouseUp=
            (_, _) => {
              onDragComplete();
              None;
            },
          (),
          hooks,
        );

      let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
        captureMouse((evt.mouseX, evt.mouseY));
      };

      (
        <View
          onMouseDown
          style={
            direction == `Vertical
              ? Styles.verticalHandle(node) : Styles.horizontalHandle(node)
          }
        />,
        hooks,
      );
    });

  let rec nodeView =
          (
            ~theme,
            ~path=[],
            ~node: Positioned.t(_),
            ~renderWindow,
            ~dispatch,
            (),
          ) => {
    switch (node.kind) {
    | `Split(direction, children) =>
      let parent = node;

      let rec loop = (index, children) => {
        let path = [index, ...path];

        switch (children) {
        | [] => []
        | [node] => [<nodeView theme path node renderWindow dispatch />]

        | [node, ...[_, ..._] as rest] =>
          let onDrag = delta => {
            let total =
              direction == `Vertical ? parent.meta.width : parent.meta.height;
            dispatch(
              SplitDragged({
                path: List.rev(path),
                delta: delta /. float(total) // normalized
              }),
            );
          };

          let onDragComplete = () => dispatch(DragComplete);

          [
            <nodeView theme path node renderWindow dispatch />,
            <handleView direction node onDrag onDragComplete />,
            ...loop(index + 1, rest),
          ];
        };
      };

      loop(0, children) |> React.listToElement;

    | `Window(id) =>
      <View
        style=Style.[
          position(`Absolute),
          left(node.meta.x),
          top(node.meta.y),
          width(node.meta.width),
          height(node.meta.height),
        ]>
        {renderWindow(id)}
      </View>
    };
  };

  let component = React.Expert.component("Feature_Layout.View");
  let make =
      (
        ~provider,
        ~model as layout,
        ~config,
        ~isFocused,
        ~showTabs,
        ~uiFont,
        ~theme,
        ~dispatch,
        (),
      ) =>
    component(hooks => {
      let ((maybeDimensions, setDimensions), hooks) =
        Hooks.state(None, hooks);

      let tree = activeTree(layout);

      let children =
        switch (maybeDimensions) {
        | Some((width, height)) =>
          let positioned = Positioned.fromLayout(0, 0, width, height, tree);

          let renderWindow = id =>
            switch (groupById(id, layout)) {
            | Some(group) =>
              <EditorGroupView
                provider
                uiFont
                config
                showTabs
                isActive={group.id == layout.activeGroupId && isFocused}
                theme
                model=group
                dispatch
              />
            | None => React.empty
            };

          <nodeView theme node=positioned renderWindow dispatch />;

        | None => React.empty
        };

      (
        <View
          onDimensionsChanged={dim =>
            setDimensions(_ => Some((dim.width, dim.height)))
          }
          style=Styles.container>
          children
        </View>,
        hooks,
      );
    });
};

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

let make =
    (
      ~children as provider,
      ~model,
      ~isFocused,
      ~isZenMode,
      ~showTabs,
      ~config,
      ~uiFont,
      ~theme,
      ~dispatch,
      (),
    ) => {
  let activeLayout =
    <Layout
      provider
      model={activeLayout(model)}
      config
      isFocused
      showTabs
      uiFont
      theme
      dispatch
    />;

  let showLayoutTabs =
    switch (Local.Configuration.showLayoutTabs.get(config)) {
    | `always => true
    | `smart when List.length(model.layouts) > 1 => true
    | `smart
    | `off => false
    };

  if (showLayoutTabs && !isZenMode) {
    module ContentModel = (val provider);

    let tabs =
      <Tabs
        items={model.layouts}
        style=Style.[
          backgroundColor(
            Colors.EditorGroupHeader.tabsBackground.from(theme),
          ),
        ]
        selectedIndex={Some(model.activeLayoutIndex)}>
        ...{(~isSelected, ~index, layout) => {
          let groupCount = List.length(layout.groups);
          let activeGroup =
            List.find(
              (group: Group.t) => group.id == layout.activeGroupId,
              layout.groups,
            );
          let activeEditor = Group.selected(activeGroup);
          let title =
            Printf.sprintf(
              "%i - %s",
              groupCount,
              ContentModel.title(activeEditor),
            );
          let isModified =
            List.exists(ContentModel.isModified, activeGroup.editors);

          <Tab
            uiFont
            theme
            title
            tooltip=title
            isActive=isSelected
            isGroupFocused=true
            isModified
            icon={ContentModel.icon(activeEditor)}
            onClick={() => dispatch(LayoutTabClicked(index))}
            onClose={() => dispatch(LayoutCloseButtonClicked(index))}
          />;
        }}
      </Tabs>;

    switch (Local.Configuration.layoutTabPosition.get(config)) {
    | `top => <View style=Styles.editorContainer> tabs activeLayout </View>
    | `bottom => <View style=Styles.editorContainer> activeLayout tabs </View>
    };
  } else {
    activeLayout;
  };
};
