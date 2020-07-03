let saveRegion = (start, stop) => {
  let buf = Native.vimBufferGetCurrent();

  let totalLines = Native.vimBufferGetLineCount(buf);
  let start = max(0, start);
  let stop = min(stop, totalLines + 1);
  let _: bool = Native.vimUndoSaveRegion(start, stop);
  ();
};
let sync = (~force: bool) => Native.vimUndoSync(force ? 1 : 0);
