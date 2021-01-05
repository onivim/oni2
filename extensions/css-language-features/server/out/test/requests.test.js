"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
require("mocha");
const assert = require("assert");
const requests_1 = require("../requests");
suite('requests', () => {
    test('join', async function () {
        assert.equal(requests_1.joinPath('foo://a/foo/bar', 'x'), 'foo://a/foo/bar/x');
        assert.equal(requests_1.joinPath('foo://a/foo/bar/', 'x'), 'foo://a/foo/bar/x');
        assert.equal(requests_1.joinPath('foo://a/foo/bar/', '/x'), 'foo://a/foo/bar/x');
        assert.equal(requests_1.joinPath('foo://a/foo/bar/', 'x/'), 'foo://a/foo/bar/x/');
        assert.equal(requests_1.joinPath('foo://a/foo/bar/', 'x', 'y'), 'foo://a/foo/bar/x/y');
        assert.equal(requests_1.joinPath('foo://a/foo/bar/', 'x/', '/y'), 'foo://a/foo/bar/x/y');
        assert.equal(requests_1.joinPath('foo://a/foo/bar/', '.', '/y'), 'foo://a/foo/bar/y');
        assert.equal(requests_1.joinPath('foo://a/foo/bar/', 'x/y/z', '..'), 'foo://a/foo/bar/x/y');
    });
    test('resolve', async function () {
        assert.equal(requests_1.resolvePath('foo://a/foo/bar', 'x'), 'foo://a/foo/bar/x');
        assert.equal(requests_1.resolvePath('foo://a/foo/bar/', 'x'), 'foo://a/foo/bar/x');
        assert.equal(requests_1.resolvePath('foo://a/foo/bar/', '/x'), 'foo://a/x');
        assert.equal(requests_1.resolvePath('foo://a/foo/bar/', 'x/'), 'foo://a/foo/bar/x/');
    });
    test('normalize', async function () {
        function assertNormalize(path, expected) {
            assert.equal(requests_1.normalizePath(path.split('/')), expected, path);
        }
        assertNormalize('a', 'a');
        assertNormalize('/a', '/a');
        assertNormalize('a/', 'a/');
        assertNormalize('a/b', 'a/b');
        assertNormalize('/a/foo/bar/x', '/a/foo/bar/x');
        assertNormalize('/a/foo/bar//x', '/a/foo/bar/x');
        assertNormalize('/a/foo/bar///x', '/a/foo/bar/x');
        assertNormalize('/a/foo/bar/x/', '/a/foo/bar/x/');
        assertNormalize('a/foo/bar/x/', 'a/foo/bar/x/');
        assertNormalize('a/foo/bar/x//', 'a/foo/bar/x/');
        assertNormalize('//a/foo/bar/x//', '/a/foo/bar/x/');
        assertNormalize('a/.', 'a');
        assertNormalize('a/./b', 'a/b');
        assertNormalize('a/././b', 'a/b');
        assertNormalize('a/n/../b', 'a/b');
        assertNormalize('a/n/../', 'a/');
        assertNormalize('a/n/../', 'a/');
        assertNormalize('/a/n/../..', '/');
        assertNormalize('..', '');
        assertNormalize('/..', '/');
    });
    test('extname', async function () {
        function assertExtName(input, expected) {
            assert.equal(requests_1.extname(input), expected, input);
        }
        assertExtName('foo://a/foo/bar', '');
        assertExtName('foo://a/foo/bar.foo', '.foo');
        assertExtName('foo://a/foo/.foo', '');
    });
});
//# sourceMappingURL=requests.test.js.map