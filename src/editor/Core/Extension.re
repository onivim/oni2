module ExtensionLanguageContribution {
    type t = {
        id: string,
        extensions: list(string),
        aliases: list(string),
    }
};

module ExtensionGrammarContribution {
    type t = {
        language: string,
        scopeName: string,
        path: string,
    }
}

module ExtensionContributes {

    type t = { 
        languages: list(ExtensionLanguageContribution.t),
        grammar: list(ExtensionGrammarContribution.t),
    };
    
}

type t = {
  name: string,
  displayName: string,

  contributes: ExtensionContributes.t,
};
