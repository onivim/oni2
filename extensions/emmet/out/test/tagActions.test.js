"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
require("mocha");
const assert = require("assert");
const vscode_1 = require("vscode");
const testUtils_1 = require("./testUtils");
const removeTag_1 = require("../removeTag");
const updateTag_1 = require("../updateTag");
const matchTag_1 = require("../matchTag");
const splitJoinTag_1 = require("../splitJoinTag");
const mergeLines_1 = require("../mergeLines");
suite('Tests for Emmet actions on html tags', () => {
    teardown(() => {
        // close all editors
        return testUtils_1.closeAllEditors;
    });
    const contents = `
	<div class="hello">
		<ul>
			<li><span>Hello</span></li>
			<li><span>There</span></li>
			<div><li><span>Bye</span></li></div>
		</ul>
		<span/>
	</div>
	`;
    let contentsWithTemplate = `
	<script type="text/template">
		<ul>
			<li><span>Hello</span></li>
			<li><span>There</span></li>
			<div><li><span>Bye</span></li></div>
		</ul>
		<span/>
	</script>
	`;
    test('update tag with multiple cursors', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<li><section>Hello</section></li>
			<section><span>There</span></section>
			<section><li><span>Bye</span></li></section>
		</ul>
		<span/>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 17, 3, 17),
                new vscode_1.Selection(4, 5, 4, 5),
                new vscode_1.Selection(5, 35, 5, 35),
            ];
            return updateTag_1.updateTag('section').then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    // #region update tag
    test('update tag with entire node selected', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<li><section>Hello</section></li>
			<li><span>There</span></li>
			<section><li><span>Bye</span></li></section>
		</ul>
		<span/>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 7, 3, 25),
                new vscode_1.Selection(5, 3, 5, 39),
            ];
            return updateTag_1.updateTag('section').then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('update tag with template', () => {
        const expectedContents = `
	<script type="text/template">
		<section>
			<li><span>Hello</span></li>
			<li><span>There</span></li>
			<div><li><span>Bye</span></li></div>
		</section>
		<span/>
	</script>
	`;
        return testUtils_1.withRandomFileEditor(contentsWithTemplate, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 4, 2, 4),
            ];
            return updateTag_1.updateTag('section').then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    // #endregion
    // #region remove tag
    test('remove tag with mutliple cursors', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<li>Hello</li>
			<span>There</span>
			<li><span>Bye</span></li>
		</ul>
		<span/>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 17, 3, 17),
                new vscode_1.Selection(4, 5, 4, 5),
                new vscode_1.Selection(5, 35, 5, 35),
            ];
            return removeTag_1.removeTag().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('remove tag with boundary conditions', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<li>Hello</li>
			<li><span>There</span></li>
			<li><span>Bye</span></li>
		</ul>
		<span/>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 7, 3, 25),
                new vscode_1.Selection(5, 3, 5, 39),
            ];
            return removeTag_1.removeTag().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('remove tag with template', () => {
        const expectedContents = `
	<script type="text/template">
\t\t
		<li><span>Hello</span></li>
		<li><span>There</span></li>
		<div><li><span>Bye</span></li></div>
\t
		<span/>
	</script>
	`;
        return testUtils_1.withRandomFileEditor(contentsWithTemplate, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 4, 2, 4),
            ];
            return removeTag_1.removeTag().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    // #endregion
    // #region split/join tag
    test('split/join tag with mutliple cursors', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<li><span/></li>
			<li><span>There</span></li>
			<div><li><span>Bye</span></li></div>
		</ul>
		<span></span>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 17, 3, 17),
                new vscode_1.Selection(7, 5, 7, 5),
            ];
            return splitJoinTag_1.splitJoinTag().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('split/join tag with boundary selection', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<li><span/></li>
			<li><span>There</span></li>
			<div><li><span>Bye</span></li></div>
		</ul>
		<span></span>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 7, 3, 25),
                new vscode_1.Selection(7, 2, 7, 9),
            ];
            return splitJoinTag_1.splitJoinTag().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('split/join tag with templates', () => {
        const expectedContents = `
	<script type="text/template">
		<ul>
			<li><span/></li>
			<li><span>There</span></li>
			<div><li><span>Bye</span></li></div>
		</ul>
		<span></span>
	</script>
	`;
        return testUtils_1.withRandomFileEditor(contentsWithTemplate, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 17, 3, 17),
                new vscode_1.Selection(7, 5, 7, 5),
            ];
            return splitJoinTag_1.splitJoinTag().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('split/join tag in jsx with xhtml self closing tag', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<li><span /></li>
			<li><span>There</span></li>
			<div><li><span>Bye</span></li></div>
		</ul>
		<span></span>
	</div>
	`;
        const oldValueForSyntaxProfiles = vscode_1.workspace.getConfiguration('emmet').inspect('syntaxProfiles');
        return vscode_1.workspace.getConfiguration('emmet').update('syntaxProfiles', { jsx: { selfClosingStyle: 'xhtml' } }, vscode_1.ConfigurationTarget.Global).then(() => {
            return testUtils_1.withRandomFileEditor(contents, 'jsx', (editor, doc) => {
                editor.selections = [
                    new vscode_1.Selection(3, 17, 3, 17),
                    new vscode_1.Selection(7, 5, 7, 5),
                ];
                return splitJoinTag_1.splitJoinTag().then(() => {
                    assert.equal(doc.getText(), expectedContents);
                    return vscode_1.workspace.getConfiguration('emmet').update('syntaxProfiles', oldValueForSyntaxProfiles ? oldValueForSyntaxProfiles.globalValue : undefined, vscode_1.ConfigurationTarget.Global);
                });
            });
        });
    });
    // #endregion
    // #region match tag
    test('match tag with mutliple cursors', () => {
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, _) => {
            editor.selections = [
                new vscode_1.Selection(1, 0, 1, 0),
                new vscode_1.Selection(1, 1, 1, 1),
                new vscode_1.Selection(1, 2, 1, 2),
                new vscode_1.Selection(1, 6, 1, 6),
                new vscode_1.Selection(1, 18, 1, 18),
                new vscode_1.Selection(1, 19, 1, 19),
            ];
            matchTag_1.matchTag();
            editor.selections.forEach(selection => {
                assert.equal(selection.active.line, 8);
                assert.equal(selection.active.character, 3);
                assert.equal(selection.anchor.line, 8);
                assert.equal(selection.anchor.character, 3);
            });
            return Promise.resolve();
        });
    });
    test('match tag with template scripts', () => {
        let templateScript = `
	<script type="text/template">
		<div>
			Hello
		</div>
	</script>`;
        return testUtils_1.withRandomFileEditor(templateScript, 'html', (editor, _) => {
            editor.selections = [
                new vscode_1.Selection(2, 2, 2, 2),
            ];
            matchTag_1.matchTag();
            editor.selections.forEach(selection => {
                assert.equal(selection.active.line, 4);
                assert.equal(selection.active.character, 4);
                assert.equal(selection.anchor.line, 4);
                assert.equal(selection.anchor.character, 4);
            });
            return Promise.resolve();
        });
    });
    // #endregion
    // #region merge lines
    test('merge lines of tag with children when empty selection', () => {
        const expectedContents = `
	<div class="hello">
		<ul><li><span>Hello</span></li><li><span>There</span></li><div><li><span>Bye</span></li></div></ul>
		<span/>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 3, 2, 3)
            ];
            return mergeLines_1.mergeLines().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('merge lines of tag with children when full node selection', () => {
        const expectedContents = `
	<div class="hello">
		<ul><li><span>Hello</span></li><li><span>There</span></li><div><li><span>Bye</span></li></div></ul>
		<span/>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 3, 6, 7)
            ];
            return mergeLines_1.mergeLines().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('merge lines is no-op when start and end nodes are on the same line', () => {
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 9, 3, 9),
                new vscode_1.Selection(4, 5, 4, 5),
                new vscode_1.Selection(5, 5, 5, 20) // selection spans multiple nodes in the same line
            ];
            return mergeLines_1.mergeLines().then(() => {
                assert.equal(doc.getText(), contents);
                return Promise.resolve();
            });
        });
    });
    // #endregion
});
//# sourceMappingURL=tagActions.test.js.map