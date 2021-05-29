show_page("start");
window.onhashchange = function(){
    if(window.location.hash === "#start")
    {
        show_page("start");
    }
    else if(window.location.hash === "#instructions")
    {
        show_page("instructions");
    }
    else if(window.location.hash === "#select-template")
    {
        show_page("select-template");
    }
    else if(window.location.hash === "#upload-image")
    {
        show_page("upload-image");
    }
    else if(window.location.hash === "#write-captions")
    {
        show_page("write-captions");
    }
}
function show_page(new_page) {
    let show = document.getElementById(new_page);
    let children = show.children;
    window.location.hash= new_page;
    let divs = document.getElementsByClassName("pages");
    for(let div of divs)
    {
        // div.style.visibility = "hidden";
        // div.style.zindex = -1;
        div.style.display = "none"
    }
    // show.style.visibility = "visible";
    show.style.display = "block"

};