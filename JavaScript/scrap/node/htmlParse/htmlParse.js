#!/usr/bin/env node

// There are too many god-damned node html parsers, and they are disgustingly incestuous.
// Some common ones are:
//  parse5            slower but "the only fully-compliant parser"
//  htmlparser2       pretty complete and fast, but big (multi-module)
//  html5parser       only valid html5
//  node-html-parser  fast good enough
//  fast-html-parser  fast older
//  htmljs-parser     fastest but difficult to use
//
// POPULARITY
// https://npmtrends.com/fast-html-parser-vs-htmlparser2-vs-node-html-parser-vs-parse5
// parse 5, htmlparser2
// 
// PERFORMANCE
// see: https://github.com/AndreasMadsen/htmlparser-benchmark
// htmljs-parser      : 0.821362 ms/file ± 1.82019
// tl                 : 1.07163 ms/file ± 1.18989
// html5parser        : 2.27917 ms/file ± 3.17972
// htmlparser2        : 2.37383 ms/file ± 3.53446
// neutron-html5parser: 2.83331 ms/file ± 1.62381
// node-html-parser   : 2.95525 ms/file ± 1.96684
// htmlparser2-dom    : 3.14310 ms/file ± 4.23269
// html-dom-parser    : 3.16142 ms/file ± 4.17564
// libxmljs           : 4.18759 ms/file ± 3.04574
// zeed-dom           : 4.79023 ms/file ± 2.94051
// parse5             : 7.02438 ms/file ± 8.09516
// sax                : 10.0438 ms/file ± 10.0932
// arijs-stream       : 18.4431 ms/file ± 16.7461
// arijs-tree         : 20.4603 ms/file ± 19.2857
// htmlparser         : 20.6097 ms/file ± 146.114
// html-parser        : 27.5719 ms/file ± 21.0579
// saxes              : 58.0260 ms/file ± 164.583
// html5              : 109.741 ms/file ± 146.367

// ----------------
// node-html-parser
// ----------------
import { parse } from 'node-html-parser';

import * as fs from 'fs';

const sampleHtml = fs.readFileSync('./htmlSample.html', 'utf8');

let doc = parse( sampleHtml );
const resultsTable = doc.querySelector( '#gvSearchResults' ) ?? {};

console.log( `Table childNodes length ${resultsTable.childNodes.length}`)

let passed = true;
resultsTable.childNodes.forEach( r => {
  // parse() adds extra child nodes for \t bits and whatnot.  Ignore them.
  if ( r.rawTagName === 'tr' ) {
    // The name is in an <a> tag in the first <td>.
    // rawText will extract the text of the <a> tag, just what we need here.
    console.log( r.childNodes.find( c => c.rawTagName === 'td' ).rawText );
  }
});
// ----------------

// ----------------
// parse5
// ----------------
// This thing is hugely popular.
// But fuck it's not easy to use.
// But... none of them are.
// ----------------
// import * as parse5 from 'parse5';
// import * as fs from 'fs';
// const sampleHtml = fs.readFileSync('./htmlSample.html', 'utf8');
// let doc = parse5.parse( sampleHtml );
// // ...?
// ----------------
