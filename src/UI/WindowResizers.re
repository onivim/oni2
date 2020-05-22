/** WindowResizers.re
  This provides a native-like resize experiment for Windows.
  Since we have the frame disabled, we have to write our own
  behavior for this.
*/


open Revery;
open Revery.UI;

module Styles = {
  open Style;

  let size = 5;

  let topLeft = [
    position(`Absolute),
    Style.left(0),
    width(size),
    Style.top(0),
    height(size),
    backgroundColor(Revery.Colors.yellow),
    cursor
  ];

  let left = [
    position(`Absolute),
    Style.left(0),
    width(size),
    Style.top(size),
    Style.bottom(size),
    backgroundColor(Revery.Colors.red),
  ];

  let bottomLeft = [
    position(`Absolute),
    Style.left(0),
    width(size),
    Style.bottom(0),
    height(size),
    backgroundColor(Revery.Colors.green),
  ];

  let bottom = [
    position(`Absolute),
    Style.left(size),
    Style.right(size),
    Style.bottom(0),
    height(size),
    backgroundColor(Revery.Colors.orange),
  ];

  let bottomRight = [
    position(`Absolute),
    Style.right(0),
    Style.bottom(0),
    width(size),
    height(size),
    backgroundColor(Revery.Colors.violet),
  ];

  let right = [
    position(`Absolute),
    Style.right(0),
    width(size),
    Style.top(size),
    Style.bottom(size),
    backgroundColor(Revery.Colors.blue),
  ];

  let topRight = [
    position(`Absolute),
    Style.right(0),
    Style.top(0),
    width(size),
    height(size),
    backgroundColor(Revery.Colors.pink),
  ];

  let top = [
    position(`Absolute),
    Style.right(size),
    Style.left(size),
    Style.top(0),
    height(size),
    backgroundColor(Revery.Colors.black),
  ];
};

/* This can't be a component itself, because it would draw over the rest
   root, making the rest of the view non-clickable. In the root we render
   these through `React.listToElement`.
*/
let resizers = [
  <View style=Styles.topLeft mouseBehavior=Revery.UI.ResizeTopLeft />,
  <View style=Styles.left mouseBehavior=Revery.UI.ResizeLeft />,
  <View style=Styles.bottomLeft mouseBehavior=Revery.UI.ResizeBottomLeft />,
  <View style=Styles.bottom mouseBehavior=Revery.UI.ResizeBottom />,
  <View style=Styles.bottomRight mouseBehavior=Revery.UI.ResizeBottomRight />,
  <View style=Styles.right mouseBehavior=Revery.UI.ResizeRight />,
  <View style=Styles.topRight mouseBehavior=Revery.UI.ResizeTopRight />,
  <View style=Styles.top mouseBehavior=Revery.UI.ResizeTop />,
];
