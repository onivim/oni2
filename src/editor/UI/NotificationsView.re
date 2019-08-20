/*
 * NotificationsView.re
 *
 * View for KeyDisplayer
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Core = Oni_Core;
open Oni_Model;

let bgc = Color.rgb(0.1, 0.1, 0.1);
let fgc = Color.rgb(0.9, 0.9, 0.9);

let notificationWidth = 300;

let notification =
    (
      ~children as _,
      ~onClose,
      ~background,
      ~foreground,
      ~uiFont,
      ~message,
      ~title: string,
      (),
    ) => {
  <AllowPointer>
    <Padding padding=16>
      <BoxShadow
        boxShadow={Style.BoxShadow.make(
          ~xOffset=-15.,
          ~yOffset=5.,
          ~blurRadius=30.,
          ~spreadRadius=5.,
          ~color=Color.rgba(0., 0., 0., 0.2),
          (),
        )}>
        <View
          style=Style.[
            width(notificationWidth),
            backgroundColor(background),
          ]>
          <View
            style=Style.[
              flexDirection(`Row),
              justifyContent(`Center),
              alignItems(`Center),
            ]>
            <View
              style=Style.[
                flexGrow(0),
                justifyContent(`Center),
                alignItems(`Center),
              ]>
              <Container width=40 height=40>
                <Center>
                  <Text
                    style=Style.[
                      fontFamily(uiFont),
                      fontSize(16),
                      textWrap(TextWrapping.NoWrap),
                      backgroundColor(background),
                      color(foreground),
                      flexGrow(0),
                    ]
                    text="!"
                  />
                </Center>
              </Container>
            </View>
            <View
              style=Style.[
                flexGrow(1),
                flexDirection(`Column),
                maxWidth(notificationWidth - 80),
              ]>
              <Padding padding=8>
                <Text
                  style=Style.[
                    fontFamily(uiFont),
                    fontSize(16),
                    textWrap(TextWrapping.NoWrap),
                    backgroundColor(background),
                    color(foreground),
                    marginRight(16),
                    flexGrow(0),
                  ]
                  text=title
                />
                <Text
                  style=Style.[
                    fontFamily(uiFont),
                    fontSize(14),
                    backgroundColor(background),
                    color(foreground),
                    marginRight(16),
                    marginVertical(8),
                    flexGrow(0),
                  ]
                  text=message
                />
              </Padding>
            </View>
            <View
              style=Style.[
                flexGrow(0),
                justifyContent(`Center),
                alignItems(`Center),
              ]>
              <Clickable onClick={_ => onClose()}>
                <Container width=40 height=40>
                  <Center>
                    <Text
                      style=Style.[
                        fontFamily(uiFont),
                        fontSize(16),
                        textWrap(TextWrapping.NoWrap),
                        backgroundColor(background),
                        color(foreground),
                        flexGrow(0),
                      ]
                      text="x"
                    />
                  </Center>
                </Container>
              </Clickable>
            </View>
          </View>
        </View>
      </BoxShadow>
    </Padding>
  </AllowPointer>;
};

let createElement = (~children as _, ~state: State.t, ()) => {
  let uiFont = state.uiFont.fontFile;

  let {
    notificationInfoBackground,
    notificationInfoForeground,
    notificationSuccessBackground,
    notificationSuccessForeground,
    notificationWarningBackground,
    notificationWarningForeground,
    notificationErrorBackground,
    notificationErrorForeground,
  }: Core.Theme.EditorColors.t =
    state.theme.colors;

  let notifications =
    state.notifications
    |> List.map((n: Notification.t) => {
         let (background, foreground) =
           switch (n.notificationType) {
           | Actions.Success => (
               notificationSuccessBackground,
               notificationSuccessForeground,
             )
           | Actions.Warning => (
               notificationWarningBackground,
               notificationWarningForeground,
             )
           | Actions.Error => (
               notificationErrorBackground,
               notificationErrorForeground,
             )
           | _ => (notificationInfoBackground, notificationInfoForeground)
           };

          let onClose = () => {
              print_endline ("Trying to hide: " ++ string_of_int(n.id));
              GlobalContext.current().hideNotification(n.id);
          };

         <notification
           background
           foreground
           onClose
           uiFont
           title={n.title}
           message={n.message}
         />;
       })
    |> List.rev;

  <Positioned bottom=50 right=50> ...notifications </Positioned>;
};
