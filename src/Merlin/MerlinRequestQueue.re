/*
 * Merlin.re
 *
 * Core merlin API
 */

type diagnosticsRequest = {
  filePath: string,
  lines: array(string),
  workingDirectory: string,
  callback: MerlinProtocol.errorResult => unit,
};

type request =
  | DiagnosticsRequest(diagnosticsRequest);

type t = {
  // Pending requests
  diagnosticsRequest: option(diagnosticsRequest),
};

let requests: ref(t) = ref({diagnosticsRequest: None});

// Create a mutex to protect the requests' object from race conditions
let requestsMutex = Mutex.create();
// And a semaphore for signaling when we have a new request
let requestsCondition = Condition.create();

let hasPendingRequest = (v: t) => {
  switch (v.diagnosticsRequest) {
  | Some(_) => true
  | _ => false
  };
};

let thread: ref(option(Thread.t)) = ref(None);

let popPendingRequest = (v: t) => {
  switch (v.diagnosticsRequest) {
  | Some(dr) => (
      Some(DiagnosticsRequest(dr)),
      {...v, diagnosticsRequest: None},
    )
  | None => (None, v)
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
  | _ => print_endline("No request")
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

let getErrors =
    (workingDirectory: string, filePath: string, lines: array(string), cb) => {
  _initializeThread();

  Mutex.lock(requestsMutex);
  requests :=
    {
      ...requests^,
      diagnosticsRequest:
        Some({workingDirectory, filePath, lines, callback: cb}),
    };

  Condition.signal(requestsCondition);
  Mutex.unlock(requestsMutex);
};
