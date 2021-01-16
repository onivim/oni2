open Oni_Core;
open Utility;
open Oni_Model;
open Oni_IntegrationTestLib;
module Buffer = Oni_Core.Buffer;

let configuration = {|
{
    "workbench.colorTheme": "very-invalid-theme",
}
|};

runTest(
  ~configuration=Some(configuration),
  ~name="ConfigurationInvalidThemeTest (Regression test for #2985)",
  ({wait, _}) => {
    // We should get an error message referencing our very-invalid-theme..
    wait(~name="Wait for error message", (state: State.t) => {
      state.notifications
      |> Feature_Notification.all
      |> List.exists(notification => {
           Feature_Notification.(
             {
               notification.kind == Error
               && StringEx.contains(
                    "very-invalid-theme",
                    notification.message,
                  );
             }
           )
         })
    });

    // But, we should revert to our default laserwave-italic theme. Check one of the colors...
    wait(~name="Wait for error message", (state: State.t) => {
      let colors = state.colorTheme |> Feature_Theme.colors;

      let editorBackgroundColor =
        Feature_Theme.Colors.Editor.background.from(colors);

      let expectedColor = Revery.Color.hex("#27212e");
      expectedColor == editorBackgroundColor;
    });
  },
);
