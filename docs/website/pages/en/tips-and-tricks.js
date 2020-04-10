/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const React = require('react');


const tips = [
{
  title: "Jump around with G",
  description: <div>
  <ul>
    <li><kbd>gg</kbd> - move to top of buffer</li>
    <li><kbd>G</kbd> - move to bottom of buffer</li>
    <li><kbd>50G</kbd> - move to line 50</li>
  </ul>
  </div>,
  videoFile: "demo-gg"
},
{
  title: "Edit Strings like a Ninja",
  description: <div>
  <ul>
    <li><kbd>ci"</kbd> - change inside string</li>
  </ul>
  </div>,
  videoFile: "TODO"
}
];

function Tip(props) {
  return <section className="tip">
    <div className="tip-inner">
      <div className="tip-text">
        <h2>{props.title}</h2>
        <div className="description">
          {props.description}
        </div>
      </div>
      <div className="tip-video">
        <figure>
          <video height={300} width={400} autoPlay={true} loop={true} muted={true} playsInline={true}>
            <source src={"/vid/" + props.videoFile + ".mp4"} type="video/mp4"/>
            <source src={"/vid/" + props.videoFile + ".webm"} type="video/webm"/>
          </video>
        </figure>
      </div>
    </div>
  </section>
}

function TipContainer(_props) {

  //const {config: siteConfig, language = ''} = props;

  const result = tips.map((tip) => <Tip title={tip.title}
  description={tip.description} videoFile={tip.videoFile} />);

  return <div className="tips">
  {result}
  </div>
}

module.exports = TipContainer;
