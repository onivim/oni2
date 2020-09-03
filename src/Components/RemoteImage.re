open Revery.UI;

module FilePathDownloader =
  RemoteAsset.Make({
    type asset = string;
    let mapper = (~filePath) => Lwt.return(filePath);
  });

let make = (~width: int, ~height: int, ~url: string, ()) => {
  <FilePathDownloader url>
    ...{
         fun
         | FilePathDownloader.Downloading => <View />
         | FilePathDownloader.Downloaded(filePath) =>
           <Image width height src={`File(filePath)} />
         | FilePathDownloader.DownloadFailed({errorMsg}) =>
           <Text text=errorMsg />
       }
  </FilePathDownloader>;
};
