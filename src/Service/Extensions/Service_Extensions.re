open Oni_Core;
module Constants = {
  let baseUrl = "https://open-vsx.org/api"
};

module Url = {

  let extensionInfo = (publisher, id) => {
    Printf.sprintf("%s/%s/%s", Constants.baseUrl, publisher, id);
  };
  
};

module Catalog = {
  module VersionInfo = {
    type t = {
      version: string,
      url: string,
    };
  };

  module Entry = {
    type t = {
      downloadUrl: string,
//      repositoryUrl: string,
//      homepageUrl: string,
//      manifestUrl: string,
//      iconUrl: string,
//      readmeUrl: string,
//      licenseName: string,
//      licenseUrl: string,
//      name: string,
//      namespace: string,
//      downloadCount: int,
//      displayName: string,
//      descrption: string,
//      categories: list(string),
//      versions: list(VersionInfo.t),
    };

    let toString = ({
      downloadUrl,
      }) => {
      Printf.sprintf({|Extension %s:
      - Download Url: %s
      |}, "derp", downloadUrl);
    };

    module Decode = {
      open Json.Decode;

      let versions = key_value_pairs(string);

      let files = field("files");
      let downloadUrl = files(field("download", string));
      let manifestUrl = files(field("manifest", string));

      let decode = obj(({whatever, _}) => {
        downloadUrl: whatever(downloadUrl),
        //manifestUrl: whatever(manifestUrl),
      });
      
      
    };
    
  };

  let query = (
    ~setup,
    ~publisher,
    name
  ) => {

    let url = Url.extensionInfo(publisher, name);

    Service_Net.Request.json(
      ~setup,
      ~decoder=Entry.Decode.decode,
      url
    )
  };
  
};

