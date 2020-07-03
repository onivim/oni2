let saveRegion = Native.vimUndoSaveRegion;
let sync = (~force: bool) => Native.vimUndoSync(force ? 1 : 0);
