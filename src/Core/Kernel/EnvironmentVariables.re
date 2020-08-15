/*
 * EnvironmentVariables.re
 *
 * Environment variables that can be used to control the behavior of the runtime
 */

// [parentPid] is set for child processes, so that if the parent quits / is killed,
// the children processes will terminate as well.
let parentPid = "__ONI2_PARENT_PID__";

let namedPipe = "__ONI2_NAMED_PIPE__";
