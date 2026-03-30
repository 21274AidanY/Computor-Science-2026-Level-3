//Event listenr for when getValues is called
window.addEventListener("load", getValues);
//
function getValues(){
    var webRequest = XMLHttpRequest();
    webRequest.onreadystatechange = function(){
        if(this.readyState == 4 && this.status == 200){
            var FormValues = JSON.parse(this.responseText);
            console.log(myObj);
            document.getElementById("launchVal").innerHTML = FormValues.textVal;
            document.getElementById("targetVal").innerHTML = FormValues.textVal;
            document.getElementById("FTSVal").innerHTML = FormValues.textVal;
        }
    };
    //Set up the web request with method, url and async conditions
    webRequest.open("GET", "/values", true);
    //Send request
    webRequest.send();
}