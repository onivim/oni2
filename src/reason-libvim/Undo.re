let saveCursor = Native.vimUndoSaveCursor;
let saveRegion = Native.vimUndoSaveRegion;
let sync = (~force: bool) => Native.vimUndoSync(force ? 1 : 0);
