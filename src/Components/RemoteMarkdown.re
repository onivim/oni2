open Revery.UI;

module FileContentsDownloader =
  RemoteAsset.Make({
    type asset = string;
    let mapper = (~filePath) => {
      Service_OS.Api.readFile(filePath) |> Lwt.map(Bytes.to_string);
    };
  });

let make =
    (
      ~colorTheme,
      ~tokenTheme,
      ~languageInfo,
      ~grammars,
      ~fontFamily,
      ~codeFontFamily,
      ~url: string,
      (),
    ) => {
  <FileContentsDownloader url>
    ...{
         fun
         | FileContentsDownloader.Downloading => <View />
         | FileContentsDownloader.Downloaded(markdown) =>
           <Markdown
             markdown
             colorTheme
             tokenTheme
             languageInfo
             grammars
             fontFamily
             codeFontFamily
             headerMargin=16
             baseFontSize=12.
             codeBlockFontSize=12.
           />
         | FileContentsDownloader.DownloadFailed({errorMsg}) =>
           <Text text=errorMsg />
       }
  </FileContentsDownloader>;
};
