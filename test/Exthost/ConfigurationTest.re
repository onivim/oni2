open TestFramework;

open Exthost;

describe("ConfigurationTest", ({test, _}) => {
  test("gets configuration value", _ => {
    let waitForChangeMany =
      fun
      | Msg.Diagnostics(ChangeMany({owner, entries})) =>
        owner == "diags" && List.length(entries) == 1
      | _ => false;

    let model1 =
      Configuration.Model.create(
        ~keys=["foo.bar"],
        `Assoc([("foo", `Assoc([("bar", `String("value1"))]))]),
      );

    let configuration1 =
      Configuration.create(~defaults=model1, ~user=model1, ());

    let model2 =
      Configuration.Model.create(
        ~keys=["foo.bar"],
        `Assoc([("foo", `Assoc([("bar", `String("value2"))]))]),
      );

    let configuration2 =
      Configuration.create(~defaults=model2, ~user=model2, ());

    Test.startWithExtensions(["oni-configuration"])
    |> Test.waitForExtensionActivation("oni-configuration")
    |> Test.withClient(
         Exthost.Request.Configuration.initializeConfiguration(
           ~configuration=configuration1,
         ),
       )
    |> Test.withClient(
         Exthost.Request.Configuration.acceptConfigurationChanged(
           ~configuration=configuration1,
           ~changed=model1,
         ),
       )
    |> Test.withClient(
         Exthost.Request.Configuration.acceptConfigurationChanged(
           ~configuration=configuration2,
           ~changed=model2,
         ),
       )
    |> Test.withClient(
         Exthost.Request.Commands.executeContributedCommand(
           ~arguments=[`String("foo.bar")],
           ~command="config.show",
         ),
       )
    |> Test.waitForMessage(~name="Diagnostics$changeMany", waitForChangeMany)
    |> Test.terminate
    |> Test.waitForProcessClosed;
  })
});
