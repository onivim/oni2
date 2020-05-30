open TestFramework;

open Oni_Core;
open Exthost;

let testUri = Uri.fromPath("/test/path");

let model = (~lines) =>
  Exthost.ModelAddedDelta.create(
    ~versionId=0,
    ~lines,
    ~eol=Eol.LF,
    ~modeId="plaintext",
    ~isDirty=false,
    testUri,
  );

module TestDocumentEvent = {
  type t = {
    eventType: string,
    filename: string,
    fullText: string,
  };

  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          eventType: field.required("type", string),
          filename: field.required("filename", string),
          fullText: field.withDefault("fullText", "", string),
        }
      )
    );
  };
};

let waitForDocEvent = (~name, f, context) => {
  context
  |> Test.waitForMessage(
       ~name,
       fun
       | Msg.MessageService(ShowMessage({message, _})) => {
           message
           |> Yojson.Safe.from_string
           |> Json.Decode.decode_value(TestDocumentEvent.decode)
           |> Result.map(f)
           |> Result.value(~default=false);
         }
       | _ => false,
     );
};

describe("DocumentsTest", ({test, _}) => {
  test("open / close event", _ => {
    let addedDelta =
      DocumentsAndEditorsDelta.create(
        ~removedDocuments=[],
        ~addedDocuments=[model(~lines=[])],
        (),
      );

    let removedDelta =
      DocumentsAndEditorsDelta.create(
        ~removedDocuments=[testUri],
        ~addedDocuments=[],
        (),
      );

    Test.startWithExtensions(["oni-document-sync"])
    |> Test.waitForExtensionActivation("oni-document-sync")
    |> Test.withClient(
         Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
           ~delta=addedDelta,
         ),
       )
    |> waitForDocEvent(~name="Open", evt => {
         evt.eventType == "workspace.onDidOpenTextDocument"
       })
    |> Test.withClient(
         Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
           ~delta=removedDelta,
         ),
       )
    |> waitForDocEvent(~name="Open", evt => {
         evt.eventType == "workspace.onDidCloseTextDocument"
       })
    |> Test.terminate
    |> Test.waitForProcessClosed;
  });

  test("open with lines", _ => {
    let addedDelta =
      DocumentsAndEditorsDelta.create(
        ~removedDocuments=[],
        ~addedDocuments=[model(~lines=["Hello", "World"])],
        (),
      );

    Test.startWithExtensions(["oni-document-sync"])
    |> Test.waitForExtensionActivation("oni-document-sync")
    |> Test.withClient(
         Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
           ~delta=addedDelta,
         ),
       )
    |> waitForDocEvent(~name="Open", evt => {
         evt.eventType == "workspace.onDidOpenTextDocument"
         && evt.fullText == "Hello\nWorld"
       })
    |> Test.terminate
    |> Test.waitForProcessClosed;
  });

  test("update with lines", _ => {
    let addedDelta =
      DocumentsAndEditorsDelta.create(
        ~removedDocuments=[],
        ~addedDocuments=[model(~lines=["hello", "world"])],
        (),
      );

    let change: ModelContentChange.t = {
      range: {
        startLineNumber: 1,
        endLineNumber: 1,
        startColumn: 1,
        endColumn: 6,
      },
      text: "Greetings",
    };

    let modelChangedEvent: ModelChangedEvent.t = {
      changes: [change],
      eol: Eol.LF,
      versionId: 1,
    };

    Test.startWithExtensions(["oni-document-sync"])
    |> Test.waitForExtensionActivation("oni-document-sync")
    |> Test.withClient(
         Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
           ~delta=addedDelta,
         ),
       )
    |> Test.withClient(
         Request.Documents.acceptModelChanged(
           ~uri=testUri,
           ~modelChangedEvent,
           ~isDirty=false,
         ),
       )
    |> waitForDocEvent(~name="Update", evt => {
         evt.eventType == "workspace.onDidChangeTextDocument"
         && evt.fullText == "Greetings\nworld"
       })
    |> Test.terminate
    |> Test.waitForProcessClosed;
  });
});
