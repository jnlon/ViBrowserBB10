function inViewport(el) {
  
  if (!el.getBoundingClientRect) 
    return false;
  
  var rect = el.getBoundingClientRect();
  
  if (rect.bottom == 0 && rect.top == 0)
    return false;
  
  return (rect.top >= 0 &&
          rect.left >= 0 &&
          rect.bottom <= (window.innerHeight || html.clientHeight) &&
          rect.right <= (window.innerWidth || html.clientWidth));
}


links = document.getElementsByTagName('a');
hints = [];
elemIndex = [];
elemBGColors = [];
elemColors = [];
filter = "";
numfilter = "";

function init_once() {
  
  var hintStyleTag = document.createElement("style");
  
  hintStyleTag.innerHTML = " \
  x-vibrowserhint \{\
      font-size: 100% !important; \
      position: absolute !important; \
      float: left !important; \
      font-weight: bold !important; \
      color: white !important; \
      background-color: red !important; \
      font-style: none !important; \
      text-decoration: none !important; \
      margin-left: -3px !important; \
      margin-top: -3px !important; \
   \} ";
  
  document.head.appendChild(hintStyleTag);
  
  
  for (var i=0;i<links.length;i++) {
     //insert empty element on ALL links
     hint = document.createElement("x-vibrowserhint");
     links[i].parentElement.insertBefore(hint,links[i]);
     hint.style.zIndex = hint.parentElement.style.zIndex;    
     hints[i] = hint;
    
    //save css values for ALL links
    elemBGColors[i] = links[i].style.backgroundColor; 
    elemColors[i] = links[i].style.color;
  }
}

function init() {
  elemIndex = [];
  
  initNumFilter();
  
  for (var i=0; i<links.length; i++) {
    var link = links[i];
    if (inViewport(link) && filterHighlight(link.text)) {
      elemIndex.push(i);
    }   
  }
}

function highlight() {
  
  var toPost = "";
  var first = elemIndex[0];
    
  for (var li=0;li<elemIndex.length;li++) {
	  
    if ( numfilter && (li+1).toString().indexOf(numfilter) != 0) {
      first = i;
      continue;
    }
    
    var i = elemIndex[li];
    
    toPost += encodeURI(links[i].href) + " " + (li+1) + "\n";
    hints[i].innerHTML = (li+1);
    links[i].style.backgroundColor = "yellow";
    links[i].style.color = "black";
  }
    
  links[first].style.backgroundColor = "lime";
  
  //if (navigator.cascades)
    navigator.cascades.postMessage(toPost);
  //else
	//  console.log(toPost);
}

function filterHighlight(linktext) {
  if (filter == "")
    return true;
    if (linktext.toLowerCase().indexOf(filter) == -1)
      return false;
    else 
      return true;
}

function exitHintMode() {
    for (var i=0;i<links.length;i++) {
      links[i].style.color = elemColors[i];
      links[i].style.backgroundColor = elemBGColors[i]; 
      hints[i].innerHTML = "";
    }
}


init_once();

function reset() {
	//var t1 = new Date();
	init();
	highlight();
	//var t2 = new Date();
	//document.write("time: " + (t2-t1));
}

function initNumFilter() { 
  numfilter = "";
  for (var cpos=filter.length-1; cpos >= 0; cpos--) {
    if (!isNaN(filter.charAt(cpos))) // is a number
      numfilter = filter.charAt(cpos).concat(numfilter);
  }
  
  if (!numfilter)
	  return;
  
  filter = filter.substring(0, filter.length-numfilter.length);
}

/*function test1() {
  filter = "18";
  exitHintMode();
  init();
  highlight();
}

setTimeout(test1, 10);
setTimeout(exitHintMode, 3000);*/

navigator.cascades.onmessage = function onmessage(message) {
   filter = message;
   exitHintMode();
   init();
   highlight();
}