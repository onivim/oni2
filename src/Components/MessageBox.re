open Oni_Core;
open Utility;

// MODEL

type action('msg) = {
  label: string,
  msg: 'msg,
  shortcut,
}

and shortcut =
  | Key(string)
  | Sequence(string);

type model('msg) = {
  actions: list(action('msg)),
  input: string,
};

// UPDATE

type msg =
  | KeyPressed(string);

let dispatchEffect = msg =>
  Isolinear.Effect.createWithDispatch(
    ~name="messageBox.dispatchMessage", dispatch =>
    dispatch(msg)
  );

let update = (model, msg) =>
  switch (msg) {
  | KeyPressed(key) =>
    if (key == "<CR>") {
      let eff =
        model.actions
        |> List.find_opt(action => action.shortcut == Sequence(model.input))
        |> Option.map(action => dispatchEffect(action.msg))
        |> Option.value(~default=Isolinear.Effect.none);

      (model, eff);
    } else if (key == "<BS>") {
      let input =
        String.sub(model.input, 0, max(0, String.length(model.input) - 1));

      ({...model, input}, Isolinear.Effect.none);
    } else {
      let maybeEff =
        model.actions
        |> List.find_opt(action => action.shortcut == Key(key))
        |> Option.map(action => dispatchEffect(action.msg));

      switch (maybeEff) {
      | Some(eff) => (model, eff)
      | None =>
        let input' = model.input ++ key;

        let matchesSomeSequence =
          model.actions
          |> List.map(action => action.shortcut)
          |> List.filter_map(
               fun
               | Key(_) => None
               | Sequence(seq) => Some(seq),
             )
          |> List.exists(StringEx.startsWith(~prefix=input'));

        matchesSomeSequence
          ? ({...model, input: input'}, Isolinear.Effect.none)
          : (model, Isolinear.Effect.none);
      };
    }
  };

// VIEW

open Revery.UI;
open Revery.UI.Components;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let container = (~theme) => [
    backgroundColor(Colors.Oni.Modal.background.from(theme)),
  ];

  let message = [padding(20), paddingBottom(10)];

  let actions = [flexDirection(`Row)];

  let buttonOuter = (~isHovered, ~theme) => [
    isHovered
      ? backgroundColor(Colors.Selection.background.from(theme))
      : backgroundColor(Colors.Oni.Modal.background.from(theme)),
    flexGrow(1),
  ];

  let buttonInner = [padding(10), flexDirection(`Row)];

  let buttonText = (~theme) => [
    color(Colors.Oni.Modal.foreground.from(theme)),
    alignSelf(`Center),
  ];

  let shortcut = [flexDirection(`Row), marginLeft(6)];

  let shortcutText = (~theme) => [
    color(Colors.Oni.Modal.shortcutForeground.from(theme)),
    alignSelf(`Center),
  ];

  let shortcutHighlight = (~theme) => [
    color(Colors.Oni.Modal.shortcutHighlightForeground.from(theme)),
    alignSelf(`Center),
  ];
};

let shortcutView = (~text, ~input="", ~theme, ~font: UiFont.t, ()) => {
  let text =
    String.sub(
      text,
      String.length(input),
      String.length(text) - String.length(input),
    );

  <View style=Styles.shortcut>
    <Text
      style={Styles.shortcutHighlight(~theme)}
      fontFamily={font.family}
      fontSize={font.size}
      text=input
    />
    <Text
      style={Styles.shortcutText(~theme)}
      fontFamily={font.family}
      fontSize={font.size}
      text
    />
  </View>;
};

let%component button =
              (
                ~text,
                ~shortcut,
                ~input,
                ~onClick,
                ~theme,
                ~font: UiFont.t,
                (),
              ) => {
  let%hook (isHovered, setHovered) = Hooks.state(false);

  let shortcut = () => {
    switch (shortcut) {
    | Key(key) => <shortcutView text=key theme font />

    | Sequence(sequence) =>
      if (input != "" && StringEx.startsWith(~prefix=input, sequence)) {
        <shortcutView text=sequence input theme font />;
      } else {
        <shortcutView text=sequence theme font />;
      }
    };
  };

  let onMouseOver = _ => setHovered(_ => true);
  let onMouseOut = _ => setHovered(_ => false);

  <Clickable onClick style={Styles.buttonOuter(~theme, ~isHovered)}>
    <View onMouseOver onMouseOut style=Styles.buttonInner>
      <Text
        style={Styles.buttonText(~theme)}
        fontFamily={font.family}
        fontSize={font.size}
        text
      />
      <shortcut />
    </View>
  </Clickable>;
};

let make = (~children as message, ~theme, ~font, ~model, ~onAction, ()) => {
  <View style={Styles.container(~theme)}>
    <View style=Styles.message> message </View>
    <View style=Styles.actions>
      {model.actions
       |> List.map(action =>
            <button
              text={action.label}
              shortcut={action.shortcut}
              input={model.input}
              onClick={() => onAction(action.msg)}
              theme
              font
            />
          )
       |> React.listToElement}
    </View>
  </View>;
};
