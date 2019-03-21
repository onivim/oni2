Oni_Core_Test.TestFramework.run(
  Rely.RunConfig.withReporters(
    [Default, JUnit("./junit.xml")],
    Rely.RunConfig.initialize(),
  ),
);

Oni_Neovim_Test.TestFramework.run(
  Rely.RunConfig.withReporters(
    [Default, JUnit("./junit.xml")],
    Rely.RunConfig.initialize(),
  ),
);
