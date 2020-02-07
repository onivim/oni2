type t = {
  fontFile: string,
  fontFileItalic: string,
  fontFileBold: string,
  fontFileSemiBold: string,
  fontFileSemiBoldItalic: string,
  fontSize: float,
};

let create =
    (
      ~fontFile,
      ~fontFileBold,
      ~fontFileItalic,
      ~fontFileSemiBold,
      ~fontFileSemiBoldItalic,
      ~fontSize,
      (),
    ) => {
  fontFile,
  fontFileBold,
  fontFileItalic,
  fontFileSemiBold,
  fontFileSemiBoldItalic,
  fontSize,
};

let default =
  create(
    ~fontFile="Inter-UI-Regular.ttf",
    ~fontFileBold="Inter-UI-Bold.ttf",
    ~fontFileItalic="Inter-UI-Italic.ttf",
    ~fontFileSemiBold="Inter-UI-Medium.ttf",
    ~fontFileSemiBoldItalic="Inter-UI-MediumItalic.ttf",
    ~fontSize=12.,
    (),
  );
