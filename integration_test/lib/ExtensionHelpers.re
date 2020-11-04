/*
 * LanguageHelpers.re
 */
module Core = Oni_Core;
module Log = (val Core.Log.withNamespace("IntegrationTest.ExtensionHelpers"));

module Model = Oni_Model;

module State = Model.State;

let waitForExtensionToActivate =
    (~extensionId, waitForState: Types.waitForState) => {
  // Wait until the extension is activated
  // Give some time for the exthost to start
  let () =
    waitForState(
      ~name=
        Printf.sprintf(
          "Validate the '%s' extension gets activated",
          extensionId,
        ),
      ~timeout=30.0,
      (state: State.t) =>
      List.exists(
        id => id == extensionId,
        state.extensions |> Feature_Extensions.activatedIds,
      )
    );
  ();
};

let waitForNewCompletionProviders =
    (~description, f, waitForState: Types.waitForState) => {
  let originalDescription = description;

  // First, figure out how many suggest providers we have...
  let existingCompletionCount = ref(0);
  waitForState(
    ~name=
      "Getting count of current suggest providers: " ++ originalDescription,
    (State.{languageSupport, _}) => {
      let current =
        Feature_LanguageSupport.Completion.providerCount(languageSupport);

      existingCompletionCount := current;
      true;
    },
  );

  f();

  waitForState(
    ~timeout=10.0,
    ~name="Waiting for new suggest providers: " ++ originalDescription,
    (State.{languageSupport, _}) => {
      let current =
        Feature_LanguageSupport.Completion.providerCount(languageSupport);

      current > existingCompletionCount^;
    },
  );
};
