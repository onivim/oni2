open Revery.UI;
open Revery.UI.Components;

module FilePathDownloader =
  RemoteAsset.Make({
    type asset = string;
    let mapper = (~filePath) => Lwt.return(filePath);
  });

let make = (~width: int, ~height: int, ~url: string, ()) => {
  <FilePathDownloader url>
    ...{
         fun
         | FilePathDownloader.Downloading => <Container width height />
         | FilePathDownloader.Downloaded(filePath) =>
           <Container width height>
             <Image width height src={`File(filePath)} />
           </Container>
         | FilePathDownloader.DownloadFailed({errorMsg}) =>
           <Container width height> <Text text=errorMsg /> </Container>
       }
  </FilePathDownloader>;
};
