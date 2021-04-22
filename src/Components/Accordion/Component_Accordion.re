/*
 * Component_Accordion.re
 */

open Revery.UI;
open Revery.UI.Components;

open Oni_Core;

module Colors = Feature_Theme.Colors;

module Constants = {
  let arrowSize = 15.;
};

module Styles = {
  open Style;

  let container = (~isFocused, ~theme, expanded) => {
    let focusColor =
      isFocused
        ? Colors.focusBorder.from(theme) : Revery.Colors.transparentWhite;
    [
      flexGrow(expanded ? 1 : 0),
      flexDirection(`Column),
      border(~color=focusColor, ~width=1),
    ];
  };

  let chevronContainer = [padding(4), flexGrow(0)];

  let titleBar = theme => [
    flexGrow(0),
    height(25),
    backgroundColor(Colors.SideBarSectionHeader.background.from(theme)),
    flexDirection(`Row),
    alignItems(`Center),
  ];

  let title = (~theme) => [
    color(Colors.SideBarSectionHeader.foreground.from(theme)),
    textWrap(Revery.TextWrapping.NoWrap),
    overflow(`Hidden),
  ];

  let countContainer = (~theme) => [
    width(16),
    height(16),
    borderRadius(8.),
    flexGrow(0),
    backgroundColor(Colors.ActivityBarBadge.background.from(theme)),
    paddingRight(4),
    position(`Relative),
  ];

  let countInner = [
    position(`Absolute),
    width(16),
    height(16),
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    top(1),
    left(0),
  ];
};

let numberToString = num => {
  num >= 10 ? "9+" : string_of_int(num);
};

module Common = {
  let make =
      (
        ~title,
        //~rowHeight,
        ~expanded,
        ~count,
        ~showCount,
        //~renderItem,
        ~isFocused,
        //~focused,
        ~theme,
        ~uiFont: UiFont.t,
        ~onClick,
        ~contents,
        (),
      ) => {
    let list = expanded ? contents : React.empty;

    let fgColor = Colors.SideBarSectionHeader.foreground.from(theme);

    let countForeground = Colors.ActivityBarBadge.foreground.from(theme);

    let countElement =
      showCount
        ? <View style={Styles.countContainer(~theme)}>
            <View style=Styles.countInner>
              <Text
                style=Style.[color(countForeground)]
                fontFamily={uiFont.family}
                fontSize=10.
                fontWeight=Revery.Font.Weight.Bold
                text={numberToString(count)}
              />
            </View>
          </View>
        : React.empty;

    <View style={Styles.container(~isFocused, ~theme, expanded)}>
      <Clickable style={Styles.titleBar(theme)} onClick>
        <View style=Styles.chevronContainer>
          <Codicon
            fontSize=Constants.arrowSize
            color=fgColor
            icon={expanded ? Codicon.chevronDown : Codicon.chevronRight}
          />
        </View>
        <View style=Style.[paddingTop(2), flexGrow(1)]>
          <Text
            style={Styles.title(~theme)}
            fontFamily={uiFont.family}
            fontSize=13.
            fontWeight=Revery.Font.Weight.Bold
            text=title
          />
        </View>
        countElement
      </Clickable>
      list
    </View>;
  };
};

module VimList = {
  let make =
      (
        ~showCount=true,
        ~title,
        ~expanded,
        ~model,
        ~dispatch,
        ~render,
        ~isFocused,
        ~theme,
        ~uiFont: UiFont.t,
        ~onClick,
        (),
      ) => {
    let count = Component_VimList.count(model);
    let contents =
      <Component_VimList.View
        font=uiFont
        isActive=isFocused
        focusedIndex=None
        theme
        model
        dispatch
        render
      />;

    <Common
      showCount
      theme
      title
      expanded
      count
      isFocused
      uiFont
      onClick
      contents
    />;
  };
};

module VimTree = {
  let make =
      (
        ~config,
        ~showCount=true,
        ~focusedIndex=None,
        ~title,
        ~expanded,
        ~model,
        ~dispatch,
        ~render,
        ~isFocused,
        ~theme,
        ~uiFont: UiFont.t,
        ~onClick,
        ~empty=React.empty,
        (),
      ) => {
    let count = Component_VimTree.count(model);
    let contents =
      count > 0
        ? <Component_VimTree.View
            config
            isActive=isFocused
            font=uiFont
            focusedIndex
            theme
            model
            dispatch
            render
          />
        : empty;

    <Common
      showCount
      theme
      title
      expanded
      count
      isFocused
      uiFont
      onClick
      contents
    />;
  };
};
