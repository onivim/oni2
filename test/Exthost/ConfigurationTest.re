open Oni_Core;
open TestFramework;

open Exthost;

let model1 =
  Configuration.Model.create(
    ~keys=["foo.bar"],
    `Assoc([("foo", `Assoc([("bar", `String("value1"))]))]),
  );

let configuration1 = Configuration.create(~user=model1, ());

module TestConfigurationEvent = {
  type t = {
    eventType: string,
    result: string,
  };

  let decode = {
    Json.Decode.(
      obj(({field, _}) => {
        {
          eventType: field.required("eventType", string),
          result: field.required("result", string),
        }
      })
    );
  };
};

let waitForConfigurationShowEvent = (~name, f, context) => {
  context
  |> Test.waitForMessage(
       ~name,
       fun
       | Msg.MessageService(ShowMessage({message, _})) => {
           message
           |> Yojson.Safe.from_string
           |> Json.Decode.decode_value(TestConfigurationEvent.decode)
           |> Result.map(f)
           |> Result.value(~default=false);
         }
       | _ => false,
     );
};

describe("ConfigurationTest", ({test, _}) => {
  test("sets configuration value", _ => {
    Test.startWithExtensions(["oni-configuration"])
    |> Test.waitForExtensionActivation("oni-configuration")
    |> Test.withClient(
         Exthost.Request.Configuration.acceptConfigurationChanged(
           ~configuration=configuration1,
           ~changed=model1,
         ),
       )
    |> Test.withClient(
         Exthost.Request.Commands.executeContributedCommand(
           ~arguments=[`String("foo.bar")],
           ~command="config.show",
         ),
       )
    |> waitForConfigurationShowEvent(
         ~name="Value should be 'value1'", ({result, _}) => {
         String.equal(result, "value1")
       })
    |> Test.terminate
    |> Test.waitForProcessClosed
  })
});
