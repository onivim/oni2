open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="Regression: Command line no completions", (dispatch, wait) => {

    wait(~name="Initial mode is normal", (state: State.t) => {
        state.mode == Vim.Types.Normal;
    });

    dispatch(KeyboardInput(":"));
    
    wait(~name="Mode switches to command line", (state: State.t) => {
        state.mode == Vim.Types.CommandLine;
    });
    
    dispatch(KeyboardInput("e"));
    wait(~name="Mode switches to command line", (state: State.t) => {
        state.commandline.text == "e"
    });
    
    dispatch(KeyboardInput("h"));
    
    wait(~name="Mode switches to command line", (state: State.t) => {
        state.commandline.text == "eh"
    });
});
