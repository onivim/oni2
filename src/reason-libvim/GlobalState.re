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
