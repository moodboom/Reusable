// ==UserScript==
// @name           gReader_Comments
// @namespace      http://www.thedigitalmachine.com
// @description    Enables the IntenseDebate comment system for every entry in Google Reader.
// @include         http://reader.google.com/*
// @include         https://reader.google.com/*
// @include         http://reader.google.com/reader/*
// @include         https://reader.google.com/reader/*
// @include         http://www.google.com/reader/*
// @include         https://www.google.com/reader/*
// ==/UserScript==
// GUID: 517529b8-95f3-4bfa-b9ee-554eb0e28c96
// version: 3.0b3

function getFirstElementMatchingClassName(root,tag,class)
{
  var elements=root.getElementsByTagName(tag); var i=0;
  while (elements[i] && !elements[i].className.match(class)) { i++; }	
  return ((!elements[i]) ? null : (elements[i]));
}

function getElementsByClassName(root,tag,class)
{
  var elements = root.getElementsByTagName(tag);
  var results = new Array();
  for(var i=0; i<elements.length; i++) { if(elements[i].className.indexOf(class)>-1) { results.push(elements[i]); } }
  return (results);
}

function findParentNode(el,tag,class)
{
  el=el.parentNode;
  if (arguments.length==3)
  {
    // Find first element's parent node matching tag and className
    while (el.nodeName.toLowerCase()!='body' && (el.nodeName.toLowerCase()!=tag || (el.className!=class && el.className.indexOf(class+' ')==-1))) { el=el.parentNode; }
    return ((el.nodeName.toLowerCase()!='body') ? el : false);
  }
  else
  {
    // Find first element's parent node matching tag
    while (el.nodeName.toLowerCase()!='body' && el.nodeName.toLowerCase()!=tag) { el=el.parentNode; }
    return ((el.nodeName.toLowerCase()!='body') ? el : false);
  }	
}

function addStyles(css)
{
  var head=document.getElementsByTagName('head')[0];
  if (head)
  {
    var style=document.createElement('style');
    style.type='text/css';
    style.innerHTML=css;
    head.appendChild(style);
  }
}

function catchEntryAdded(e)
{
    var el=e.target;

    if (el.nodeName=='DIV' && el.className.indexOf('entry')>-1)
    {
      if (el.className.indexOf('entry-actions')>-1)
      {
        // Expanding article in list view
        addpreviewsButton(el);	
      }
      else if (getFirstElementMatchingClassName(el,'tbody','card-tbody'))
      {
        // Adding article in expanded view
        addpreviewsButton(getFirstElementMatchingClassName(el,'div','entry-actions'));
      }
    }
}

function addpreviewsButton(el)
{
  // Top link
  var entry=findParentNode(el,'div','entry');	
		 var link  = getFirstElementMatchingClassName(entry,'a','entry-title-link');		 
 		 var link2 = getFirstElementMatchingClassName(entry,'a','entry-title-link');
		 var e = encodeURIComponent;
		 var ttitle = e(link2.innerHTML);
		 var linkurl = e(link2);
		 var title = new Array();
		 title = ttitle.split('%3Cimg');
		 //title[0] = e(title[0]);
 //link.addEventListener('click', previewsMouseClick, false);
  // link.addEventListener('click', function(e) { if (e.ctrlKey) { previewsMouseClick(e); }  }, false);
  
//var idcomments_acct = '511633b778533829312a3c8e5a1ecdd6';
//var idcomments_post_id=getFirstElementMatchingClassName(entry,'a','entry-title-link');
//var idcomments_post_url=getFirstElementMatchingClassName(entry,'a','entry-title-link');
//var idcomments_post_title="Google Reader Comments";
//var commentScript = document.createElement("SCRIPT");
//commentScript.type = "text/javascript";
//commentScript.src = "http://www.intensedebate.com/js/getCommentLink.php?acct="+idcomments_acct+"&posttitle="+idcomments_post_title+"&posturl="+idcomments_post_url;
  
  // Bottom button
  linkurl = linkurl.replaceAll("%26","%3F-%3F");
  title[0] = title[0].replace("%0A", "");
  var previews=document.createElement('span');
  previews.className='item-previews previews link';

  // MDM This needs an overhaul to decouple from sixteenseven's version 3.1.3.
  // How did this (adding javascript after the page has loaded) ever work?
  // previews.innerHTML = '<script>var idcomments_acct = "511633b778533829312a3c8e5a1ecdd6"; var idcomments_post_id = "' + link + '"; var idcomments_post_url = "http://www.sixteenseven.com/comments.php?url=' + linkurl + '============' + title[0] + '"; var idcomments_post_title = "' + title[0] + '"; </script> <script type="text/javascript" src="http://www.intensedebate.com/js/genericLinkWrapper.js"></script>';
  previews.innerHTML = 'Comments';
  
  // MDM Before, a hardcoded account was used.  Now we need to look one up via AJAX.
  // But that means we'll need to take the result and build the comments javascript.
  // We can't inject that, so we need a second AJAX call to get the comment count.
  // Tabling this for now until Jon can give me an AJAX  URL for comment count.
  // previews.id = "previews_" + linurl;
  // previews.innerHTML = "Looking for comments...";
  // ajaxRequestCommentCount( previews.id );

  el.appendChild(previews);
  previews.addEventListener('click', previewsMouseClick, false);	
}

function calcEntryIndex(e)
{
    var index=0;
    while (e.previousSibling)
    {
      index++;
      e=e.previousSibling;
    }
    return index;
}

function previewsMouseClick(e)
{
    var el=e.target;
    var entry=findParentNode(el,'div','entry');
    var index = calcEntryIndex(entry);
    previews(entry,index);
    e.preventDefault();
}

function previewsShortcut()
{
  var index=unsafeWindow.s.ma;
  if (index>-1)
  {
    // An article is selected, previews it
    previews(getEntryDOMObject(index),index);
  }
}

function previews(entry,index)
{
    var previews;
    var button = getFirstElementMatchingClassName(entry,'span','item-previews');
    
    // Update entry with previews mode, need to do it before scrolling, because scrolling will repaint previews button (list view only)
    if (entry.className.indexOf('previews')==-1)
    {
      entry.className=entry.className+' previews';
      previews=true;
    }
    else
    {
      entry.className=entry.className.replace('previews','');    	
      previews=false;
    }
    	    
    // Need to scroll before changing entry-body, because scrolling repaints article from scratch (list view only)
    scrollTo(index);
    
    root = getEntryDOMObject(index);
    var body = getFirstElementMatchingClassName(entry,'div','entry-body');
    var ins = body.getElementsByTagName('ins')[0];

    if (previews)
    {
       // classic mode-> previews mode

       // hide rss item
       //ins.style.display='none';
       
       // iframe creation/display
        var iframe = getFirstElementMatchingClassName(entry,'iframe','previews');
        var link2 = getFirstElementMatchingClassName(entry,'a','entry-title-link');
        var e = encodeURIComponent;
        var ttitle = e(link2.innerHTML);
        var title = new Array();
        var linkurl = e(link2);
        title = ttitle.split('%3Cimg');
        if (iframe)
            {
                // iframe already in document, display it
                iframe.style.display='block';
            }
        else
            {
                // iframe not in document, create it

                // We now use a single-round-trip script in the iframe.
                // Eventually it will be hosted on IntenseDebate.
                var comments_url = "http://intensedebate.com/commentPopup.php?acct=" + account + "&posturl=" + url;

                var iframe = document.createElement('iframe');
                iframe.setAttribute('width','100%');
                iframe.setAttribute('height','300');
                iframe.setAttribute('frameborder','0');
                iframe.className='previews';

                iframe.setAttribute('src',comments_url);
                body.appendChild(iframe);

                // -----------------------------------------------
                // OLD (pre-single-round-trip)
                // -----------------------------------------------
                
                // We'll need the account number, retrieved via AJAX.
                // Then we can build a URL for the iframe.
                
                // We want to know where to inject the iframe, way later.
                // Let's set up a DOM ID to do it...
                // body.id = linkurl;
                
                // ajaxRequestIframe( linkurl, entry );
                // -----------------------------------------------

            }		
        
        // Scale article container to fullwidth
        body.setAttribute('style','max-width: 98%');
    }
    else
    {
       // previews mode -> classic mode
       
       // hide iframe
       var iframe = getFirstElementMatchingClassName(entry,'iframe','previews');
       if (iframe) iframe.style.display='none';
       
       // show rss item
       ins.style.display='block';
       
       // Go back to initial width
       body.removeAttribute('style','');     
    }  
}      

function handleKeypress(e)
{
  // Handle a Shift-V keypress
  if (e.target.nodeName.toLowerCase()!='input' && e.shiftKey && e.keyCode==86)
  {
    previewsShortcut();
    e.preventDefault();
  }
}

function getEntryDOMObject(index)
{
    // Because of repaint, entry doesn't point to correct DOM object, we need to find entry using index
    var entries=document.getElementById('entries');
    var i=0;
    entry=entries.firstChild;
    while ((i++)<index)
    {
      entry=entry.nextSibling;
    }	
    return entry;
}       

function scrollTo(index)
{   
    // Force scrolling to top of article
    try
    {
      location.href = 'javascript:void(s.ma=-1);';
      location.href = 'javascript:void(s.hf('+index+'));';    
    }
    catch(err) { }
}      
       
function restyle()
{
    // Overwrites Better GReader extension css modifications regarding entry-actions class.
    // Indeed, entry-actions was set to "float : right", thus div was not in document flow.
    // Then, clicking on previews button let entry actions div in place instead of going down automatically when iframe was added.
    // That's why I use here text-align: right. That has the same effect, but keeps div in document flow.
    // restyle() is called after document load, in order to ensure that Better GReader has already added its styles modifications
    var styles = document.getElementsByTagName('head')[0].getElementsByTagName('style');
    var i=0;
    
    while (i<styles.length)
    {
    	if (styles[i].innerHTML.indexOf('.entry-actions { float:right !important; }')>-1)
    	{
          styles[i].innerHTML=styles[i].innerHTML.replace('.entry-actions { float:right !important; }','.entry-actions { text-align: right; !important; }');
    	}
    	i++;
    }
}

// Replaces all instances of the given substring.
String.prototype.replaceAll = function( 
	strTarget, // The substring you want to replace
	strSubString // The string you want to replace in.
	){
	var strText = this;
	var intIndexOfMatch = strText.indexOf( strTarget );
	 
	// Keep looping while an instance of the target string
	// still exists in the string.
	while (intIndexOfMatch != -1){
		// Relace out the current instance.
		strText = strText.replace( strTarget, strSubString )
		 
		// Get the index of any next matching substring.
		intIndexOfMatch = strText.indexOf( strTarget );
	}
	 
	// Return the updated string with ALL the target strings
	// replaced out with the new substring.
	return( strText );
}

function init()
{
  restyle();
  addStyles('span.item-previews { background: url("data:image/gif,GIF89a%10%00%10%00%D5%13%00%D8%D8%D8%FA%FA%FA%CB%CB%CB%C8%C8%C8%D2%D2%D2%BA%BA%BA%C6%C6%C6%A1%A1%A1%9C%9C%9C%BD%BD%BD%C9%C9%C9%AB%AB%AB%F4%F4%F4%BF%BF%BF%FC%FC%FC%DB%DB%DB%AD%AD%AD%FF%FF%FF%CC%CC%CC%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00!%F9%04%01%00%00%13%00%2C%00%00%00%00%10%00%10%00%00%06I%C0%89pH%2C%1A%8F%C8d%F1!i%3A%9F%8F%E1%03B%ADZ%A9%D1%89%04%12%E9z%BF%10%89p%FB-G%C2c%AE%D9%8B%D6%AA%03_%F8Y%EC%8E%C8%E3%F3%F4%9AM\'%7B%1D%0E%60t%00W%85%10%00RO%8A%12YJ%8E%8EA%00%3B") no-repeat; padding-left: 16px; } div.entry.previews span.item-previews { background: url("data:image/gif,GIF89a%10%00%10%00%C2%05%00%3BY%98%AD%AD%AD%CC%CC%CC%D8%D8%D8%DB%DB%DB%FF%FF%FF%FF%FF%FF%FF%FF%FF!%F9%04%01%00%00%07%00%2C%00%00%00%00%10%00%10%00%00%036x%BA%DC%FE06%22%AA%BDd%91%C0%BB%E7%D9!%04%40i%9E%81%A0%8Cg%0B%A4%2B%E9%9A%B0(%CF%AFj%E3e%CD%F2%BE%DB%AC6%F8%18%03%03%CDe)%08I%9E%CF%04%00%3B") no-repeat; padding-left: 16px; }');
}
      
// MDM Here I'm attempting a GM call...

function makeRequest(call_index, extra_data, extra_data2, url, parameters) {

    var full_url = url + parameters;
    var response_text;
    GM_xmlhttpRequest({
            method: 'GET',
                url: full_url,
                headers: {
                'User-agent': 'Mozilla/4.0 (compatible) Greasemonkey',
                    'Accept': 'application/atom+xml,application/xml,text/xml',
                    },
                onload: function(responseDetails) {

                // DEBUG
                // alert('status = ' + responseDetails.status + ' ' + responseDetails.statusText + '\n\ndata =\n' + responseDetails.responseText);

                // Results include responseDetails.status and responseDetails.responseText.
                // response_text = responseDetails.responseText;

                postProcess( call_index, extra_data, extra_data2, responseDetails.responseText );
            }
        });
}


function postProcess( call_index, extra_data, extra_data2, response_text )
{
    if ( call_index == 1 )
    {
        // response_text = the blog url.
        // extra_data = linkurl
        // extra_data2 = entry
        
        // We make ANOTHER AJAX request to get the account number.
        // We call injectIframe in the post-processing for that.
        // makeRequest( call_index, linkurl, entry, 'http://intensedebate.com/services/v1/siteHasID','?appKey=testid&url=http://jonefox.com/blog' );
        call_index = 2;
        makeRequest( call_index, extra_data, extra_data2, 'http://intensedebate.com/services/v1/siteHasID','?appKey=testid&url='+response_text );

        // alert( response_text ); 

    } else if ( call_index == 2 )
    {
        // Create the iframe and set the URL.
        injectIframe( extra_data, extra_data2, response_text );
    }
}


function ajaxRequestIframe( linkurl, entry )
{
    var call_index = 1;

    // We need to get the account number for this feed.
    // First, get the feed...
    var feedurl = getFirstElementMatchingClassName(entry,'a','entry-source-title').href;
    // if feedurl
    feedurl = feedurl.split("reader/view/feed/")[1];
    
    // Un-replace %3A with : and %2F with /.
    feedurl = feedurl.replace(/%3A/g, ":");
    feedurl = feedurl.replace(/%2F/g, "/");

    // Try to get the url from the feed.
    // Break it down - can we do it??
    // Start by...?
    // feedurl = feedurl.replace(/\/feed/g, "");
    // feedurl = feedurl.replace(/\/atom/g, "");

    // DEBUG
    // Only the first one works (only ID blogs are supported at the moment).
    // feedurl = "http://jonefox.com/blog";
    // feedurl = "http://rss.slashdot.org/Slashdot/slashdot";
    // feedurl = "http://slashdot.org";
    // feedurl = "http://planet.gentoo.org";
    // alert(feedurl);

    // We make the AJAX request to get the blog url from the feed url.
    // Followup occurs in post-processing, to get account number, and then inject iframe.
    makeRequest( call_index, linkurl, entry, 'http://thedigitalmachine.com/get_blog_url_from_feed_url.php', '?query_index=1&feed_url='+feedurl );

}

function injectIframe( url, entry, response_text )
{
    // The result is a JSON object.
    // We use the acct var to build the iframe URL.
    // THEN we can insert the iframe.

    // Extract the JSON data.
    var json_object = eval('(' + response_text + ')');
    var account = json_object.data.acct;
    // alert( 'account = ' + account );

    // Get the iframe location.
    // root = getEntryDOMObject(index);

    // MDM Trying to get body.
    // 1st attempt = pass entry around and use it.
    // var body = getFirstElementMatchingClassName(entry,'div','entry-body');
    // 2nd attempt = set an id beforehand, use it to find the body.
    var body = document.getElementById(url);

    // Build the iframe object.
    // Insert it.

    url = url.replaceAll("%26","%3F-%3F");

    var iframe = document.createElement('iframe');

    iframe.setAttribute('width','100%');
    iframe.setAttribute('height','300');
    iframe.setAttribute('frameborder','0');
    iframe.className='previews';

    // From Jon:
    // For the comment section itself, loading it in an iframe seems like the           
    // best option on the surface.  You can use the page we use to display the
    // comments for the popup version of our system (when you click the comment
    // link it opens in a new window).  The url for this page is:
    // 
    // http://intensedebate.com/commentPopup.php?postid=32&acct=3f0bab8ef3a8376d641d43458f7a358a&posttitle=Post%20Title&posturl=http://jonefox.com/blog/?p=32
    //
    // Where acct is the acct value pulled from the previous API call, posturl
    // is the permalink url of the blogpost, posttitle is the title of the post,
    // and postid is the blog post id from the native comment system (unique
    // identifier for the post).  You should have no problem extracting the url
    // and the title, but the postid might be a challenge.  If you can get it,
    // it's the best way to find the post and ensure we don't end up
    // splintering the comments into two different comment threads, but if this isn't
    // possible (I'm not sure it is) then you can just pass what you know and
    // we'll do the best we can to match it.

    // NOTE: I don't see ANY way to get the "postid", Jon's own blog doesn't use one for more recent posts.
    // var comments_url = "http://intensedebate.com/commentPopup.php?postid=32&acct=" + account + "&posttitle=Post%20Title&posturl=" + url;
    // NOTE: I'm also not getting post title, yet...
    // var comments_url = "http://intensedebate.com/commentPopup.php?acct=" + account + "&posttitle=Post%20Title&posturl=" + url;
    var comments_url = "http://intensedebate.com/commentPopup.php?acct=" + account + "&posturl=" + url;
    // alert( comments_url );

    iframe.setAttribute('src',comments_url);
    body.appendChild(iframe);

    // OLD

    // MDM THIS NEEDS TO GO.
    // iframe.setAttribute('src','http://www.sixteenseven.com/comments.php?url='+linkurl+'============'+title[0]);
    
    // Testing any old url... yep, works fine.  :>
    // iframe.setAttribute('src','http://news.thedigitalmachine.com');
    
    // We could try to set the URL we've built as the frame src, but the URL just returns javascript.
    //
    // Now BUILD THE CORRECT URL as is done here:
    //
    //    http://www.intensedebate.com/js/genericCommentWrapperV2.js
    //
    // We'll need acct, postid, title, url.
    // var my_acct = '278e1ad127da50bbf55fe1262be7f330';
    // var my_post_title = document.title;
    // if (my_post_title.length==0)
    //     my_post_title="Post";
    // var my_post_url = window.location;
    // var my_post_id = window.location;
    // var my_post_url = "http://www.intensedebate.com/js/genericCommentWrapper2.php?acct="+my_acct+"&postid="+my_post_id+"&title="+my_post_title+"&url="+my_post_url;
    // iframe.setAttribute('src',my_post_url);

    // Try giving a name and using window["iframe_name"], below...
    // iframe.setAttribute('name','previews');
    // iframe.name='iframe_name';
    // iframe.id='iframe_name';
    // alert(iframe.id);   

    // Temp solution: use my own third-party page...
    // Hopefully Jon@IntenseDebate.com can set me up with something better...
    // iframe.setAttribute('src','http://thedigitalmachine.com/intensedebatetest.html');
        
    // body.appendChild(iframe);
}

document.body.addEventListener('DOMNodeInserted', catchEntryAdded, false);
//document.addEventListener('keydown',handleKeypress, false);
window.addEventListener('load',init,false);
