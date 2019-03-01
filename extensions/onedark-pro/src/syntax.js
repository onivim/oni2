const _ = require('lodash')

const colorConfig = require('./color.json')

const configFactory = type => {
  const colorObj = colorConfig[type]
  let tokenColorsBold

  /**
   * Overwrites for theme type "bold"
   */
  if (type === 'bold') {
    tokenColorsBold = [
      {
        name: 'Markup: Heading',
        scope: 'markup.heading',
        settings: {
          fontStyle: 'bold'
        }
      },
      {
        name: 'Markup: Strong',
        scope: 'markup.bold',
        settings: {
          fontStyle: 'bold'
        }
      },
      {
        name: 'Sections',
        scope: 'entity.name.section',
        settings: {
          fontStyle: 'bold'
        }
      },
      {
        name: 'CSS: Important Keyword',
        scope: 'keyword.other.important',
        settings: {
          fontStyle: 'bold'
        }
      },
      {
        name: 'Functions',
        scope: [
          'entity.name.function',
          'meta.require',
          'support.function.any-method',
          'variable.function'
        ],
        settings: {
          fontStyle: 'bold',
          foreground: colorObj['malibu']
        }
      },
      {
        name: 'Classes',
        scope: 'entity.name.type.namespace',
        settings: {
          fontStyle: 'bold',
          foreground: colorObj['chalky']
        }
      }
    ]
  }

  /**
   * Default theme settings
   */
  const tokenColorsDefault = [
    {
      name: 'support.constant.edge',
      scope: 'support.constant.edge',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'regexp constant character-class',
      scope: 'constant.other.character-class.regexp',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'regexp operator.quantifier',
      scope: 'keyword.operator.quantifier.regexp',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'punctuation.definition',
      scope:
        'punctuation.definition.string.begin,punctuation.definition.string.end',
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: 'Text',
      scope: 'variable.parameter.function',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'Comments',
      scope: 'comment, punctuation.definition.comment',
      settings: {
        foreground: colorObj['lightDark'],
        fontStyle: 'italic'
      }
    },
    {
      name: 'Comment Markup Link',
      scope: 'comment markup.link',
      settings: {
        foreground: colorObj['dark']
      }
    },
    {
      name: 'markup diff',
      scope: 'markup.changed.diff',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'diff',
      scope: 'meta.diff.header.from-file,punctuation.definition.from-file.diff',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'inserted.diff',
      scope: 'markup.inserted.diff',
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: 'deleted.diff',
      scope: 'markup.deleted.diff',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'c++ function',
      scope: 'meta.function.c',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'c++ block',
      scope:
        'punctuation.section.block.begin.bracket.curly.cpp,punctuation.section.block.end.bracket.curly.cpp,punctuation.terminator.statement.c,punctuation.section.block.begin.bracket.curly.c,punctuation.section.block.end.bracket.curly.c,punctuation.section.parens.begin.bracket.round.c,punctuation.section.parens.end.bracket.round.c,punctuation.section.parameters.begin.bracket.round.c,punctuation.section.parameters.end.bracket.round.c',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'js/ts punctuation separator key-value',
      scope: 'punctuation.separator.key-value',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'js/ts italic',
      scope:
        'entity.other.attribute-name.js,entity.other.attribute-name.ts,entity.other.attribute-name.jsx,entity.other.attribute-name.tsx,variable.parameter,variable.language.super',
      settings: {
        fontStyle: 'italic'
      }
    },
    {
      name: 'js/ts import keyword',
      scope: 'keyword.operator.expression.import',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'math js/ts',
      scope: 'support.constant.math',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'math property js/ts',
      scope: 'support.constant.property.math',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'js/ts variable.other.constant',
      scope: 'variable.other.constant',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'java type',
      scope: 'storage.type.annotation.java',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'java source',
      scope: 'source.java',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'java modifier.import',
      scope:
        'punctuation.section.block.begin.java,punctuation.section.block.end.java,punctuation.definition.method-parameters.begin.java,punctuation.definition.method-parameters.end.java,meta.method.identifier.java,punctuation.section.method.begin.java,punctuation.section.method.end.java,punctuation.terminator.java,punctuation.section.class.begin.java,punctuation.section.class.end.java,punctuation.section.inner-class.begin.java,punctuation.section.inner-class.end.java,meta.method-call.java,storage.type.generic.java,punctuation.section.class.begin.bracket.curly.java,punctuation.section.class.end.bracket.curly.java,punctuation.section.method.begin.bracket.curly.java,punctuation.section.method.end.bracket.curly.java,punctuation.separator.period.java,meta.method.body.java',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'java modifier.import',
      scope: 'meta.method.java',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'java modifier.import',
      scope: 'storage.modifier.import.java,storage.type.java',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'java variable.name',
      scope: 'meta.definition.variable.name.java',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'operator logical',
      scope: 'keyword.operator.logical.js',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'operator bitwise',
      scope: 'keyword.operator.bitwise',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'operator channel',
      scope: 'keyword.operator.channel',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'support.constant.property-value.scss',
      scope:
        'support.constant.property-value.scss,support.constant.property-value.css',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'CSS/SCSS/LESS Operators',
      scope: 'keyword.operator.css,keyword.operator.scss,keyword.operator.less',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'css color standard name',
      scope:
        'support.constant.color.w3c-standard-color-name.css,support.constant.color.w3c-standard-color-name.scss',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'css comma',
      scope: 'punctuation.separator.list.comma.css',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'css attribute-name.id',
      scope: 'support.constant.color.w3c-standard-color-name.css',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'css property-name',
      scope: 'support.type.vendored.property-name.css',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'js/ts template-expression',
      scope:
        'punctuation.definition.template-expression.begin,punctuation.definition.template-expression.end',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'js/ts module',
      scope:
        'support.module.node,support.type.object.module,support.module.node',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'entity.name.type.module',
      scope: 'entity.name.type.module',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'js variable readwrite',
      scope:
        'variable.other.readwrite,meta.object-literal.key,support.variable.property,support.variable.object.process,support.variable.object.node',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'comment',
      scope: 'comment.line.double-slash,comment.block.documentation',
      settings: {
        fontStyle: 'italic'
      }
    },
    {
      name: 'js/ts json',
      scope: 'support.constant.json',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'js/ts Keyword',
      scope: [
        'keyword.operator.expression.instanceof',
        'keyword.operator.new',
        'keyword.operator.ternary',
        'keyword.operator.optional'
      ],
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'js/ts console',
      scope: 'support.type.object.console',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'js/ts support.variable.property.process',
      scope: 'support.variable.property.process',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'js console function',
      scope: 'entity.name.function,support.function.console',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'js operator',
      scope: 'keyword.operator',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'js dom',
      scope: 'support.type.object.dom',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'js dom variable',
      scope: 'support.variable.dom,support.variable.property.dom',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'keyword.operator',
      scope:
        'keyword.operator.arithmetic,keyword.operator.comparison,keyword.operator.decrement,keyword.operator.increment',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'C operator assignment',
      scope:
        'keyword.operator.assignment.c,keyword.operator.comparison.c,keyword.operator.c,keyword.operator.increment.c,keyword.operator.decrement.c,keyword.operator.bitwise.shift.c',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'Punctuation',
      scope: 'punctuation.separator.delimiter',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'Other punctuation .c',
      scope: 'punctuation.separator.c',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'C type posix-reserved',
      scope: 'support.type.posix-reserved.c',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'keyword.operator.sizeof.c',
      scope: 'keyword.operator.sizeof.c',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'python parameter',
      scope: 'variable.parameter.function.language.python',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'python type',
      scope: 'support.type.python',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'python logical',
      scope: 'keyword.operator.logical.python',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'meta.function-call.arguments.python',
      scope: 'meta.function-call.arguments.python',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'meta.function-call.python',
      scope: 'meta.function-call.python',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'pyCs',
      scope: 'variable.parameter.function.python',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'python block',
      scope:
        'punctuation.definition.arguments.begin.python,punctuation.definition.arguments.end.python,punctuation.separator.arguments.python,punctuation.definition.list.begin.python,punctuation.definition.list.end.python,meta.function-call.arguments.python',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'python function-call.generic',
      scope: 'meta.function-call.generic.python',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'python placeholder reset to normal string',
      scope: 'constant.character.format.placeholder.other.python',
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: 'Delimiters',
      scope: 'none',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'Operators',
      scope: 'keyword.operator',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'Compound Assignment Operators',
      scope: 'keyword.operator.assignment.compound',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'Keywords',
      scope: 'keyword',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'Variables',
      scope: 'variable',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'Java Variables',
      scope: 'token.variable.parameter.java',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'Java Imports',
      scope: 'import.storage.java',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Packages',
      scope: 'token.package.keyword',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'Packages',
      scope: 'token.package',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'Functions',
      scope: [
        'entity.name.function',
        'meta.require',
        'support.function.any-method',
        'variable.function'
      ],
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'Classes',
      scope: 'entity.name.type.namespace',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Classes',
      scope: 'support.class, entity.name.type.class',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Class name',
      scope: 'entity.name.class.identifier.namespace.type',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Class name',
      scope: [
        'entity.name.class',
        'variable.other.class.js',
        'variable.other.class.ts'
      ],
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Class name php',
      scope: 'variable.other.class.php',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'Type Name',
      scope: 'entity.name.type',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Keyword Control',
      scope: 'keyword.control',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'Python Keyword Control',
      scope: 'keyword.control.import.python,keyword.control.flow.python',
      settings: {
        fontStyle: 'italic'
      }
    },
    {
      name: 'Control Elements',
      scope: 'control.elements, keyword.operator.less',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'Methods',
      scope: 'keyword.other.special-method',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'Storage',
      scope: 'storage',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'Storage JS TS',
      scope: 'token.storage',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name:
        'Source Js Keyword Operator Delete,source Js Keyword Operator In,source Js Keyword Operator Of,source Js Keyword Operator Instanceof,source Js Keyword Operator New,source Js Keyword Operator Typeof,source Js Keyword Operator Void',
      scope:
        'keyword.operator.expression.delete,keyword.operator.expression.in,keyword.operator.expression.of,keyword.operator.expression.instanceof,keyword.operator.new,keyword.operator.expression.typeof,keyword.operator.expression.void',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'Java Storage',
      scope: 'token.storage.type.java',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Support',
      scope: 'support.function',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'Support type',
      scope: 'support.type.property-name',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'Support type',
      scope: 'support.constant.property-value',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'Support type',
      scope: 'support.constant.font-name',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'Meta tag',
      scope: 'meta.tag',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'Strings, Inherited Class',
      scope: 'string, entity.other.inherited-class',
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: 'Constant other symbol',
      scope: 'constant.other.symbol',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'Integers',
      scope: 'constant.numeric',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'Floats',
      scope: 'none',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'Boolean',
      scope: 'none',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'Constants',
      scope: 'constant',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'Constants',
      scope: 'punctuation.definition.constant',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'Tags',
      scope: 'entity.name.tag',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'Attributes',
      scope: 'entity.other.attribute-name',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'Attribute IDs',
      scope: 'entity.other.attribute-name.id',
      settings: {
        foreground: colorObj['malibu'],
        fontStyle: 'normal'
      }
    },
    {
      name: 'Attribute class',
      scope: 'entity.other.attribute-name.class.css',
      settings: {
        foreground: colorObj['whiskey'],
        fontStyle: 'normal'
      }
    },
    {
      name: 'Selector',
      scope: 'meta.selector',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'Values',
      scope: 'none',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'Headings',
      scope: 'markup.heading',
      settings: {
        fontStyle: 'bold',
        foreground: colorObj['coral']
      }
    },
    {
      name: 'Headings',
      scope:
        'markup.heading punctuation.definition.heading, entity.name.section',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'Units',
      scope: 'keyword.other.unit',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'Bold',
      scope: 'markup.bold,todo.bold',
      settings: {
        fontStyle: 'bold',
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'Bold',
      scope: 'punctuation.definition.bold',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Italic',
      scope: 'markup.italic, punctuation.definition.italic,todo.emphasis',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'Italic',
      scope: 'emphasis md',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown headings',
      scope: 'entity.name.section.markdown',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown heading Punctuation Definition',
      scope: 'punctuation.definition.heading.markdown',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'punctuation.definition.list.begin.markdown',
      scope: 'punctuation.definition.list.begin.markdown',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown heading setext',
      scope: 'markup.heading.setext',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown Punctuation Definition Bold',
      scope: 'punctuation.definition.bold.markdown',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown Inline Raw',
      scope: 'markup.inline.raw.markdown',
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown Inline Raw',
      scope: 'markup.inline.raw.string.markdown',
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown List Punctuation Definition',
      scope: 'punctuation.definition.list.markdown',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown Quote',
      scope: 'markup.quote.markdown',
      settings: {
        foreground: colorObj['dark'],
        fontStyle: 'italic'
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown Punctuation Definition String',
      scope: [
        'punctuation.definition.string.begin.markdown',
        'punctuation.definition.string.end.markdown',
        'punctuation.definition.metadata.markdown'
      ],
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'beginning.punctuation.definition.list.markdown',
      scope: ['beginning.punctuation.definition.list.markdown'],
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown Punctuation Definition Link',
      scope: 'punctuation.definition.metadata.markdown',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown Underline Link/Image',
      scope:
        'markup.underline.link.markdown,markup.underline.link.image.markdown',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: '[VSCODE-CUSTOM] Markdown Link Title/Description',
      scope:
        'string.other.link.title.markdown,string.other.link.description.markdown',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'markup.italic.markdown',
      scope: 'markup.italic.markdown',
      settings: {
        fontStyle: 'italic'
      }
    },
    {
      name: 'markup.bold.markdown',
      scope: 'markup.bold.markdown',
      settings: {
        fontStyle: 'bold'
      }
    },
    {
      name: 'Regular Expressions',
      scope: 'string.regexp',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'Escape Characters',
      scope: 'constant.character.escape',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'Embedded',
      scope: 'punctuation.section.embedded, variable.interpolation',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'Illegal',
      scope: 'invalid.illegal',
      settings: {
        foreground: colorObj['invalid']
      }
    },
    {
      name: 'Broken',
      scope: 'invalid.broken',
      settings: {
        foreground: colorObj['invalid']
      }
    },
    {
      name: 'Deprecated',
      scope: 'invalid.deprecated',
      settings: {
        foreground: colorObj['invalid']
      }
    },
    {
      name: 'Unimplemented',
      scope: 'invalid.unimplemented',
      settings: {
        foreground: colorObj['invalid']
      }
    },
    {
      name: 'Source Json Meta Structure Dictionary Json > String Quoted Json',
      scope: 'source.json meta.structure.dictionary.json > string.quoted.json',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name:
        'Source Json Meta Structure Dictionary Json > String Quoted Json > Punctuation String',
      scope:
        'source.json meta.structure.dictionary.json > string.quoted.json > punctuation.string',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name:
        'Source Json Meta Structure Dictionary Json > Value Json > String Quoted Json,source Json Meta Structure Array Json > Value Json > String Quoted Json,source Json Meta Structure Dictionary Json > Value Json > String Quoted Json > Punctuation,source Json Meta Structure Array Json > Value Json > String Quoted Json > Punctuation',
      scope:
        'source.json meta.structure.dictionary.json > value.json > string.quoted.json,source.json meta.structure.array.json > value.json > string.quoted.json,source.json meta.structure.dictionary.json > value.json > string.quoted.json > punctuation,source.json meta.structure.array.json > value.json > string.quoted.json > punctuation',
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name:
        'Source Json Meta Structure Dictionary Json > Constant Language Json,source Json Meta Structure Array Json > Constant Language Json',
      scope:
        'source.json meta.structure.dictionary.json > constant.language.json,source.json meta.structure.array.json > constant.language.json',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: '[VSCODE-CUSTOM] JSON Property Name',
      scope: 'support.type.property-name.json',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: '[VSCODE-CUSTOM] JSON Punctuation for Property Name',
      scope: 'support.type.property-name.json punctuation',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'laravel blade tag',
      scope:
        'text.html.laravel-blade source.php.embedded.line.html entity.name.tag.laravel-blade',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'laravel blade @',
      scope:
        'text.html.laravel-blade source.php.embedded.line.html support.constant.laravel-blade',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'use statement for other classes',
      scope:
        'support.other.namespace.use.php,support.other.namespace.use-as.php,support.other.namespace.php,entity.other.alias.php,meta.interface.php',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'error suppression',
      scope: 'keyword.operator.error-control.php',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'php instanceof',
      scope: 'keyword.operator.type.php',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'style double quoted array index normal begin',
      scope: 'punctuation.section.array.begin.php',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'style double quoted array index normal end',
      scope: 'punctuation.section.array.end.php',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'php illegal.non-null-typehinted',
      scope: 'invalid.illegal.non-null-typehinted.php',
      settings: {
        foreground: colorObj['error']
      }
    },
    {
      name: 'php types',
      scope:
        'storage.type.php,meta.other.type.phpdoc.php,keyword.other.type.php,keyword.other.array.phpdoc.php',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'php call-function',
      scope:
        'meta.function-call.php,meta.function-call.object.php,meta.function-call.static.php',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'php function-resets',
      scope:
        'punctuation.definition.parameters.begin.bracket.round.php,punctuation.definition.parameters.end.bracket.round.php,punctuation.separator.delimiter.php,punctuation.section.scope.begin.php,punctuation.section.scope.end.php,punctuation.terminator.expression.php,punctuation.definition.arguments.begin.bracket.round.php,punctuation.definition.arguments.end.bracket.round.php,punctuation.definition.storage-type.begin.bracket.round.php,punctuation.definition.storage-type.end.bracket.round.php,punctuation.definition.array.begin.bracket.round.php,punctuation.definition.array.end.bracket.round.php,punctuation.definition.begin.bracket.round.php,punctuation.definition.end.bracket.round.php,punctuation.definition.begin.bracket.curly.php,punctuation.definition.end.bracket.curly.php,punctuation.definition.section.switch-block.end.bracket.curly.php,punctuation.definition.section.switch-block.start.bracket.curly.php,punctuation.definition.section.switch-block.begin.bracket.curly.php,punctuation.definition.section.switch-block.end.bracket.curly.php',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'support php constants',
      scope:
        'support.constant.ext.php,support.constant.std.php,support.constant.core.php,support.constant.parser-token.php',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'php goto',
      scope: 'entity.name.goto-label.php,support.other.php',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'php logical/bitwise operator',
      scope:
        'keyword.operator.logical.php,keyword.operator.bitwise.php,keyword.operator.arithmetic.php',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'php regexp operator',
      scope: 'keyword.operator.regexp.php',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'php comparison',
      scope: 'keyword.operator.comparison.php',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'php dollar sign',
      scope: 'punctuation.definition.variable.php',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'php heredoc/nowdoc',
      scope: 'keyword.operator.heredoc.php,keyword.operator.nowdoc.php',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'python function decorator @',
      scope: 'meta.function.decorator.python',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'python function support',
      scope:
        'support.token.decorator.python,meta.function.decorator.identifier.python',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'parameter function',
      scope: 'function.parameter',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'parameter function js/ts',
      scope: 'function.parameter',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'brace function',
      scope: 'function.brace',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'parameter function ruby cs',
      scope: 'function.parameter.ruby, function.parameter.cs',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'constant.language.symbol.ruby',
      scope: 'constant.language.symbol.ruby',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'rgb-value',
      scope: 'rgb-value',
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'rgb value',
      scope: 'inline-color-decoration rgb-value',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'rgb value less',
      scope: 'less rgb-value',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'sass selector',
      scope: 'selector.sass',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'js ts this',
      scope:
        'var.this,variable.language.this.js,variable.language.this.ts,variable.language.this.jsx,variable.language.this.tsx',
      settings: {
        foreground: colorObj['chalky'],
        fontStyle: 'italic'
      }
    },
    {
      name: 'ts primitive/builtin types',
      scope:
        'support.type.primitive.ts,support.type.builtin.ts,support.type.primitive.tsx,support.type.builtin.tsx',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'block scope',
      scope: 'block.scope.end,block.scope.begin',
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'cs storage type',
      scope: 'storage.type.cs',
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'cs local variable',
      scope: 'entity.name.variable.local.cs',
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      scope: 'token.info-token',
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      scope: 'token.warn-token',
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      scope: 'token.error-token',
      settings: {
        foreground: colorObj['error']
      }
    },
    {
      scope: 'token.debug-token',
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'String interpolation',
      scope: [
        'punctuation.definition.template-expression.begin',
        'punctuation.definition.template-expression.end',
        'punctuation.section.embedded'
      ],
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'Reset JavaScript string interpolation expression',
      scope: ['meta.template.expression'],
      settings: {
        foreground: colorObj['lightWhite']
      }
    },
    {
      name: 'Import module JS',
      scope: ['keyword.operator.module'],
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'js Flowtype',
      scope: ['support.type.type.flowtype'],
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'js Flow',
      scope: ['support.type.primitive'],
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'js class prop',
      scope: ['meta.property.object'],
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'js func parameter',
      scope: ['variable.parameter.function.js'],
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'js template literals begin',
      scope: ['keyword.other.template.begin'],
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: 'js template literals end',
      scope: ['keyword.other.template.end'],
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: 'js template literals variable braces begin',
      scope: ['keyword.other.substitution.begin'],
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: 'js template literals variable braces end',
      scope: ['keyword.other.substitution.end'],
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: 'js operator.assignment',
      scope: ['keyword.operator.assignment'],
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'go operator',
      scope: ['keyword.operator.assignment.go', 'keyword.operator.address.go'],
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Go package name',
      scope: ['entity.name.package.go'],
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'elm prelude',
      scope: ['support.type.prelude.elm'],
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'elm constant',
      scope: ['support.constant.elm'],
      settings: {
        foreground: colorObj['whiskey']
      }
    },
    {
      name: 'template literal',
      scope: ['punctuation.quasi.element'],
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'html/pug (jade) escaped characters and entities',
      scope: ['constant.character.entity'],
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name:
        'styling css pseudo-elements/classes to be able to differentiate from classes which are the same colour',
      scope: [
        'entity.other.attribute-name.pseudo-element',
        'entity.other.attribute-name.pseudo-class'
      ],
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'Clojure globals',
      scope: ['entity.global.clojure'],
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Clojure symbols',
      scope: ['meta.symbol.clojure'],
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'Clojure constants',
      scope: ['constant.keyword.clojure'],
      settings: {
        foreground: colorObj['fountainBlue']
      }
    },
    {
      name: 'CoffeeScript Function Argument',
      scope: ['meta.arguments.coffee', 'variable.parameter.function.coffee'],
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'Ini Default Text',
      scope: ['source.ini'],
      settings: {
        foreground: colorObj['green']
      }
    },
    {
      name: 'Makefile prerequisities',
      scope: ['meta.scope.prerequisites.makefile'],
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'Makefile text colour',
      scope: ['source.makefile'],
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Groovy import names',
      scope: ['storage.modifier.import.groovy'],
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'Groovy Methods',
      scope: ['meta.method.groovy'],
      settings: {
        foreground: colorObj['malibu']
      }
    },
    {
      name: 'Groovy Variables',
      scope: ['meta.definition.variable.name.groovy'],
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'Groovy Inheritance',
      scope: ['meta.definition.class.inherited.classes.groovy'],
      settings: { foreground: colorObj['green'] }
    },
    {
      name: 'HLSL Semantic',
      scope: ['support.variable.semantic.hlsl'],
      settings: {
        foreground: colorObj['chalky']
      }
    },
    {
      name: 'HLSL Types',
      scope: [
        'support.type.texture.hlsl',
        'support.type.sampler.hlsl',
        'support.type.object.hlsl',
        'support.type.object.rw.hlsl',
        'support.type.fx.hlsl',
        'support.type.object.hlsl'
      ],
      settings: {
        foreground: colorObj['purple']
      }
    },
    {
      name: 'SQL Variables',
      scope: ['text.variable', 'text.bracketed'],
      settings: {
        foreground: colorObj['coral']
      }
    },
    {
      name: 'types',
      scope: ['support.type.swift', 'support.type.vb.asp'],
      settings: {
        foreground: colorObj['chalky']
      }
    }
  ]

  // add ability to generate custom syntax settings per layout type
  switch (type) {
    case 'bold':
      return {
        // consider name & scope for deduping
        tokenColors: _.uniqBy(
          [...tokenColorsBold, ...tokenColorsDefault],
          setting => {
            return setting.name + setting.scope
          }
        )
      }
    default:
      return {
        tokenColors: tokenColorsDefault
      }
  }
}

module.exports = {
  classic: configFactory('classic'),
  bold: configFactory('bold'),
  vivid: configFactory('vivid')
}
