show_page("start");
window.onhashchange = function () {
    if (window.location.hash === "#start") {
        show_page("start");
    }
    else if (window.location.hash === "#instructions") {
        show_page("instructions");
    }
    else if (window.location.hash === "#select-template") {
        show_page("select-template");
    }
    else if (window.location.hash === "#upload-image") {
        show_page("upload-image");
    }
    else if (window.location.hash === "#write-captions") {
        show_page("write-captions");
    }
}
function show_page(new_page) {
    let show = document.getElementById(new_page);
    window.location.hash = new_page;
    let divs = document.getElementsByClassName("pages");
    for (let div of divs) {
        div.style.display = "none"
    }
    show.style.display = "block"

};
document.getElementById("upload").onchange = () => {
    let file = document.getElementById("upload").value;
    var filename = file.replace(/^.*\\/, "");
    document.getElementById("file-display").innerHTML = "Uploaded: " + filename;
}
