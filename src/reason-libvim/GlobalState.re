open EditorCoreTypes;

let context: ref(option(Context.t)) = ref(None);

let effects: ref(list(Effect.t)) = ref([]);

let queuedFunctions: ref(list(unit => unit)) = ref([]);

let overriddenMessageHandler:
  ref(option((Types.msgPriority, string, string) => unit)) =
  ref(None);

let viewLineMotion:
  ref(
    option(
      (~motion: ViewLineMotion.t, ~count: int, ~startLine: LineNumber.t) =>
      LineNumber.t,
    ),
  ) =
  ref(None);

let screenPositionMotion:
  ref(
    option(
      (
        ~direction: [ | `Up | `Down],
        ~count: int,
        ~line: LineNumber.t,
        ~currentByte: ByteIndex.t,
        ~wantByte: ByteIndex.t
      ) =>
      BytePosition.t,
    ),
  ) =
  ref(None);

let toggleComments: ref(option(array(string) => array(string))) =
  ref(None);
