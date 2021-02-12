let getWidth = Native.vimWindowGetWidth;
let getHeight = Native.vimWindowGetHeight;
let getLeftColumn = Native.vimWindowGetLeftColumn;
let getTopLine = Native.vimWindowGetTopLine;
let setWidth = Native.vimWindowSetWidth;
let setHeight = Native.vimWindowSetHeight;
let setTopLeft = (top, left) => {
  Native.vimWindowSetTopLeft(top, left);
};
