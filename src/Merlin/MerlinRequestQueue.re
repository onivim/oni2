/*
 * Merlin.re
 *
 * Core merlin API
 */

open Oni_Core;

type diagnosticsRequest = {
  filePath: string,
  lines: array(string),
  workingDirectory: string,
  callback: MerlinProtocol.errorResult => unit,
};

type completionRequest = {
  filePath: string,
  lines: array(string),
  workingDirectory: string,
  prefix: string,
  position: Types.Position.t,
  callback: MerlinProtocol.completionResult => unit,
};

type request =
  | DiagnosticsRequest(diagnosticsRequest)
  | CompletionRequest(completionRequest);

type t = {
  // Pending requests
  diagnosticsRequest: option(diagnosticsRequest),
  completionRequest: option(completionRequest),
};

let requests: ref(t) =
  ref({completionRequest: None, diagnosticsRequest: None});

// Create a mutex to protect the requests' object from race conditions
let requestsMutex = Mutex.create();
// And a semaphore for signaling when we have a new request
let requestsCondition = Condition.create();

let hasPendingRequest = (v: t) => {
  switch (v.completionRequest, v.diagnosticsRequest) {
  | (Some(_), _) => true
  | (_, Some(_)) => true
  | _ => false
  };
};

let thread: ref(option(Thread.t)) = ref(None);

let popPendingRequest = (v: t) => {
  switch (v.completionRequest, v.diagnosticsRequest) {
  | (Some(cr), _) => (
      Some(CompletionRequest(cr)),
      {...v, completionRequest: None},
    )
  | (None, Some(dr)) => (
      Some(DiagnosticsRequest(dr)),
      {...v, diagnosticsRequest: None},
    )
  | (None, None) => (None, v)
  };
};

let _executeNextRequest = () => {
  // Get the next request we should run, and update the 'requests' object
  let (req, newRequests) = popPendingRequest(requests^);
  requests := newRequests;
  Mutex.unlock(requestsMutex);

  switch (req) {
  | Some(DiagnosticsRequest(dr)) =>
    Merlin.getErrors(dr.workingDirectory, dr.filePath, dr.lines, dr.callback)
  | Some(
      CompletionRequest({
        workingDirectory,
        filePath,
        lines,
        position,
        prefix,
        callback,
      }),
    ) =>
    Merlin.getCompletions(
      ~workingDirectory,
      ~filePath,
      ~fileContents=lines,
      ~position,
      ~prefix,
      callback,
    )
  | _ => Log.info("[MerlinRequestQueue] No request")
  };
};

// Create a thread for interacting with Merlin, if we haven't already
let _initializeThread = () => {
  switch (thread^) {
  | Some(_) => ()
  | None =>
    thread :=
      Some(
        Thread.create(
          () => {
            while (true) {
              Mutex.lock(requestsMutex);
              while (!hasPendingRequest(requests^)) {
                Condition.wait(requestsCondition, requestsMutex);
              };

              _executeNextRequest();
            }
          },
          (),
        ),
      )
  };
};

let _makeRequest = f => {
  _initializeThread();

  Mutex.lock(requestsMutex);
  requests := f(requests^);

  Condition.signal(requestsCondition);
  Mutex.unlock(requestsMutex);
};

let getErrors =
    (workingDirectory: string, filePath: string, lines: array(string), cb) => {
  let f = request => {
    ...request,
    diagnosticsRequest:
      Some({workingDirectory, filePath, lines, callback: cb}),
  };

  let _ =
    Thread.create(
      () => {
        print_endline("getErrors");
        // Artificially delay the get errors as a way to 'debounce'
        Unix.sleepf(0.1);
        _makeRequest(f);
      },
      (),
    );
  ();
};

let getCompletions =
    (
      workingDirectory: string,
      filePath: string,
      lines: array(string),
      prefix,
      position,
      cb: MerlinProtocol.completionResult => unit,
    ) => {
  let f = request => {
    ...request,
    completionRequest:
      Some({
        workingDirectory,
        filePath,
        lines,
        callback: cb,
        prefix,
        position,
      }),
  };

  _makeRequest(f);
};
