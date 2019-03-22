/*
 * FontIcon.re
 *
 * Helper component for using icon fonts, like FontAwesome
 */

open Revery;
open Revery.UI;
open CamomileLibraryDefault.Camomile;

let component = React.component("FontIcon");

let codeToIcon = icon => Zed_utf8.singleton(UChar.of_int(icon));

let createElement =
    (
      ~icon,
      ~fontFamily="FontAwesome5FreeSolid.otf",
      ~fontSize=50,
      ~backgroundColor,
      ~color,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let (fontFamily_, fontSize_, backgroundColor_, color_) = (
      fontFamily,
      fontSize,
      backgroundColor,
      color,
    );

    ignore(fontFamily_);
    ignore(fontSize_);
    ignore(icon);
    ignore(backgroundColor_);
    ignore(color_);

    let fontFamily_ = "seti.ttf";
    /* let _icon = 0x001; */

    print_endline(string_of_int(int_of_string("0xE001")));

    (
      hooks,
      <Text
        text={codeToIcon(0xE001)}
        style=Style.[
          fontFamily(fontFamily_),
          fontSize(30),
          color(Colors.white),
          backgroundColor(Colors.red),
          width(50),
          height(50),
        ]
      />,
    );
  });
