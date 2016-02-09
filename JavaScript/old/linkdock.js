// LinkDock - V 1.4
// By Brian Gosselin of http://scriptasylum.com
// Release Info:
// V 1.0 - Initial release.
// V 1.1 - Minor code changes for magnification smoothness and accuracy.
// V 1.2 - Added option to include text under the links as they are hovered over.
//         Fixed a magnification bug when the page is too small to fit the whole menu.
// V 1.3 - Fixed a bug where you get script errors if you hover over an image before the
//         page finishes loading.
// V 1.4 - Added a tweak by RAJ to smoothen the entry into the menu.

// ENTER LINK ATTRIBUTES IN THE ARRAY BELOW; EACH LINE CONTAINS ALL THE
// PARAMETERS FOR ONE LINK. USE THE FOLLOWING FORMAT:
//   [ 'LINK_URL' , 'IMAGE_URL' , 'URL_TARGET', 'TEXT_UNDER_LINK' ]
// VALID VALUES FOR 'URL_TARGET' ARE:
//   '_blank'   (NEW WINDOW)
//   'name'     (THE NAME OF AN EXISTING WINDOW OR FRAME)
//   ''         (CURRENT PAGE)
// IF YOU DO NOT WANT TEXT DISPLAYED UNDER A LINK, SIMPLY USE AN EMPTY STRING AS THE PARAMETER FOR 'TEXT_UNDER_LINK'.

var linkList=[
[ 'index.html' , 'images/home.gif' , 'content', 'Home Page' ],
[ 'people.html' , 'images/people.gif' , 'content', 'Friends & Family' ],
[ 'hobbies.html' , 'images/hobbies.gif' , 'content', 'My Interests' ],
[ 'win98.html' , 'images/windows98.gif' , 'content', 'Windows 98 Hints & Tricks' ],
[ 'winxp.html' , 'images/windowsxp.gif' , 'content', 'Windows XP Hints & Tricks' ],
[ 'funny.html' , 'images/thoughts.gif' , 'content', 'Random Thoughts' ],
[ '../index.html' , 'images/sa.gif' , '_top', 'My Javascript/DHTML site...' ]
//[ 'mailto:scriptasylum@hotmail.com' , 'images/email.gif' , 'content', 'Contact Me...' ]
]

// CHANGE THE OTHER VALUES BELOW TO SUIT YOUR APPLICATION

var startSize=50;  // THE STARTING WIDTH *AND* HEIGHT OF EACH IMAGE (THE IMAGES WILL BE SCALED).
//<!-- RAJ
var curSize=50;    // variable used in the first time zoom/shrink
// RAJ -->
var endSize=100;   // THE ENDING WIDTH *AND* HEIGHT OF EACH IMAGE (THE IMAGES WILL BE SCALED).
var useText=true;  // true = USE TEXT UNDER THE LINK, false = NO TEXT UNDER THE LINK.
var defText='Hover over a link...'  // DEFAULT TEXT TO APPEAR UNDER THE LINKS WHEN NOT HOVERED OVER.
                                    // USE AN EMPTY STRING FOR NO TEXT.
var textGap=10;    // PIXEL GAP FROM BOTTOM OF MENU TO TOP OF OPTIONAL TEXT (WHEN defText IS SET TO true).
var effectW=3.5;   // THE NUMBER OF ICONS AFFECTED BY OF THE MAGNIFICATION AT ONCE (APPROXIMATE). USE VALUES BETWEEN 2 AND 5.

// BELOW IS THE STYLE-SHEET RULE FOR HOW THE TEXT IS TO BE DISPLAYED. USE VALID CSS RULES.

var textStyle="font-family:verdana; font-size:11pt; color:white; font-weight:bold";


//********** DO NOT EDIT BEYOND THIS POINT **********\\


var w3c=(document.getElementById)?true:false;
var ie4=(document.all && !w3c)?true:false;
var ie5=(document.all && w3c)?true:false;
var ns4=(document.layers)?true:false;
var mx=0;
var overEl=false;
var enterEl=false;
var id=0;
var elList=new Array();
var elText;
var pgLoaded=false;
if(defText=='')defText='&nbsp;';
effectW=Math.max(2,Math.min(5,effectW))+.5;
var wA=effectW*endSize/2;
var mX=wA/1.5;

function getMxy(v){
mx=(ie5||ie4)?event.clientX:v.pageX;
}

function getEl(s){
if(ns4)return findLayer(s,document);
else return (ie4)?document.all[s]:document.getElementById(s);
}

function getW(e){
return parseInt(e.style.width);
}

function setImgS(i,x){
elList[i].style.width=x;
elList[i].style.height=x;
document.images['linkDockI'+i].width=x;
document.images['linkDockI'+i].height=x;
}

function getL(el){
var x=0;
var sx=(document.all)?document.body.scrollLeft:0;
while(el.offsetParent!=null){
x+=el.offsetLeft;
el=el.offsetParent;
}
return x+el.offsetLeft-sx;
}

function rAll(){
// decrease size of zoomed images gradually
for(i=0;i<elList.length;i++) {
//<!-- RAJ
curSize=getW(elList[i]);
if (curSize>startSize) {
id=setTimeout('rAll()',10);
curSize--;
// RAJ -->
setImgS(i,curSize);
}}}

function dockMagnify(){
var tEl,n1,n2;
//<!-- RAJ
if(overEl) {if(curSize<endSize) curSize+=5; } else curSize=50;
// RAJ -->
if(overEl){
for(i=0;i<linkList.length;i++){
tEl=elList[i];
if((getL(tEl)>=mx-wA)&&(getL(tEl)<=mx+wA)){
n1=getL(tEl)+getW(tEl)/2+10;
n2=mx-wA;
//<!-- RAJ
n1=(curSize*Math.sin(Math.abs(n1-n2)/mX));
// RAJ -->
setImgS(i,Math.max(n1,startSize));
}else setImgS(i,startSize);
}}}

function mOver(){
overEl=true;
clearTimeout(id);
}

function mOut(){
overEl=false;
id=setTimeout('rAll()',100);
}

// FUNCTION TO FIND NESTED LAYERS IN NS4 BY MIKE HALL
function findLayer(name,doc){
var i,layer;
for(i=0;i<doc.layers.length;i++){
layer=doc.layers[i];
if(layer.name==name)return layer;
if(layer.document.layers.length>0)if((layer=findLayer(name,layer.document))!=null)return layer;
}
return null;
}

function writeText(text){
if(useText && pgLoaded){
text=(text<0)?defText:linkList[text][3];
if(text=='')text='&nbsp;';
if(ns4){
elText.document.open();
elText.document.write('<center><span style="'+textStyle+'">'+text+'</span></center>');
elText.document.close();
}
else elText.innerHTML=text;
}}

function writeHTML(){
var t='';
if(w3c||ie4){
t+='<table cellpadding=0 cellspacing=0 border=0 height="'+endSize+'" width="'+(((linkList.length-4)*startSize)+(4*endSize))+'px"><tr valign="bottom" align="center"><td>';
for(i=0;i<linkList.length;i++){
t+='<span id="linkDockD'+i+'" style="width:'+startSize+'px; height:'+startSize+'px;">';
t+='<a href="'+linkList[i][0]+'" target="'+linkList[i][2]+'" onmouseover="writeText('+i+')" onmouseout="writeText(-1)"><img name="linkDockI'+i+'" src="'+linkList[i][1]+'" width="'+startSize+'" height="'+startSize+'" border="none"></a>';
t+='</span>';
}
t+='</td></tr></table>';
if(useText)t+='<center><div id="dockText" style="'+textStyle+'; padding-top:'+textGap+'px">'+defText+'</div></center>';
}else{
t+='<table cellpadding=0 cellspacing=0 border=0 width="'+(linkList.length*startSize)+'"><tr valign="bottom">';
for(i=0;i<linkList.length;i++)t+='<td height="'+endSize+'"><a href="'+linkList[i][0]+'" target="'+linkList[i][2]+'" onmouseover="writeText('+i+')" onmouseout="writeText(-1)"><img src="'+linkList[i][1]+'" width="'+startSize+'" height="'+startSize+'" border="none"></a></td>';
t+='</tr>';
if(useText)t+='<tr><td colspan="'+linkList.length+'"><td height="'+textGap+'"></td></tr><tr><td colspan="'+linkList.length+'"><ilayer width="'+(linkList.length*startSize)+'"><layer name="dockText" height="100%"></layer></ilayer></td></tr>';
t+='</table>';
}
document.write(t);
}

window.onload=function(){
if(w3c||ie4){
for(j=0;j<linkList.length;j++){
elList[j]=getEl('linkDockD'+j);
elList[j].n=j;
elList[j].onmouseover=mOver;
elList[j].onmouseout=mOut;
}
document.onmousemove=getMxy;
setInterval('dockMagnify()',20);
}
elText=getEl('dockText');
if(ns4)writeText(-1);
pgLoaded=true;
}

writeHTML();