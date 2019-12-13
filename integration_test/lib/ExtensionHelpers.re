/*
 * LanguageHelpers.re
 */
module Core = Oni_Core;
module Log = Core.Log;
module Option = Core.Utility.Option;

module Model = Oni_Model;

module State = Model.State;
module LanguageFeatures = Model.LanguageFeatures;

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
      List.exists(id => id == extensionId, state.extensions.activatedIds)
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
    (State.{languageFeatures, _}) => {
      let current = LanguageFeatures.getCompletionProviders(languageFeatures);

      Log.info("Current suggest providers: ");
      List.iter(id => Log.info("-- " ++ id), current);

      existingCompletionCount := List.length(current);
      true;
    },
  );

  f();

  waitForState(
    //~timeout=30.0,

      ~name="Waiting for new suggest providers: " ++ originalDescription,
      (State.{languageFeatures, _}) => {
        let current =
          LanguageFeatures.getCompletionProviders(languageFeatures);

        Log.info("Current suggest providers: ");
        List.iter(id => Log.info("-- " ++ id), current);

        List.length(current) > existingCompletionCount^;
      },
    );
};
