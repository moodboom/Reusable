

function getFirstElementMatchingClassName(root,tag,class)
{var elements=root.getElementsByTagName(tag);var i=0;while(elements[i]&&!elements[i].className.match(class)){i++;}
return((!elements[i])?null:(elements[i]));}
function getElementsByClassName(root,tag,class)
{var elements=root.getElementsByTagName(tag);var results=new Array();for(var i=0;i<elements.length;i++){if(elements[i].className.indexOf(class)>-1){results.push(elements[i]);}}
return(results);}
function findParentNode(el,tag,class)
{el=el.parentNode;if(arguments.length==3)
{while(el.nodeName.toLowerCase()!='body'&&(el.nodeName.toLowerCase()!=tag||(el.className!=class&&el.className.indexOf(class+' ')==-1))){el=el.parentNode;}
return((el.nodeName.toLowerCase()!='body')?el:false);}
else
{while(el.nodeName.toLowerCase()!='body'&&el.nodeName.toLowerCase()!=tag){el=el.parentNode;}
return((el.nodeName.toLowerCase()!='body')?el:false);}}
function addStyles(css)
{var head=document.getElementsByTagName('head')[0];if(head)
{var style=document.createElement('style');style.type='text/css';style.innerHTML=css;head.appendChild(style);}}
function catchEntryAdded(e)
{var el=e.target;if(el.nodeName=='DIV'&&el.className.indexOf('entry')>-1)
{if(el.className.indexOf('entry-actions')>-1)
{addpreviewsButton(el);}
else if(getFirstElementMatchingClassName(el,'tbody','card-tbody'))
{addpreviewsButton(getFirstElementMatchingClassName(el,'div','entry-actions'));}}}
function addpreviewsButton(el)
{var entry=findParentNode(el,'div','entry');var link=getFirstElementMatchingClassName(entry,'a','entry-title-link');var link2=getFirstElementMatchingClassName(entry,'a','entry-title-link');var e=encodeURIComponent;var ttitle=e(link2.innerHTML);var linkurl=e(link2);var title=new Array();title=ttitle.split('%3Cimg');linkurl=linkurl.replaceAll("%26","%3F-%3F");title[0]=title[0].replace("%0A","");var previews=document.createElement('span');previews.className='item-previews previews link';previews.innerHTML='Comments';el.appendChild(previews);previews.addEventListener('click',previewsMouseClick,false);}
function calcEntryIndex(e)
{var index=0;while(e.previousSibling)
{index++;e=e.previousSibling;}
return index;}
function previewsMouseClick(e)
{var el=e.target;var entry=findParentNode(el,'div','entry');var index=calcEntryIndex(entry);previews(entry,index);e.preventDefault();}
function previewsShortcut()
{var index=unsafeWindow.s.ma;if(index>-1)
{previews(getEntryDOMObject(index),index);}}
function previews(entry,index)
{var previews;var button=getFirstElementMatchingClassName(entry,'span','item-previews');if(entry.className.indexOf('previews')==-1)
{entry.className=entry.className+' previews';previews=true;}
else
{entry.className=entry.className.replace('previews','');previews=false;}
scrollTo(index);root=getEntryDOMObject(index);var body=getFirstElementMatchingClassName(entry,'div','entry-body');var ins=body.getElementsByTagName('ins')[0];if(previews)
{var iframe=getFirstElementMatchingClassName(entry,'iframe','previews');var link2=getFirstElementMatchingClassName(entry,'a','entry-title-link');var e=encodeURIComponent;var ttitle=e(link2.innerHTML);var title=new Array();var linkurl=e(link2);title=ttitle.split('%3Cimg');if(iframe)
{iframe.style.display='block';}
else
{var comments_url="http://intensedebate.com/commentPopup.php?acct="+account+"&posturl="+url;var iframe=document.createElement('iframe');iframe.setAttribute('width','100%');iframe.setAttribute('height','300');iframe.setAttribute('frameborder','0');iframe.className='previews';iframe.setAttribute('src',comments_url);body.appendChild(iframe);}
body.setAttribute('style','max-width: 98%');}
else
{var iframe=getFirstElementMatchingClassName(entry,'iframe','previews');if(iframe)iframe.style.display='none';ins.style.display='block';body.removeAttribute('style','');}}
function handleKeypress(e)
{if(e.target.nodeName.toLowerCase()!='input'&&e.shiftKey&&e.keyCode==86)
{previewsShortcut();e.preventDefault();}}
function getEntryDOMObject(index)
{var entries=document.getElementById('entries');var i=0;entry=entries.firstChild;while((i++)<index)
{entry=entry.nextSibling;}
return entry;}
function scrollTo(index)
{try
{location.href='javascript:void(s.ma=-1);';location.href='javascript:void(s.hf('+index+'));';}
catch(err){}}
function restyle()
{var styles=document.getElementsByTagName('head')[0].getElementsByTagName('style');var i=0;while(i<styles.length)
{if(styles[i].innerHTML.indexOf('.entry-actions { float:right !important; }')>-1)
{styles[i].innerHTML=styles[i].innerHTML.replace('.entry-actions { float:right !important; }','.entry-actions { text-align: right; !important; }');}
i++;}}
String.prototype.replaceAll=function(strTarget,strSubString){var strText=this;var intIndexOfMatch=strText.indexOf(strTarget);while(intIndexOfMatch!=-1){strText=strText.replace(strTarget,strSubString)
intIndexOfMatch=strText.indexOf(strTarget);}
return(strText);}
function init()
{restyle();addStyles('span.item-previews { background: url("data:image/gif,GIF89a%10%00%10%00%D5%13%00%D8%D8%D8%FA%FA%FA%CB%CB%CB%C8%C8%C8%D2%D2%D2%BA%BA%BA%C6%C6%C6%A1%A1%A1%9C%9C%9C%BD%BD%BD%C9%C9%C9%AB%AB%AB%F4%F4%F4%BF%BF%BF%FC%FC%FC%DB%DB%DB%AD%AD%AD%FF%FF%FF%CC%CC%CC%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00!%F9%04%01%00%00%13%00%2C%00%00%00%00%10%00%10%00%00%06I%C0%89pH%2C%1A%8F%C8d%F1!i%3A%9F%8F%E1%03B%ADZ%A9%D1%89%04%12%E9z%BF%10%89p%FB-G%C2c%AE%D9%8B%D6%AA%03_%F8Y%EC%8E%C8%E3%F3%F4%9AM\'%7B%1D%0E%60t%00W%85%10%00RO%8A%12YJ%8E%8EA%00%3B") no-repeat; padding-left: 16px; } div.entry.previews span.item-previews { background: url("data:image/gif,GIF89a%10%00%10%00%C2%05%00%3BY%98%AD%AD%AD%CC%CC%CC%D8%D8%D8%DB%DB%DB%FF%FF%FF%FF%FF%FF%FF%FF%FF!%F9%04%01%00%00%07%00%2C%00%00%00%00%10%00%10%00%00%036x%BA%DC%FE06%22%AA%BDd%91%C0%BB%E7%D9!%04%40i%9E%81%A0%8Cg%0B%A4%2B%E9%9A%B0(%CF%AFj%E3e%CD%F2%BE%DB%AC6%F8%18%03%03%CDe)%08I%9E%CF%04%00%3B") no-repeat; padding-left: 16px; }');}
function makeRequest(call_index,extra_data,extra_data2,url,parameters){var full_url=url+parameters;var response_text;GM_xmlhttpRequest({method:'GET',url:full_url,headers:{'User-agent':'Mozilla/4.0 (compatible) Greasemonkey','Accept':'application/atom+xml,application/xml,text/xml',},onload:function(responseDetails){postProcess(call_index,extra_data,extra_data2,responseDetails.responseText);}});}
function postProcess(call_index,extra_data,extra_data2,response_text)
{if(call_index==1)
{call_index=2;makeRequest(call_index,extra_data,extra_data2,'http://intensedebate.com/services/v1/siteHasID','?appKey=testid&url='+response_text);}else if(call_index==2)
{injectIframe(extra_data,extra_data2,response_text);}}
function ajaxRequestIframe(linkurl,entry)
{var call_index=1;var feedurl=getFirstElementMatchingClassName(entry,'a','entry-source-title').href;feedurl=feedurl.split("reader/view/feed/")[1];feedurl=feedurl.replace(/%3A/g,":");feedurl=feedurl.replace(/%2F/g,"/");makeRequest(call_index,linkurl,entry,'http://thedigitalmachine.com/get_blog_url_from_feed_url.php','?query_index=1&feed_url='+feedurl);}
function injectIframe(url,entry,response_text)
{var json_object=eval('('+response_text+')');var account=json_object.data.acct;var body=document.getElementById(url);url=url.replaceAll("%26","%3F-%3F");var iframe=document.createElement('iframe');iframe.setAttribute('width','100%');iframe.setAttribute('height','300');iframe.setAttribute('frameborder','0');iframe.className='previews';var comments_url="http://intensedebate.com/commentPopup.php?acct="+account+"&posturl="+url;iframe.setAttribute('src',comments_url);body.appendChild(iframe);}
document.body.addEventListener('DOMNodeInserted',catchEntryAdded,false);window.addEventListener('load',init,false);