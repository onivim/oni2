let subscription = (~buffers, ~editors, ~activeEditorId, ~client) => {
  buffers
  |> List.map(buffer => {Service_Exthost.Sub.buffer(~buffer, ~client)})
  |> Isolinear.Sub.batch;
};
