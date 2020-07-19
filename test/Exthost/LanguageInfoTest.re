open Oni_Core;
open TestFramework;

open Exthost;

let li =
  Exthost.Extension.Scanner.load(
    ~category=Exthost.Extension.Scanner.Default,
    "extensions/shellscript/package.json",
  )
  |> Option.to_list
  |> Exthost.LanguageInfo.ofExtensions;

let buf = Buffer.ofLines([|"#!/bin/bash", "ls *"|]);

describe("LanguageInfo", ({describe, _}) => {
  describe("get language from file path", ({test, _}) => {
    // test("get language from file path: Oni2 Path", ({expect, _}) => {
    //   let lang =
    //     Exthost.LanguageInfo.getLanguageFromFilePath(li, "oni://Welcome");
    //   expect.string(lang).toEqual("Welcome");
    // });
    test("get language from file path: extension", ({expect, _}) => {
      let fp = "/home/oni2/git/oni2/test.sh";
      let lang = Exthost.LanguageInfo.getLanguageFromFilePath(li, fp);
      expect.string(lang).toEqual("shellscript");
    });
    test(
      "get language from file path: extension (no file name)", ({expect, _}) => {
      let fp = "/home/oni2/git/oni2/.bash_profile";
      let lang = Exthost.LanguageInfo.getLanguageFromFilePath(li, fp);
      expect.string(lang).toEqual("shellscript");
    });
    test("get language from file path: file name", ({expect, _}) => {
      let fp = "/home/oni2/git/oni2/PKGBUILD";
      let lang = Exthost.LanguageInfo.getLanguageFromFilePath(li, fp);
      expect.string(lang).toEqual("shellscript");
    });
    test("get language from file path: file name pattern", ({expect, _}) => {
      // Load both the TS and JSON extensions, so we are sure that the ordering
      // is correct. Pattern > extension, so this json file is a jsonc file
      // as stated in the TS package.json.
      let tsli =
        Exthost.Extension.Scanner.load(
          ~category=Exthost.Extension.Scanner.Default,
          "extensions/typescript-basics/package.json",
        )
        |> Option.to_list;
      let jsonli =
        Exthost.Extension.Scanner.load(
          ~category=Exthost.Extension.Scanner.Default,
          "extensions/json/package.json",
        )
        |> Option.to_list;
      let combinedli = jsonli @ tsli |> Exthost.LanguageInfo.ofExtensions;

      let fp = "/home/oni2/git/oni2/tsconfig.base.json";
      let lang = Exthost.LanguageInfo.getLanguageFromFilePath(combinedli, fp);
      expect.string(lang).toEqual("jsonc");
    });
  });
  describe("get language from buffer", ({test, _}) => {
    // test("get language from buffer: Oni2 Path", ({expect, _}) => {
    //   let lang =
    //     Exthost.LanguageInfo.getLanguageFromFilePath(li, "oni://Welcome");
    //   expect.string(lang).toEqual("Welcome");
    // });
    test("get language from buffer: extension", ({expect, _}) => {
      let buf = Buffer.setFilePath(Some("/home/oni2/git/oni2/test.sh"), buf);
      let lang = Exthost.LanguageInfo.getLanguageFromBuffer(li, buf);
      expect.string(lang).toEqual("shellscript");
    });
    test("get language from buffer: extension (no file name)", ({expect, _}) => {
      let buf =
        Buffer.setFilePath(Some("/home/oni2/git/oni2/.bash_profile"), buf);
      let lang = Exthost.LanguageInfo.getLanguageFromBuffer(li, buf);
      expect.string(lang).toEqual("shellscript");
    });
    test("get language from buffer: file name", ({expect, _}) => {
      let buf =
        Buffer.setFilePath(Some("/home/oni2/git/oni2/PKGBUILD"), buf);
      let lang = Exthost.LanguageInfo.getLanguageFromBuffer(li, buf);
      expect.string(lang).toEqual("shellscript");
    });
    test("get language from buffer: file name pattern", ({expect, _}) => {
      let tsli =
        Exthost.Extension.Scanner.load(
          ~category=Exthost.Extension.Scanner.Default,
          "extensions/typescript-basics/package.json",
        )
        |> Option.to_list;
      let jsonli =
        Exthost.Extension.Scanner.load(
          ~category=Exthost.Extension.Scanner.Default,
          "extensions/json/package.json",
        )
        |> Option.to_list;
      let combinedli = jsonli @ tsli |> Exthost.LanguageInfo.ofExtensions;

      let buf =
        Buffer.setFilePath(
          Some("/home/oni2/git/oni2/tsconfig.base.json"),
          buf,
        );
      let lang = Exthost.LanguageInfo.getLanguageFromBuffer(combinedli, buf);
      expect.string(lang).toEqual("jsonc");
    });
    test("get language from buffer: first line", ({expect, _}) => {
      let buf =
        Buffer.setFilePath(Some("/home/oni2/git/oni2/my_script"), buf);
      let lang = Exthost.LanguageInfo.getLanguageFromBuffer(li, buf);
      expect.string(lang).toEqual("shellscript");
    });
  });
});
