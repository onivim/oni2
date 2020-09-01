/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require('react');

function Key(props) {
  return <li>
        <kbd>{props.name}</kbd> {" - " + props.description}</li>;
};

const tips = [
{
  title: "Scroll relative to cursor",
  description: "Use zz, zb, and zt to scroll the buffer, relative to cursor position",
  keys: [
    <Key name="zz" description="Center cursor line in viewport" />,
    <Key name="zt" description="Move viewport such that cursor line is at the top" />,
    <Key name="zb" description="Move viewport such that cursor line is at the bottom" />,
  ],
  videoFile: "RodY2O1xFfI"
},
{
  title: "Move up and down a buffer",
  description: "Use gg and G to quickly move up and down the buffer.",
  keys: [
    <Key name="gg" description="move to first line in buffer" />,
    <Key name="G" description="move to last line in buffer" />,
    <Key name="50G" description="move to line 50 in buffer" />,
  ],
  videoFile: "xh4TTWy-okQ"
},
{
  title: "Working with window splits",
  description: "It's easy to create and navigate window splits in Onivim, without touching the mouse.",
  keys: [
    <Key name="Control+w v" description="Create new vertical split" />,
    <Key name="Control+w s" description="Create new horizontal split" />,
    <Key name="Control+w h" description="Move a split left" />,
    <Key name="Control+w j" description="Move a split down" />,
    <Key name="Control+w l" description="Move a split right" />,
    <Key name="Control+w l" description="Move a split up" />,
  ],
  videoFile: "Xpfb2pMmeVg",
},
{
  title: "Edit strings like a Ninja",
  description: "The change operator, combined with the in-string motion, is a powerful combo - letting you strike and edit strings from afar.",
  keys: [
    <Key name="c" description="Change operator" />,
    <Key name={`i"`} description="In string motion" />,
  ],
  videoFile: "DMgNUMfGAQ4",
}
//{
//  title: "Jump around with G",
//  description: <div>
//  <ul>
//    <li><kbd>gg</kbd> - move to top of buffer</li>
//    <li><kbd>G</kbd> - move to bottom of buffer</li>
//    <li><kbd>50G</kbd> - move to line 50</li>
//  </ul>
//  </div>,
//  videoFile: "demo-gg"
//},
//{
//  title: "Edit Strings like a Ninja",
//  description: <div>
//  <ul>
//    <li><kbd>ci"</kbd> - change inside string</li>
//  </ul>
//  </div>,
//  videoFile: "TODO"
//}
];

function Tip(props) {
  return <section className="tip">
    <div className="tip-inner">
      <div className="tip-text">
        <h2>{props.title}</h2>
        <div className="description">
          {props.description}
        </div>
        <h3>Keys Used:</h3>
        <ul children={props.keys} />
      </div>
      <div className="tip-video">
        <iframe
        width="480"
        height="270"
        src={"https://www.youtube.com/embed/" + props.videoFile + "?&autoplay=1&controls=0&modestbranding=1&loop=1&playlist=" + props.videoFile}
        frameBorder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowFullScreen>
        </iframe>
      </div>
    </div>
  </section>
}

function TipContainer(_props) {

  //const {config: siteConfig, language = ''} = props;

  const result = tips.map((tip) => <Tip title={tip.title}
  description={tip.description} keys={tip.keys} videoFile={tip.videoFile} />);

  return <div className="tips">
  {result}
  </div>
}

module.exports = TipContainer;
