"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.getImageSize = void 0;
// Based on @sergeche's work on the emmet plugin for atom
// TODO: Move to https://github.com/emmetio/image-size
const path = require("path");
const http = require("http");
const https = require("https");
const url_1 = require("url");
const sizeOf = require("image-size");
const reUrl = /^https?:/;
/**
 * Get size of given image file. Supports files from local filesystem,
 * as well as URLs
 */
function getImageSize(file) {
    file = file.replace(/^file:\/\//, '');
    return reUrl.test(file) ? getImageSizeFromURL(file) : getImageSizeFromFile(file);
}
exports.getImageSize = getImageSize;
/**
 * Get image size from file on local file system
 */
function getImageSizeFromFile(file) {
    return new Promise((resolve, reject) => {
        const isDataUrl = file.match(/^data:.+?;base64,/);
        if (isDataUrl) {
            // NB should use sync version of `sizeOf()` for buffers
            try {
                const data = Buffer.from(file.slice(isDataUrl[0].length), 'base64');
                return resolve(sizeForFileName('', sizeOf(data)));
            }
            catch (err) {
                return reject(err);
            }
        }
        sizeOf(file, (err, size) => {
            if (err) {
                reject(err);
            }
            else {
                resolve(sizeForFileName(path.basename(file), size));
            }
        });
    });
}
/**
 * Get image size from given remove URL
 */
function getImageSizeFromURL(urlStr) {
    return new Promise((resolve, reject) => {
        const url = url_1.parse(urlStr);
        const getTransport = url.protocol === 'https:' ? https.get : http.get;
        if (!url.pathname) {
            return reject('Given url doesnt have pathname property');
        }
        const urlPath = url.pathname;
        getTransport(url, resp => {
            const chunks = [];
            let bufSize = 0;
            const trySize = (chunks) => {
                try {
                    const size = sizeOf(Buffer.concat(chunks, bufSize));
                    resp.removeListener('data', onData);
                    resp.destroy(); // no need to read further
                    resolve(sizeForFileName(path.basename(urlPath), size));
                }
                catch (err) {
                    // might not have enough data, skip error
                }
            };
            const onData = (chunk) => {
                bufSize += chunk.length;
                chunks.push(chunk);
                trySize(chunks);
            };
            resp
                .on('data', onData)
                .on('end', () => trySize(chunks))
                .once('error', err => {
                resp.removeListener('data', onData);
                reject(err);
            });
        })
            .once('error', reject);
    });
}
/**
 * Returns size object for given file name. If file name contains `@Nx` token,
 * the final dimentions will be downscaled by N
 */
function sizeForFileName(fileName, size) {
    const m = fileName.match(/@(\d+)x\./);
    const scale = m ? +m[1] : 1;
    return {
        realWidth: size.width,
        realHeight: size.height,
        width: Math.floor(size.width / scale),
        height: Math.floor(size.height / scale)
    };
}
//# sourceMappingURL=imageSizeHelper.js.map