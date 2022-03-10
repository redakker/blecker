function setManifest() {
    var sel = document.getElementById('ver');
    var opt = sel.options[sel.selectedIndex];
    var m = opt.dataset.manifest;
    var me = opt.dataset.ethernet;
    //document.getElementById('eth').style.display = me ? "block":"none";
    //if (me && document.getElementById('ethernet').checked) {
    //  m = me;
    //}
    document.getElementById('install').setAttribute('manifest',m);
    //document.getElementById('verstr').textContent = opt.text;
}