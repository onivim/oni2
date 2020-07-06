// Is this enough to repro the OCaml 4.10 ASAN failure?
Oni_Core_Test.TestFramework.run(
  Rely.RunConfig.withReporters(
    [Default, JUnit("./junit.xml")],
    Rely.RunConfig.initialize(),
  ),
);

//Oni_Core_Utility_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Oni_Input_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Oni_Model_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Oni_Syntax_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Oni_Extensions_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Oni_Components_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Feature_Editor_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//Feature_LanguageSupport_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Exthost_Transport_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Exthost_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Vim.init();
//Libvim_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Oniguruma_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Service_Extensions_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Service_Net_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//Textmate_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
//
//Oni_Cli_Test.TestFramework.run(
//  Rely.RunConfig.withReporters(
//    [Default, JUnit("./junit.xml")],
//    Rely.RunConfig.initialize(),
//  ),
//);
