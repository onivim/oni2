let getWidth = Native.vimWindowGetWidth;
let getHeight = Native.vimWindowGetHeight;
let getLeftColumn = Native.vimWindowGetLeftColumn;
let getTopLine = Native.vimWindowGetTopLine;
let setWidth = Native.vimWindowSetWidth;
let setHeight = Native.vimWindowSetHeight;
let setTopLeft = (top, left) => {
  let prevTop = getTopLine();
  let prevLeft = getLeftColumn();

  Native.vimWindowSetTopLeft(top, left);

  let newTop = getTopLine();
  let newLeft = getLeftColumn();

  if (prevTop != newTop) {
    Event.dispatch(newTop, Listeners.topLineChanged);
  };

  if (prevLeft != newLeft) {
    Event.dispatch(newLeft, Listeners.leftColumnChanged);
  };
};

let onLeftColumnChanged = f => Event.add(f, Listeners.leftColumnChanged);
let onTopLineChanged = f => Event.add(f, Listeners.topLineChanged);
let onSplit = f => Event.add2(f, Listeners.windowSplit);
