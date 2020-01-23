/*
 * EnvironmentVariables.re
 *
 * Environment variables that can be used to control the behavior of the runtime
 */

// [parentPid] is set for child processes, so that if the parent quits / is killed,
// the children processes will terminate as well.
let parentPid = "__ONI2_PARENT_PID__";

// [camomilePath] specifies the path for the camomile runtime files (for Unicode)
// In some environments, our [Setup.t] won't infer the correct paths.
let camomilePath = "__ONI2_CAMOMILE_PATH__";
