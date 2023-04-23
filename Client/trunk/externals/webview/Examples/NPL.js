// NPL helper class for communication
allMsg = {};

var GetSystem = function(){
  const { userAgent } = navigator;

  if (userAgent.match('Macintosh')) {
    return 'macos';
  }
  if (userAgent.match('Windows')) {
    return 'windows';
  }
  if (userAgent.match('Android')) {
    return 'android';
  }
  if (userAgent.match('iPhone')) {
    return 'iPhone';
  }
  if (userAgent.match('iPad')) {
    return 'iPad';
  }
  return 'unknown';
};
  
window.NPL = {
  activate: function(filename, msg){
    if (GetSystem() === 'iPhone' || GetSystem() === 'macos') {
      const params = { filename, msg };
      params.msg = JSON.stringify(msg);

      if (window.webkit &&
        window.webkit.messageHandlers &&
        window.webkit.messageHandlers.activate &&
        window.webkit.messageHandlers.activate.postMessage) {
        window.webkit.messageHandlers.activate.postMessage(params);
      }
    } else if (GetSystem() === 'windows') {

      if (window.chrome &&
        window.chrome.webview &&
        window.chrome.webview.postMessage) {
        const params = { filename, msg };
        window.chrome.webview.postMessage(JSON.stringify(params))
        return
      }
      const params = { filename, msg };
      window.location.href = 'paracraft://sendMsg?' + JSON.stringify(params);
    }
  },
  this: function(callback, params){
    if (params && params.filename) {
      allMsg[params.filename] = callback;
    }
  },
  receive: function(filename, msg){
    if (allMsg[filename]) {
      var msgJson = JSON.parse(msg) || {};
      allMsg[filename](msgJson);
    }
  },
};