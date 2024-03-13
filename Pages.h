
static const char rebooting_html[] PROGMEM = R"==(
<!DOCTYPE html><html>
<head>
<meta http-equiv="refresh" content="8; url=/info.html">
<style type="text/css">#loader {z-index: 1;width: 50px;height: 50px;margin: 0 0 0 0;border: 6px solid #f3f3f3;border-radius: 50%;border-top: 6px solid #3498db;width: 50px;height: 50px;-webkit-animation: spin 2s linear infinite;animation: spin 2s linear infinite; } @-webkit-keyframes spin {0%{-webkit-transform: rotate(0deg);}100%{-webkit-transform: rotate(360deg);}}@keyframes spin{0%{ transform: rotate(0deg);}100%{transform: rotate(360deg);}}
body {background-color: #303030; color: #ffffff; font-size: 20px; font-weight: bold; margin: 0 0 0 0.0; padding: 0.4em 0.4em 0.4em 0.6em;}  
#msgfmt { font-size: 16px; font-weight: normal;}
#status {font-size: 16px; font-weight: normal;}
</style>
</head>
<center>
<br><br><br><br><br>
<p id="status"><div id='loader'>
</div><br>Rebooting</p>
</center>
</html>
)==";









static const char upload_html[] PROGMEM = R"==(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>File Upload</title>
<link rel="stylesheet" href="style.css">
<style>body{overflow-y:auto;}
table{height:300px; width:600px;overflow-y:scroll; overflow-x:hidden; display:block;}
.tdf {margin-left:25%;margin-right:25%;width:500px;}
.tds {margin-left:25%;margin-right:25%;width:200px;}
.tdc {margin-left:25%;margin-right:25%;width:120px;}
</style>
<script>
function formatBytes(bytes) {  if(bytes == 0) return '0 Bytes';  var k = 1024,  dm = 2,  sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'],  i = Math.floor(Math.log(bytes) / Math.log(k));  return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];}
function statusUpl() {  document.getElementById("upload").style.display="none";  document.getElementById("btnsel").style.display="none";  document.getElementById("status").innerHTML = "<div id='loader'></div><br>Uploading files";}
function FileSelected(e)
{  
var totalsize = 0;
var strdisp = "<table style='border:1px solid black;margin-left:auto;margin-right:auto;'>";  
var file = document.getElementById("upfiles").files;  
for (var i = 0; i < file.length; i++)  
{   
totalsize = totalsize + file[i].size;
strdisp = strdisp + "<tr><td class'tdc'>" + ((i) + 1) + "</td><td class='tdf'>" + file[i].name + "</td><td class='tds'>" + formatBytes(file[i].size) + "</td></tr>";  
}  
document.getElementById("selfile").innerHTML = strdisp + "</table><br> Total Upload Size: " + formatBytes(totalsize);  
document.getElementById("upload").style.display="block";
}
</script>
</head>
<body>
<center>
<form action="/upload.html" enctype="multipart/form-data" method="post">
<p>File Uploader<br><br></p><p>
<input id="btnsel" type="button" onclick="document.getElementById('upfiles').click()" value="Select files" style="display: block;"><p id="selfile"></p>
<input id="upfiles" type="file" name="fwupdate" size="0" onChange="FileSelected();" style="width:0; height:0;" multiple></p><div><p id="status"></p>
<input id="upload" type="submit" value="Upload Files" onClick="statusUpl();" style="display: none;"></div>
</form><center>
</body></html>
)==";










#if USECONFIG



static const char index_html[] PROGMEM = R"==(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Admin Panel</title>
<link rel="stylesheet" href="style.css">
</head>
<body><div class="sidenav">
<a href="/fileman.html" target="mframe">File Manager</a>
<a href="/upload.html" target="mframe">File Uploader</a>
<a href="/info.html" target="mframe">ESP Information</a>
<a href="/config.html" target="mframe">Config Editor</a>
<a href="/reboot.html" target="mframe">Reboot ESP</a>
</div>
<div class="main">
<iframe src="/fileman.html" name="mframe" height="100%" width="100%" frameborder="0"></iframe>
</div>
</table>
</body></html> 
)==";



#else



static const char index_html[] PROGMEM = R"==(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Admin Panel</title>
<link rel="stylesheet" href="style.css">
</head>
<body><div class="sidenav">
<a href="/fileman.html" target="mframe">File Manager</a>
<a href="/upload.html" target="mframe">File Uploader</a>
<a href="/info.html" target="mframe">ESP Information</a>
<a href="/reboot.html" target="mframe">Reboot ESP</a>
</div>
<div class="main">
<iframe src="/fileman.html" name="mframe" height="100%" width="100%" frameborder="0"></iframe>
</div>
</table>
</body></html> 
)==";



#endif







static const char reboot_html[] PROGMEM = R"==(
<!DOCTYPE html><html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP Reboot</title>
<link rel="stylesheet" href="style.css">
<script>
function statusRbt() { var answer = confirm("Are you sure you want to reboot?");  if (answer) {document.getElementById("reboot").style.display="none";   document.getElementById("status").innerHTML = "<div id='loader'></div><br>Rebooting ESP Board"; return true;  }else {   return false;  }}
</script>
</head>
<body>
<center>
<form action="/reboot.html" method="post">
<p>ESP Reboot<br><br></p>
<p id="msgrbt">This will reboot the esp board</p><div>
<p id="status"></p>
<input id="reboot" type="submit" value="Reboot ESP" onClick="return statusRbt();" style="display: block;"></div>
</form><center>
</body></html>
)==";








static const char style_css[] PROGMEM = R"==(
body {
background-color: #303030;
color: #ffffff;
font-size: 14px;
font-weight: bold;
margin: 0 0 0 0.0;
overflow-y:hidden;
padding: 0.4em 0.4em 0.4em 0.6em;
} 

.slct  {
transition-duration: 0.4s;
box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2), 0 6px 20px 0 rgba(0,0,0,0.19);
text-align: center;
-webkit-appearance: none;
background-color: DodgerBlue;
border: none;
color: white;
padding: 9px 1px;
font-size: 16px;
cursor: pointer;
font-weight: bold;
}


.main {
margin-left: 150px;     
padding: 10px 10px; 
position: absolute; 
top: 0; 
right: 0;
bottom: 0; 
left: 0;
overflow-y:hidden;
}


a:link {
color: #ffffff; 
text-decoration: none;
} 

a:visited {
color: #ffffff; 
text-decoration: none;
} 

a:hover {
color: #0d4fb5; 
text-decoration: underline;
} 

a:active {
color: #ffffff; 
text-decoration: underline;
} 

table {
font-family: arial, sans-serif; 
border-collapse: collapse; 
width: 100%;
} 

td, th {
border: 1px solid #dddddd; text-align: left; padding: 8px;
} 


input[type="submit"]:hover { 
background: #ffffff;
color: #1d1d1d; 
}

input[type="submit"]:active { 
outline-color: #1d1d1d;
color: #1d1d1d;
background: #ffffff; 
}

#selfile {  
font-size: 16px;  
font-weight: normal;
}

input[type="button"]:hover {
background: #ffffff;
color: #1d1d1d;
}

input[type="button"]:active {
outline-color: #1d1d1d;
color: #1d1d1d;
background: #ffffff;
}


.sidenav {
width: 140px;
position: fixed;
z-index: 1;
top: 20px;
left: 10px;
background: #494949;
overflow-x: hidden;
padding: 8px 0;
}

.sidenav a {
padding: 6px 8px 6px 16px;
text-decoration: none;
font-size: 14px;
color: #ffffff;
display: block;
}

.sidenav a:hover {
color: #0d4fb5;
}

msg {
color: #ffffff; 
font-weight: 
normal; 
text-shadow: none;
}

#loader {  
z-index: 1;  
width: 50px;  
height: 50px;  
margin: 0 0 0 0;  
border: 6px solid #f3f3f3;  
border-radius: 50%;  
border-top: 6px solid #626262;  
width: 50px;  
height: 50px;  
-webkit-animation: spin 2s linear infinite;  
animation: spin 2s linear infinite;
}
  
@-webkit-keyframes spin {
0%{ -webkit-transform: rotate(0deg); }
100% { -webkit-transform: rotate(360deg); }
}

@keyframes spin {
0% { transform: rotate(0deg); }
100% {transform: rotate(360deg); }
}

#status {
font-size: 16px;
font-weight: normal;
}

#msgfmt {
font-size: 16px;  
font-weight: normal;
}

)==";
