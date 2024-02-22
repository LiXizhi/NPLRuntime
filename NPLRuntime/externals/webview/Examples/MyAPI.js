/* Desc: This file is used for old PPT page in paracraft. It is only used when webview2 is available.
  Example:
https://keepwork.com/official/open/apps/video?video_url=https://api.keepwork.com/ts-storage/siteFiles/26256/raw#1681717764392physics_demo_small.mp4
  
NPL.activate("video.page", {cmd="pause"})
NPL.activate("video.page", {cmd="play"})
*/

const queryString = window.location.search;
const urlParams = new URLSearchParams(queryString);
const video_url = urlParams.get('video_url')

var activate = function(msg){
  if(msg.cmd === "pause"){
    document.getElementById("myPlayer").pause()
  }
  else if(msg.cmd === "play"){
    document.getElementById("myPlayer").play()
  }
};

window.NPL.this(activate, { filename: 'video.page' });
// NPL.activate("NplBrowserPlugin", {cmd: "log", msg: "video is loaded", url: video_url})