open Revery.UI;

let make = (~width: int, ~height: int, ~url: string, ()) => {
  <RemoteAsset url>
    ...{
         fun
         | RemoteAsset.Downloading => <View />
         | RemoteAsset.Downloaded({filePath}) =>
           <Image width height src={`File(filePath)} />
         | RemoteAsset.DownloadFailed({errorMsg}) => <Text text=errorMsg />
       }
  </RemoteAsset>;
};
