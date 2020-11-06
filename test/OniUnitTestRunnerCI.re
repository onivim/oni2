Printexc.record_backtrace(true);

Printexc.set_uncaught_exception_handler((exn, bt) => {
  prerr_endline(" == UNHANDLED EXCEPTION:");
  prerr_endline(Printexc.to_string(exn));
  prerr_endline(" -- Backtrace:");
  prerr_endline(Printexc.raw_backtrace_to_string(bt));
});

let initializeRunConfig = runFn => {
  let runConfig =
    Rely.RunConfig.initialize()
    |> Rely.RunConfig.withReporters([Default, JUnit("./junit.xml")])
    |> Rely.RunConfig.onTestFrameworkFailure(() => {
         prerr_endline("Exiting test due to failure...");
         exit(2);
       });

  runFn(runConfig);
};

Oni_Core_Test.TestFramework.run |> initializeRunConfig;

Oni_Core_Utility_Test.TestFramework.run |> initializeRunConfig;

Oni_Input_Test.TestFramework.run |> initializeRunConfig;

Oni_Model_Test.TestFramework.run |> initializeRunConfig;

Oni_Syntax_Test.TestFramework.run |> initializeRunConfig;

Feature_Editor_Test.TestFramework.run |> initializeRunConfig;

Feature_Diagnostics_Test.TestFramework.run |> initializeRunConfig;

Feature_LanguageSupport_Test.TestFramework.run |> initializeRunConfig;

EditorCoreTypes_Test.TestFramework.run |> initializeRunConfig;

EditorInput_Test.TestFramework.run |> initializeRunConfig;

Exthost_Transport_Test.TestFramework.run |> initializeRunConfig;

Exthost_Test.TestFramework.run |> initializeRunConfig;

Vim.init();

Libvim_Test.TestFramework.run |> initializeRunConfig;

Oniguruma_Test.TestFramework.run |> initializeRunConfig;

Service_Extensions_Test.TestFramework.run |> initializeRunConfig;

Service_Net_Test.TestFramework.run |> initializeRunConfig;

Service_OS_Test.TestFramework.run |> initializeRunConfig;
Textmate_Test.TestFramework.run |> initializeRunConfig;

TreeSitter_Test.TestFramework.run |> initializeRunConfig;

Oni_Cli_Test.TestFramework.run |> initializeRunConfig;
