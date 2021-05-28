open Types;

type autocmdListener = (autocmd, Native.buffer) => unit;
type bufferListener = Native.buffer => unit;
type bufferLineEndingsChangedListener = (int, lineEnding) => unit;
type bufferModifiedChangedListener = (int, bool) => unit;
type bufferMetadataChangedListener = BufferMetadata.t => unit;
type bufferWriteListener = int => unit;
type bufferUpdateListener = BufferUpdate.t => unit;
type commandLineUpdateListener = Types.cmdline => unit;
type effectListener = Effect.t => unit;
type directoryChangedListener = string => unit;
type messageListener = (Types.msgPriority, string, string) => unit;
type quitListener = (Types.quitType, bool) => unit;
type windowMovementListener = (Types.windowMovementType, int) => unit;
type yankListener = Yank.t => unit;
type writeFailureListener = (writeFailureReason, buffer) => unit;
type noopListener = unit => unit;

let autocmd: ref(list(autocmdListener)) = ref([]);
let bufferFilenameChanged: ref(list(bufferMetadataChangedListener)) =
  ref([]);
let bufferFiletypeChanged: ref(list(bufferMetadataChangedListener)) =
  ref([]);
let bufferLineEndingsChanged: ref(list(bufferLineEndingsChangedListener)) =
  ref([]);
let bufferModifiedChanged: ref(list(bufferModifiedChangedListener)) =
  ref([]);
let bufferUpdate: ref(list(bufferUpdateListener)) = ref([]);
let bufferWrite: ref(list(bufferWriteListener)) = ref([]);
let commandLineEnter: ref(list(commandLineUpdateListener)) = ref([]);
let commandLineUpdate: ref(list(commandLineUpdateListener)) = ref([]);
let commandLineLeave: ref(list(noopListener)) = ref([]);
let directoryChanged: ref(list(directoryChangedListener)) = ref([]);
let effect: ref(list(effectListener)) = ref([]);
let intro: ref(list(noopListener)) = ref([]);
let message: ref(list(messageListener)) = ref([]);
let quit: ref(list(quitListener)) = ref([]);
let terminalRequested: Event.t(Types.terminalRequest => unit) =
  Event.create();
let unhandledEscape: ref(list(noopListener)) = ref([]);
let version: ref(list(noopListener)) = ref([]);
let windowMovement: ref(list(windowMovementListener)) = ref([]);
let yank: ref(list(yankListener)) = ref([]);
let writeFailure: ref(list(writeFailureListener)) = ref([]);
