"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.binarySearch = exports.mergeSort = exports.contains = exports.pushAll = void 0;
function pushAll(to, from) {
    if (from) {
        for (const e of from) {
            to.push(e);
        }
    }
}
exports.pushAll = pushAll;
function contains(arr, val) {
    return arr.indexOf(val) !== -1;
}
exports.contains = contains;
/**
 * Like `Array#sort` but always stable. Usually runs a little slower `than Array#sort`
 * so only use this when actually needing stable sort.
 */
function mergeSort(data, compare) {
    _divideAndMerge(data, compare);
    return data;
}
exports.mergeSort = mergeSort;
function _divideAndMerge(data, compare) {
    if (data.length <= 1) {
        // sorted
        return;
    }
    const p = (data.length / 2) | 0;
    const left = data.slice(0, p);
    const right = data.slice(p);
    _divideAndMerge(left, compare);
    _divideAndMerge(right, compare);
    let leftIdx = 0;
    let rightIdx = 0;
    let i = 0;
    while (leftIdx < left.length && rightIdx < right.length) {
        let ret = compare(left[leftIdx], right[rightIdx]);
        if (ret <= 0) {
            // smaller_equal -> take left to preserve order
            data[i++] = left[leftIdx++];
        }
        else {
            // greater -> take right
            data[i++] = right[rightIdx++];
        }
    }
    while (leftIdx < left.length) {
        data[i++] = left[leftIdx++];
    }
    while (rightIdx < right.length) {
        data[i++] = right[rightIdx++];
    }
}
function binarySearch(array, key, comparator) {
    let low = 0, high = array.length - 1;
    while (low <= high) {
        let mid = ((low + high) / 2) | 0;
        let comp = comparator(array[mid], key);
        if (comp < 0) {
            low = mid + 1;
        }
        else if (comp > 0) {
            high = mid - 1;
        }
        else {
            return mid;
        }
    }
    return -(low + 1);
}
exports.binarySearch = binarySearch;
//# sourceMappingURL=arrays.js.map