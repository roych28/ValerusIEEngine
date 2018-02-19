
function save_options() {
    var urlInput = document.getElementById('url_input').value;
    chrome.storage.sync.set({
        url: urlInput
    }, function() {
        console.log("chrome storage set success");
    });
}

function restore_options() {
    // Use default value color = 'red' and likesColor = true.
    chrome.storage.sync.get({
        url: 'http://47.21.44.216/'
    }, function(items) {
        document.getElementById('url_input').value = items.url;
    });
}
document.addEventListener('DOMContentLoaded', restore_options);
document.getElementById('save').addEventListener('click', save_options);