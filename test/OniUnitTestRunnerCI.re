Oni_Core_Test.TestFramework.run(
  Rely.RunConfig.withReporters(
    [Default, JUnit("./junit.xml")],
    Rely.RunConfig.initialize(),
  ),
);

Oni_Model_Test.TestFramework.run(
  Rely.RunConfig.withReporters(
    [Default, JUnit("./junit.xml")],
    Rely.RunConfig.initialize(),
  ),
);

Oni_Extensions_Test.TestFramework.run(
  Rely.RunConfig.withReporters(
    [Default, JUnit("./junit.xml")],
    Rely.RunConfig.initialize(),
  ),
);
