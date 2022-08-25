function clear_All_Information() {
    document.getElementById('pass').innerHTML = '';
    document.getElementById('pass_repeat').innerHTML = '';
}

function reset_Button() {
    clear_All_Information();
    document.getElementById("inputs").reset();
}

document.getElementById("reset_btn").onclick = function () {
    reset_Button();
}