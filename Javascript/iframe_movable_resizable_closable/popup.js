/********************************************************************

Popup Windows - V 4.5
Author: Brian Gosselin
Site URL: http://scriptasylum.com
Read the "releasenotes.txt" for supported features and release notes.

************** EDIT THE LINES BELOW AT YOUR OWN RISK ****************/

var w3c=(document.getElementById)? true: false;
var ns4=(document.layers)?true:false;
var ie5=(w3c && document.all)? true : false;
var ns6=(w3c && !document.all)? true: false;
var d=document;
currIDb=null; xoff=0; yoff=0;
currRS=null; rsxoff=0; rsyoff=0;
oldac=null; newac=null; zdx=1; mx=0; my=0;
var currFb=null; var currFs=null; var currFID=0; var currFcnt=0;
var cidlist=new Array();

//******* START OF EXPOSED FUNCTIONS. THESE CAN BE USED IN HYPERLINKS. *******\\

function fadeboxin(id){
if((currFb==null) && w3c){
clearInterval(currFID);
currFb=d.getElementById(id+'_b');
currFs=d.getElementById(id+'_s');
if(currFb.style.display=='none'){
currFcnt=0;
if(ie5)currFb.style.filter=currFs.style.filter="alpha(opacity=0)";
else currFb.style.MozOpacity=currFs.style.MozOpacity=0;
setAllScrollbars(true);
currFb.style.display=currFs.style.display='block';
changez(currFb);
currFID=setInterval('sub_fadein()',20);
}else currFb=null;
}}

function fadeboxout(id){
if((currFb==null) && w3c){
clearInterval(currFID);
currFb=d.getElementById(id+'_b');
currFs=d.getElementById(id+'_s');
if(currFb.style.display=='block'){
currFcnt=100;
if(ie5){
currFb.style.filter="alpha(opacity=100)";
currFs.style.filter="alpha(opacity=50)";
}else{
currFb.style.MozOpacity=1;
currFs.style.MozOpacity=.5;
}
setAllScrollbars(true);
currFb.style.display=currFs.style.display='block';
changez(currFb);
currFID=setInterval('sub_fadeout()',20);
}else currFb=null;
}}

function hidebox(id){
if(w3c){
//if(currFb!=d.getElementById(id+'_b')){
d.getElementById(id+'_b').style.display='none';
d.getElementById(id+'_s').style.display='none';
//}
}}

function showbox(id){
if(w3c){
var bx=d.getElementById(id+'_b');
var sh=d.getElementById(id+'_s');
bx.style.display='block';
sh.style.display='block';
sh.style.zIndex=++zdx;
bx.style.zIndex=++zdx;
if(ns6){
bx.style.MozOpacity=1;
sh.style.MozOpacity=.5;
}else{
bx.style.filter="alpha(opacity=100)";
sh.style.filter="alpha(opacity=50)";
}
changez(bx);
}}

function changecontent(id,text){
if(!document.getElementById(id+'_b').isExt){
var d=document.getElementById(id+'_c');
if(ns6)d.style.overflow="hidden";
d.innerHTML=text;
if(ns6)d.style.overflow="block";
}else document.getElementById(id+'_ifrm').src=text;
}

function movePopup(ids,x,y){
if(w3c){
var idb=document.getElementById(ids+'_b');
var ids=document.getElementById(ids+'_s');
idb.style.left=x+'px';
ids.style.left=x+8+'px';
idb.style.top=y+'px';
ids.style.top=y+8+'px';
}}

function resizePopup(ids,rx,ry){
if(w3c){
if(d.getElementById(ids+'_rs').rsEnable){
d.gEl=d.getElementById;
d.gEl(ids+"_extWA").style.display="block";
d.gEl(ids+"_rs").style.left=Math.max(rx,((ie5)?88:92))+'px';
d.gEl(ids+"_rs").style.top=Math.max(ry,((ie5)?68:72))+'px';
d.gEl(ids+"_b").style.width=Math.max(rx+((ie5)?12:8),100)+'px';
d.gEl(ids+"_b").style.height=Math.max(ry+((ie5)?12:8),80)+'px';
d.gEl(ids+"_t").style.width=Math.max(rx+((ie5)?4:3),((ns6)?95:92))+'px';
d.gEl(ids+"_btt").style.left=parseInt(d.gEl(ids+"_t").style.width)-48+'px';
d.gEl(ids+"_s").style.width=Math.max(rx+12,((ie5)?100:104))+'px';
d.gEl(ids+"_s").style.height=Math.max(ry+((ie5)?12:13),((ie5)?80:86))+'px';
d.gEl(ids+"_c").style.width=Math.max(rx-((ie5)?-5:5),((ie5)?92:87))+'px';
d.gEl(ids+"_c").style.height=Math.max(ry-((ie5)?24:28),44)+'px';
d.gEl(ids+"_max").h=parseInt(d.gEl(ids+"_b").style.height);
}}}

//******* END OF EXPOSED FUNCTIONS *******\\

function setAllScrollbars(ishide){
if(document.all){
var id;
for(i=0;i<cidlist.length;i++){
id=cidlist[i];
if(!document.getElementById(id+'_b').isExt)document.getElementById(id+'_c').style.overflow=(ishide)?"hidden":"auto";
}}}

function sub_fadein(){
currFcnt+=4;
if(ie5){
currFb.style.filter="alpha(opacity="+currFcnt+")";
currFs.style.filter="alpha(opacity="+(currFcnt/2)+")";
}else{
currFb.style.MozOpacity=currFcnt/100;
currFs.style.MozOpacity=(currFcnt/2)/100;
}
if(currFcnt>=99){
currFb.style.display=currFs.style.display='block';
setAllScrollbars(false)
currFb=null;
clearInterval(currFID);
}}

function sub_fadeout(){
currFcnt=currFcnt-4;
if(ie5){
currFb.style.filter="alpha(opacity="+currFcnt+")";
currFs.style.filter="alpha(opacity="+(currFcnt/2)+")";
}else{
currFb.style.MozOpacity=currFcnt/100;
currFs.style.MozOpacity=(currFcnt/2)/100;
}
if(currFcnt<=0){
currFb.style.display=currFs.style.display='none';
setAllScrollbars(false)
currFb=null;
clearInterval(currFID);
}}

function preloadBttns(){
var btns=new Array();
btns[0]=new Image(); btns[0].src="min.gif";
btns[1]=new Image(); btns[1].src="max.gif";
btns[2]=new Image(); btns[2].src="close.gif";
btns[3]=new Image(); btns[3].src="resize.gif";
}
preloadBttns();

function minimize(){
if(w3c){
d.getElementById(this.cid+"_b").style.height=(ie5)? '28px':'24px';
d.getElementById(this.cid+"_s").style.height='28px';
d.getElementById(this.cid+"_c").style.display='none';
d.getElementById(this.cid+"_rs").style.display='none';
ns6bugfix();
}}

function restore(){
if(w3c){
d.getElementById(this.cid+"_b").style.height=this.h+'px';
d.getElementById(this.cid+"_s").style.height=(ie5)? this.h+'px':this.h+5+'px';
d.getElementById(this.cid+"_c").style.display='block';
d.getElementById(this.cid+"_rs").style.display='block';
ns6bugfix();
}}

function ns6bugfix(){
if(navigator.userAgent.indexOf("Netscape/6")>0)setTimeout('self.resizeBy(0,1); self.resizeBy(0,-1);', 100);
}

function trackmouse(evt){
mx=(ie5)?event.clientX+d.body.scrollLeft:evt.pageX;
my=(ie5)?event.clientY+d.body.scrollTop:evt.pageY;
if(!ns6)movepopup();
if((currIDb!=null)||(currRS!=null))return false;
}

function movepopup(){
if((currIDb!=null)&&w3c)movePopup(currIDb.cid,mx+xoff,my+yoff);
if((currRS!=null)&&w3c)resizePopup(currRS.cid,mx+rsxoff,my+rsyoff);
return false;
}

function stopRS(){
d.getElementById(this.cid+"_extWA").style.display="none";
currRS=null;
}

function startRS(evt){
var ex=(ie5)?event.clientX+d.body.scrollLeft:evt.pageX;
var ey=(ie5)?event.clientY+d.body.scrollTop:evt.pageY;
rsxoff=parseInt(this.style.left)-ex;
rsyoff=parseInt(this.style.top)-ey;
currRS=this;
if(ns6)d.getElementById(this.cid+"_c").style.overflow='hidden';
return false;
}

function changez(v){
var th=(v!=null)?v:this;
if(oldac!=null)d.getElementById(oldac.cid+"_t").style.backgroundColor=oldac.inactivecolor;
if(ns6)d.getElementById(th.cid+"_c").style.overflow='auto';
oldac=th;
d.getElementById(th.cid+"_t").style.backgroundColor=th.activecolor;
d.getElementById(th.cid+"_s").style.zIndex=++zdx;
th.style.zIndex=++zdx;
d.getElementById(th.cid+"_rs").style.zIndex=++zdx;
}

function stopdrag(){
currIDb=null;
document.getElementById(this.cid+"_extWA").style.display="none";
ns6bugfix();
}

function grab_id(evt){
var ex=(ie5)?event.clientX+d.body.scrollLeft:evt.pageX;
var ey=(ie5)?event.clientY+d.body.scrollTop:evt.pageY;
xoff=parseInt(d.getElementById(this.cid+"_b").style.left)-ex;
yoff=parseInt(d.getElementById(this.cid+"_b").style.top)-ey;
currIDb=d.getElementById(this.cid+"_b");
currIDs=d.getElementById(this.cid+"_s");
d.getElementById(this.cid+"_extWA").style.display="block";
return false;
}

function subBox(x,y,w,h,bgc,id){
var v=d.createElement('div');
v.setAttribute('id',id);
v.style.position='absolute';
v.style.left=x+'px';
v.style.top=y+'px';
v.style.width=w+'px';
v.style.height=h+'px';
if(bgc!='')v.style.backgroundColor=bgc;
v.style.visibility='visible';
v.style.padding='0px';
return v;
}

function get_cookie(Name) {
var search=Name+"=";
var returnvalue="";
if(d.cookie.length>0){
offset=d.cookie.indexOf(search);
if(offset!=-1){
offset+=search.length;
end=d.cookie.indexOf(";",offset);
if(end==-1)end=d.cookie.length;
returnvalue=unescape(d.cookie.substring(offset,end));
}}
return returnvalue;
}

function popUp(x,y,w,h,cid,text,bgcolor,textcolor,fontstyleset,title,titlecolor,titletextcolor,bordercolor,scrollcolor,shadowcolor,showonstart,isdrag,isresize,oldOK,isExt,popOnce,minImg,maxImg,clsImg,rsImg){
var okPopUp=false;
if (popOnce){
if (get_cookie(cid)==""){
okPopUp=true;
d.cookie=cid+"=yes"
}}
else okPopUp=true;
if(okPopUp){
if(w3c){
cidlist[cidlist.length]=cid;
w=Math.max(w,100);
h=Math.max(h,80);
var rdiv=new subBox(w-((ie5)?12:8),h-((ie5)?12:8),7,7,'',cid+'_rs');
if(isresize){
rdiv.innerHTML='<img src="'+rsImg+'" width="7" height="7">';
rdiv.style.cursor='move';
}
rdiv.rsEnable=isresize;
var tw=(ie5)?w:w+4;
var th=(ie5)?h:h+6;
var shadow=new subBox(x+8,y+8,tw,th,shadowcolor,cid+'_s');
if(ie5)shadow.style.filter="alpha(opacity=50)";
else shadow.style.MozOpacity=.5;
shadow.style.zIndex=++zdx;
var outerdiv=new subBox(x,y,w,h,bordercolor,cid+'_b');
outerdiv.style.display="block";
outerdiv.style.borderStyle="outset";
outerdiv.style.borderWidth="2px";
outerdiv.style.borderColor=bordercolor;
outerdiv.style.zIndex=++zdx;
tw=(ie5)?w-8:w-5;
th=(ie5)?h+4:h-4;
var titlebar=new subBox(2,2,tw,20,titlecolor,cid+'_t');
titlebar.style.overflow="hidden";
titlebar.style.cursor="default";
var tmp=(isresize)?'<img src="'+minImg+'" width="16" height="16" id="'+cid+'_min"><img src="'+maxImg+'" width="16" height="16"  id="'+cid+'_max">':'';
titlebar.innerHTML='<span style="position:absolute; left:3px; top:1px; font:bold 10pt sans-serif; color:'+titletextcolor+'; height:18px; overflow:hidden; clip-height:16px;">'+title+'</span><div id="'+cid+'_btt" style="position:absolute; width:48px; height:16px; left:'+(tw-48)+'px; top:2px; text-align:right">'+tmp+'<img src="'+clsImg+'" width="16" height="16" id="'+cid+'_cls"></div>';
tw=(ie5)?w-7:w-13;
var content=new subBox(2,24,tw,h-36,bgcolor,cid+'_c');
content.style.borderColor=bordercolor;
content.style.borderWidth="2px";
if(isExt){
content.innerHTML='<iframe id="'+cid+'_ifrm" src="'+text+'" width="100%" height="100%"></iframe>';
content.style.overflow="hidden";
}else{
if(ie5)content.style.scrollbarBaseColor=scrollcolor;
content.style.borderStyle="inset";
content.style.overflow="auto";
content.style.padding="0px 2px 0px 4px";
content.innerHTML=text;
content.style.font=fontstyleset;
content.style.color=textcolor;
}
var extWA=new subBox(2,24,0,0,'',cid+'_extWA');
extWA.style.display="none";
extWA.style.width='100%';
extWA.style.height='100%';
outerdiv.appendChild(titlebar);
outerdiv.appendChild(content);
outerdiv.appendChild(extWA);
outerdiv.appendChild(rdiv);
d.body.appendChild(shadow);
d.body.appendChild(outerdiv);
d.gEl=d.getElementById;
if(!showonstart)hidebox(cid);
var wB=d.gEl(cid+'_b');
wB.cid=cid;
wB.isExt=(isExt)?true:false;
var wT=d.gEl(cid+'_t');
wT.cid=cid;
if(isresize){
var wRS=d.gEl(cid+'_rs');
wRS.cid=cid;
var wMIN=d.gEl(cid+'_min');
wMIN.cid=cid;
var wMAX=d.gEl(cid+'_max');
wMAX.h=h;
wMAX.cid=cid;
wMIN.onclick=minimize;
wMAX.onclick=restore;
wRS.onmousedown=startRS;
wRS.onmouseup=stopRS;
}
var wCLS=d.gEl(cid+'_cls');
var wEXTWA=d.gEl(cid+'_extWA');
wB.activecolor=titlecolor;
wB.inactivecolor=scrollcolor;
if(oldac!=null)d.gEl(oldac.cid+"_t").style.backgroundColor=oldac.inactivecolor;
oldac=wB;
wCLS.onclick=new Function("hidebox('"+cid+"');");
wB.onmousedown=function(){ changez(this) }
if(isdrag){
wT.onmousedown=grab_id;
wT.onmouseup=stopdrag;
}
}else{
if(oldOK){
var ctr=new Date();
ctr=ctr.getTime();
var t=(isExt)?text:'';
var posn=(ns4)? 'screenX='+x+',screenY='+y: 'left='+x+',top='+y;
var win=window.open(t , "abc"+ctr , "status=no,menubar=no,width="+w+",height="+h+",resizable="+((isresize)?"yes":"no")+",scrollbars=yes,"+posn);
if(!isExt){
t='<html><head><title>'+title+'</title></head><body bgcolor="'+bgcolor+'"><font style="font:'+fontstyleset+'; color:'+textcolor+'">'+text+'</font></body></html>';
win.document.write(t);
win.document.close();
}}}}}

if(ns6)setInterval('movepopup()',40);

if(w3c){
d.onmousemove=trackmouse;
d.onmouseup=new Function("currRS=null");
}