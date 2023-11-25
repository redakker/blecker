// Must be a flat json object		
let boardData = {};
let ajax = {};
let timer;
let boardname;
let resetconfig = false;
let advancedNetwork = false;
let rebootCheck;

let BOARD_NAME = "%BOARD_NAME%";

// AJAX service
ajax.x = function () {
    if (typeof XMLHttpRequest !== 'undefined') {
        return new XMLHttpRequest();
    }
    var versions = [
        "MSXML2.XmlHttp.6.0",
        "MSXML2.XmlHttp.5.0",
        "MSXML2.XmlHttp.4.0",
        "MSXML2.XmlHttp.3.0",
        "MSXML2.XmlHttp.2.0",
        "Microsoft.XmlHttp"
    ];

    var xhr;
    for (var i = 0; i < versions.length; i++) {
        try {
            xhr = new ActiveXObject(versions[i]);
            break;
        } catch (e) {
        }
    }
    return xhr;
};

ajax.send = function (url, callback, method, data, async) {
    if (async === undefined) {
        async = true;
    }
    var x = ajax.x();
    x.open(method, url, async);
    x.onreadystatechange = function () {
        if (x.readyState == 4) {
            callback(x.responseText, x.status)
        }
    };
    if (method == 'POST') {
        x.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    }
    x.send(data)
};

ajax.get = function (url, data, callback, async) {
    var query = [];
    for (var key in data) {
        query.push(key + '=' + data[key]);
    }
    ajax.send(url + (query.length ? '?' + query.join('&') : ''), callback, 'GET', null, async)
};

ajax.post = function (url, data, callback, async) {
    var query = [];
    for (var key in data) {
        query.push(key + '=' + data[key]);
    }
    ajax.send(url, callback, 'POST', query.join('&'), async)
};

// Ajax upload
// Function that will allow us to know if Ajax uploads are supported
function supportAjaxUploadWithProgress() {
    return supportFileAPI() && supportAjaxUploadProgressEvents() && supportFormData();
  
    // Is the File API supported?
    function supportFileAPI() {
      var fi = document.createElement('INPUT');
      fi.type = 'file';
      return 'files' in fi;
    };
  
    // Are progress events supported?
    function supportAjaxUploadProgressEvents() {
      var xhr = new XMLHttpRequest();
      return !! (xhr && ('upload' in xhr) && ('onprogress' in xhr.upload));
    };
  
    // Is FormData supported?
    function supportFormData() {
      return !! window.FormData;
    }
  }
  
  function initFullFormAjaxUpload() {
    var form = document.getElementById('form-id');
    form.onsubmit = function() {
      // FormData receives the whole form
      var formData = new FormData(form);
  
      // We send the data where the form wanted
      var action = form.getAttribute('action');
  
      // Code common to both variants
      sendXHRequest(formData, action);
  
      // Avoid normal form submission
      return false;
    }
  }
  
  function initFileOnlyAjaxUpload() {
    var uploadBtn = document.getElementById('upload-button-id');
    uploadBtn.onclick = function (evt) {
      var formData = new FormData();
  
      // Since this is the file only, we send it to a specific location
      var action = '/upgrade';
  
      // FormData only has the file
      var fileInput = document.getElementById('file-id');
      var file = fileInput.files[0];
      formData.append('our-file', file);

      getItem("custom-file-upload").style.display = "none";
      uploadBtn.style.display = "none";
  
      // Code common to both variants
      sendXHRequest(formData, action);
    }
  }
  
  // Once the FormData instance is ready and we know
  // where to send the data, the code is the same
  // for both variants of this technique
  function sendXHRequest(formData, uri) {
    // Get an XMLHttpRequest instance
    var xhr = new XMLHttpRequest();
  
    // Set up events
    xhr.upload.addEventListener('loadstart', onloadstartHandler, false);
    xhr.upload.addEventListener('progress', onprogressHandler, false);
    xhr.upload.addEventListener('load', onloadHandler, false);
    xhr.addEventListener('readystatechange', onreadystatechangeHandler, false);
  
    // Set up request
    xhr.open('POST', uri, true);
  
    // Fire!
    xhr.send(formData);
  }
  
  // Handle the start of the transmission
  function onloadstartHandler(evt) {
    var div = document.getElementById('upload-status');
    div.innerHTML = 'Upload started.';

    var loader = getItem("loader");
    loader.style.display = "block";

    rebootCheck = setInterval(ping, 60000);

  }
  
  // Handle the end of the transmission
  function onloadHandler(evt) {
    var div = document.getElementById('upload-status');
    // div.innerHTML += '<' + 'br>File uploaded. Waiting for response.';
  }
  
  // Handle the progress
  function onprogressHandler(evt) {
    var div = document.getElementById('progress');
    var percent = evt.loaded/evt.total*100;
    // div.innerHTML = 'Progress: ' + percent + '%';
  }
  
  // Handle the response from the server
  function onreadystatechangeHandler(evt) {
    var status, text, readyState;
  
    try {
      readyState = evt.target.readyState;
      text = evt.target.responseText;
      status = evt.target.status;
    }
    catch(e) {
      return;
    }
  
    if (readyState == 4 && status == '200' && evt.target.responseText) {
      var status = document.getElementById('upload-status');
      status.innerHTML += '<' + 'br>Success!';

      // var result = document.getElementById('result');
      // result.innerHTML = '<p>The server saw it as:</p><pre>' + evt.target.responseText + '</pre>';
    }
  }

// Clock functions
function clearconfig() {
    if (getItem("reset").checked) {
        resetconfig = true;
        getItem("resetbutton").style.display = 'block';
        getItem("savebutton").style.display = 'none';
    } else {
        resetconfig = false;
        getItem("resetbutton").style.display = 'none';
        getItem("savebutton").style.display = 'block';
    }
}

function advanced() {
    if (!advancedNetwork) {
        getItem("networkmore_presence").style.display = 'block';
        getItem("networkmore_status").style.display = 'block';
        getItem("networkmore_deviceid").style.display = 'block';
        getItem("networkmore_retain").style.display = 'block';
        getItem("networkmore_presence_retain").style.display = 'block';
        

        getItem("advancednet").style.display = 'none';
        getItem("basicnet").style.display = 'block';

        advancedNetwork = true;

    } else {
        getItem("networkmore_presence").style.display = 'none';
        getItem("networkmore_status").style.display = 'none';
        getItem("networkmore_deviceid").style.display = 'none';
        getItem("networkmore_retain").style.display = 'none';
        getItem("networkmore_presence_retain").style.display = 'none';
        
        getItem("advancednet").style.display = 'block';
        getItem("basicnet").style.display = 'none';

        advancedNetwork = false;
    }
}

function save(){
    if (resetconfig) {
        boardData = {
            name: boardname
        }
    
    } else {
    
        var button = getItem("savebutton");
        var loader = getItem("loader");
        
        loader.style.display = "block";
        button.style.display = "none";						
    
        clockdata = collectData();
        
        
        clockdata.name = boardname;
    }
    
    var json = JSON.stringify(clockdata);
    ajax.post("/savedata", {data : json}, function(){});
    
    rebootCheck = setInterval(ping, 20000);
    
}

function ping(){
    ajax.get("/", {}, function(response, status){
        if(status == 200) {
            clearInterval(rebootCheck);
            var button = getItem("savebutton");
            
            var loader = getItem("loader");
            loader.style.display = "none";
            if (button) {
                button.style.display = "block";
            }

            // Reload the page
            window.location.href = "/";            
            
        }
    });
}

function validateInteger(input){
    var value = input.value; 

    if (value == parseInt(value) && !value.includes(" ") && !value.includes(".")) {
        return;
    } else {
        input.value = "";
        return;
    }

}

function collectData (){
    var form = document.getElementById('dataform');
    const values = {};
    const inputs = form.elements;
    for (let i = 0; i < inputs.length; i++) {
        if (inputs[i].name) {
            values[inputs[i].name] = encodeURIComponent(inputs[i].value);
        }
    }
    return values;
}


function fillData() {
    Object.entries(boardData).forEach(([key, value]) => {
        if (getItem(key)) {
            getItem(key).value = value;
        }
    });
}


function pad(n) { return ("0" + n).slice(-2); }

function getItem(id) {
    return document.getElementById(id);
}

function getData() {

    ajax.get('/data', {}, function(response) {
            //var response = '{"ssid":"","pw":"","timestamp":"1611008990","ntp":"","timezone":"","dst":false,"name":"24H"}';
            if (response) {
                boardData = JSON.parse(response);
                boardname = boardData.name;
                fillData();
            } else {
                console.log("Response was empty.");
            }
    });
}

// Add css slowly to mitigate the network traffic for ESP32
function loadCSS(cssName){
    document.getElementsByTagName("head")[0].insertAdjacentHTML(
        "beforeend",
        "<link rel=\"stylesheet\" href=\"" + cssName + "\" />");
}

function getChipInfo() {
    ajax.get('/chipinfo', {}, function(response) {        
        if (response) {
            var chipInfo = JSON.parse(response);            
            document.getElementById("chip").innerHTML = chipInfo.model + " - V" + chipInfo.revision + "<br /> Cores: " + chipInfo.cores;
        } else {
            console.log("Response was empty.");
        }
    });
}

// Modal

var modal = document.getElementById("myModal");

var btn = document.getElementById("reset");

var span = document.getElementsByClassName("close")[0];

if (modal && btn && span) {
    btn.onclick = function() {
    modal.style.display = "block";
    }

    span.onclick = function() {
    modal.style.display = "none";
    }

    window.onclick = function(event) {
    if (event.target == modal) {
        modal.style.display = "none";
    }
    }
}

setTimeout(function() {
    if (location.pathname == "/" || location.pathname.includes("features")) {
        getData();
    }
}, 300);

setTimeout(function() {
    loadCSS('/normalize.css');
}, 600);

setTimeout(function() {
    loadCSS('/skeleton.css');
}, 900);

setTimeout(function() {
    loadCSS('/style.css');
}, 1200);
setTimeout(function() {
    getChipInfo();
}, 1500);
 // Actually confirm support
 // initialize the ajax upload in the update page
 if (window.location.href.includes("update") && supportAjaxUploadWithProgress()) {
    // Ajax uploads are supported!
    // Change the support message and enable the upload button
    var notice = document.getElementById('support-notice');
    
  
    // Init the Ajax form submission
    initFullFormAjaxUpload();
  
    // Init the single-field file upload
    initFileOnlyAjaxUpload();
  }

  function getFile() {
    getItem("file-id").click();
  }

  function getFileName(obj) {
    var file = obj.value;
    var fileName = file.split("\\");
    fileName = fileName[fileName.length - 1];
    document.getElementById("fileinput").innerHTML = fileName;
    var extension = fileName.substr(fileName.length - 4);

    var uploadBtn = document.getElementById('upload-button-id');
    var error = document.getElementById('error');
    // Small verification
    // file name should contains the BOARD_NAME string and the extension should be ".bin"
    if (fileName.includes(BOARD_NAME + "_") && extension == ".bin") {
        uploadBtn.removeAttribute('disabled');
        error.style.display = "none";
    } else {
        error.style.display = "block";
    }
  }