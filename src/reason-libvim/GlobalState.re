open EditorCoreTypes;

let effects: ref(list(Effect.t)) = ref([]);

let autoIndent:
  ref(
    option(
      (~previousLine: string, ~beforePreviousLine: option(string)) =>
      AutoIndent.action,
    ),
  ) =
  ref(None);
let queuedFunctions: ref(list(unit => unit)) = ref([]);

let colorSchemeProvider: ref(ColorScheme.Provider.t) =
  ref(ColorScheme.Provider.default);

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
