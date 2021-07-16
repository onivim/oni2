open Oni_Core;
open Utility;

module Icon = {
  type t =
    | Seti(IconTheme.IconDefinition.t)
    | Codicon({
        fontSize: option(float),
        color: option(ColorTheme.Schema.definition),
        icon: int,
      });

  let seti = iconDefinition => Seti(iconDefinition);

  let codicon = (~fontSize=?, ~color=?, icon) =>
    Codicon({fontSize, color, icon});
};

module Renderer = {
  open Revery;
  open Revery.UI;
  type t('item) =
    (
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~text: string,
      ~highlights: list((int, int)),
      'item
    ) =>
    Revery.UI.element;

  module Constants = {
    let iconSize = 20.;
  };

  module Styles = {
    open Style;
    module Colors = Feature_Theme.Colors;

    let label = (~theme, ~highlighted) => [
      textOverflow(`Ellipsis),
      color(
        highlighted
          ? Colors.Oni.normalModeBackground.from(theme)
          : Colors.Menu.foreground.from(theme),
      ),
      textWrap(TextWrapping.NoWrap),
    ];

    let icon = fg =>
      Style.[
        color(fg),
        width(int_of_float(Constants.iconSize *. 0.75)),
        height(int_of_float(Constants.iconSize *. 0.85)),
        textWrap(TextWrapping.NoWrap),
        marginRight(10),
      ];

    let codiconStyle =
      Style.[
        marginRight(10),
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
      ];
  };

  let common: t(_) =
    (~theme, ~font: UiFont.t, ~text, ~highlights, _item) => {
      let style = Styles.label(~theme);
      let normalStyle = style(~highlighted=false);
      let highlightStyle = style(~highlighted=true);
      <Oni_Components.HighlightText
        fontFamily={font.family}
        fontSize=12.
        style=normalStyle
        highlightStyle
        text
        highlights
      />;
    };

  let default = (~theme, ~font, ~text, ~highlights, item) => {
    // Reserve the icon space to be consistent with menus w/ icons
    [
      <Text style={Styles.icon(Revery.Colors.transparentWhite)} text="" />,
      common(~theme, ~font, ~text, ~highlights, item),
    ]
    |> React.listToElement;
  };

  let defaultWithIcon =
      (iconSelector, ~theme, ~font, ~text, ~highlights, item) => {
    let labelView = common(~theme, ~font, ~text, ~highlights, item);
    let icon = iconSelector(item);
    let iconView =
      switch (icon) {
      | Some(Icon.Seti(icon)) =>
        Oni_Core.IconTheme.IconDefinition.(
          <Text
            style={Styles.icon(icon.fontColor)}
            fontFamily={Revery.Font.Family.fromFile("seti.ttf")}
            fontSize=Constants.iconSize
            text={Oni_Components.FontIcon.codeToIcon(icon.fontCharacter)}
          />
        )
      | Some(Icon.Codicon({fontSize, color, icon})) =>
        let fontSize = fontSize |> Option.value(~default=font.size);
        let colorSchema: Oni_Core.ColorTheme.Schema.definition =
          color |> Option.value(~default=Feature_Theme.Colors.Menu.foreground);

        let color = colorSchema.from(theme);

        <Revery.UI.View style=Styles.codiconStyle>
          <Codicon icon fontSize color />
        </Revery.UI.View>;
      | None =>
        <Text style={Styles.icon(Revery.Colors.transparentWhite)} text="" />
      };

    [iconView, labelView] |> Revery.UI.React.listToElement;
  };
};

type internal('item, 'outmsg) = {
  focusFirstItemByDefault: bool,
  onItemFocused: option('item => 'outmsg),
  onAccepted: option((~text: string, ~item: option('item)) => 'outmsg),
  onCancelled: option(unit => 'outmsg),
  placeholderText: string,
  itemRenderer: Renderer.t('item),
  toString: 'item => string,
  items: list('item),
};

type menu('outmsg) =
  | Menu(internal('item, 'outmsg)): menu('outmsg);

let menu:
  (
    ~focusFirstItemByDefault: bool=?,
    ~onItemFocused: 'item => 'outmsg=?,
    ~onAccepted: (~text: string, ~item: option('item)) => 'outmsg=?,
    ~onCancelled: unit => 'outmsg=?,
    ~placeholderText: string=?,
    ~itemRenderer: Renderer.t('item)=?,
    ~toString: 'item => string,
    list('item)
  ) =>
  menu('outmsg) =
  (
    ~focusFirstItemByDefault=true,
    ~onItemFocused=?,
    ~onAccepted=?,
    ~onCancelled=?,
    ~placeholderText="type to search...",
    ~itemRenderer=Renderer.default,
    ~toString,
    initialItems,
  ) => {
    Menu({
      focusFirstItemByDefault,
      onItemFocused,
      onAccepted,
      onCancelled,
      placeholderText,
      itemRenderer,
      toString,
      items: initialItems,
    });
  };

let mapFunction: ('a => 'b, 'item => 'a, 'item) => 'b =
  (f, orig, item) => {
    f(orig(item));
  };

let map: ('a => 'b, menu('a)) => menu('b) =
  (f, model) => {
    switch (model) {
    | Menu({onItemFocused, onAccepted, onCancelled, _} as orig) =>
      Menu({
        ...orig,
        onItemFocused: onItemFocused |> Option.map(mapFunction(f)),
        onAccepted: {
          onAccepted
          |> Option.map((orig, ~text, ~item) => {f(orig(~text, ~item))});
        },
        onCancelled: onCancelled |> Option.map(mapFunction(f)),
      })
    };
  };

module Instance = {
  type t('outmsg) =
    | Instance({
        schema: internal('item, 'outmsg),
        text: Component_InputText.model,
        allItems: list('item),
        filteredItems: array(Filter.result('item)),
        focused: option(int),
      })
      : t('outmsg);

  let updateFilteredItems =
    fun
    | Instance({text, schema, allItems, _} as orig) => {
        let format = (item, ~shouldLower) => {
          let str = schema.toString(item);
          shouldLower ? String.lowercase_ascii(str) : str;
        };
        let queryStr = Component_InputText.value(text);
        let shouldLower = queryStr == String.lowercase_ascii(queryStr);
        let query = Zed_utf8.explode(queryStr);
        let filteredItems =
          if (StringEx.isEmpty(queryStr)) {
            // If there is no query, preserve original item order
            allItems
            |> List.mapi((idx, item) =>
                 Filter.{item, highlight: [], score: (-1.0) *. float(idx)}
               )
            |> Array.of_list;
          } else {
            allItems
            |> List.filter(item =>
                 Filter.fuzzyMatches(query, format(item, ~shouldLower))
               )
            |> Filter.rank(queryStr, format)
            |> Array.of_list;
          };

        Instance({...orig, filteredItems});
      };

  let text =
    fun
    | Instance({text, _}) => text;

  let paste = (~text) =>
    fun
    | Instance({text: prev, _} as orig) =>
      Instance({...orig, text: Component_InputText.paste(~text, prev)})
      |> updateFilteredItems;

  let key = (~key) =>
    fun
    | Instance({text, _} as orig) =>
      Instance({...orig, text: Component_InputText.handleInput(~key, text)})
      |> updateFilteredItems;

  let input = msg =>
    fun
    | Instance(orig) => {
        let (text', _outmsg) = Component_InputText.update(msg, orig.text);
        Instance({...orig, text: text'}) |> updateFilteredItems;
      };

  let count =
    fun
    | Instance({filteredItems, _}) => Array.length(filteredItems);

  let render = (~index, ~theme, ~font) =>
    fun
    | Instance({filteredItems, schema, _}) => {
        let len = Array.length(filteredItems);

        if (index >= 0 && index < len) {
          let item = filteredItems[index];
          let itemStr = schema.toString(item.item);
          let highlights = item.highlight;

          Some(
            schema.itemRenderer(
              ~theme,
              ~font,
              ~text=itemStr,
              ~highlights,
              item.item,
            ),
          );
        } else {
          None;
        };
      };

  let next =
    fun
    | Instance({filteredItems, focused, _} as orig) => {
        let focused' =
          focused
          |> Utility.IndexEx.nextRollOverOpt(
               ~last=Array.length(filteredItems) - 1,
             );
        Instance({...orig, focused: focused'});
      };

  let focus = index =>
    fun
    | Instance({filteredItems, _} as orig) => {
        let focused' =
          Utility.IntEx.clamp(
            index,
            ~lo=0,
            ~hi=Array.length(filteredItems) - 1,
          );
        Instance({...orig, focused: Some(focused')});
      };

  let previous =
    fun
    | Instance({filteredItems, focused, _} as orig) => {
        let focused' =
          focused
          |> Utility.IndexEx.prevRollOverOpt(
               ~last=Array.length(filteredItems) - 1,
             );
        Instance({...orig, focused: focused'});
      };

  let select: t('a) => option('a) =
    fun
    | Instance({schema, filteredItems, focused, text, _}) => {
        let len = Array.length(filteredItems);
        let item =
          focused
          |> OptionEx.flatMap(focusedIndex =>
               if (focusedIndex >= 0 && focusedIndex < len) {
                 Some(filteredItems[focusedIndex].item);
               } else {
                 None;
               }
             );

        let text = Component_InputText.value(text);
        schema.onAccepted |> Option.map(f => f(~text, ~item));
      };

  let focusMsg: t('a) => option('a) =
    fun
    | Instance({schema, filteredItems, focused, _}) => {
        let len = Array.length(filteredItems);
        focused
        |> OptionEx.flatMap(focusedIndex =>
             if (focusedIndex >= 0 && focusedIndex < len) {
               let item = filteredItems[focusedIndex];
               schema.onItemFocused |> Option.map(f => f(item.item));
             } else {
               None;
             }
           );
      };

  let focused =
    fun
    | Instance({focused, _}) => focused;

  let sub = _model => {
    Isolinear.Sub.none;
  };
};

let instantiate: menu('outmsg) => Instance.t('outmsg) =
  menu =>
    switch (menu) {
    | Menu(internal) =>
      let canFocusDefault =
        internal.focusFirstItemByDefault && internal.items != [];
      let focused = canFocusDefault ? Some(0) : None;
      Instance.Instance({
        schema: internal,
        text:
          Component_InputText.empty
          |> Component_InputText.setPlaceholder(
               ~placeholder=internal.placeholderText,
             ),
        allItems: internal.items,
        filteredItems: [||],
        focused,
      })
      |> Instance.updateFilteredItems;
    };
