/**
 *
 * The [Scheduler] module represents a way of scheduling
 * callbacks - should they run on the main thread? Is
 * it ok to run them on another thread?
 */

type t;

/* [mainThread] is a scheduler that only runs the jobs on the main thread. */
let mainThread: t;

/* [immediate] will run a job immediately, regardless of the thread */
let immediate: t;

type callback = unit => unit;

let run: (callback, t) => unit;
