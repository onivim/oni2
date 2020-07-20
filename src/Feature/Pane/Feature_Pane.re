/*
 * Feature_Pane.re
 */

[@deriving show({with_path: false})]
type pane =
  | Search
  | Diagnostics
  | Notifications;

module Constants = {
  let defaultHeight = 225;
  let minHeight = 80;
  let maxHeight = 600;
};

[@deriving show({with_path: false})]
type msg =
  | ResizeHandleDragged(int)
  | ResizeCommitted;

module Msg = {
  let resizeHandleDragged = v => ResizeHandleDragged(v);
  let resizeCommitted = ResizeCommitted;
};

type outmsg =
  | Nothing
  | PopFocus(pane);

type model = {
  selected: pane,
  isOpen: bool,
  height: int,
  resizeDelta: int,
};

let height = ({height, resizeDelta, _}) => {
  let candidateHeight = height + resizeDelta;
  if (candidateHeight < Constants.minHeight) {
    0;
  } else if (candidateHeight > Constants.maxHeight) {
    Constants.maxHeight;
  } else {
    candidateHeight;
  };
};

let update = (msg, model) =>
  switch (msg) {
  | ResizeHandleDragged(delta) => (
      {...model, resizeDelta: (-1) * delta},
      Nothing,
    )
  | ResizeCommitted =>
    let height = model |> height;

    if (height <= 0) {
      ({...model, isOpen: false, resizeDelta: 0}, PopFocus(model.selected));
    } else {
      ({...model, height, resizeDelta: 0}, Nothing);
    };
  };

let initial = {
  height: Constants.defaultHeight,
  resizeDelta: 0,
  selected: Search,
  isOpen: false,
};

let selected = ({selected, _}) => selected;

let isVisible = (pane, model) => model.isOpen && model.selected == pane;
let isOpen = ({isOpen, _}) => isOpen;

let show = (~pane, model) => {...model, isOpen: true, selected: pane};

let close = model => {...model, isOpen: false};
