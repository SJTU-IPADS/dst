//>>built
define("dojox/highlight/widget/Code", ["dojo", "dijit", "dijit/_Widget", "dijit/_Templated", "dojox/highlight"], function(dojo, dijit){

	dojo.declare("dojox.highlight.widget.Code",[dijit._Widget, dijit._Templated],{
		// summary:
		//		A simple source code formatting widget that adds line numbering, alternating line colors
		//		and line range support on top of dojox.highlight module.

		url: "",
		range:null,
		style:"",
		listType:"1",
		lang:"",

		// Note: If more control over formatting is required, the order list items can be replaced
		// with a table implementation instead... Excercise is left for those that need it...
		templateString:
			'<div class="formatted" style="${style}">'+
				'<div class="titleBar"></div>'+
				'<ol type="${listType}" dojoAttachPoint="codeList" class="numbers"></ol>' +
				'<div style="display:none" dojoAttachPoint="containerNode"></div>' +
			'</div>',
	
		postCreate: function(){
			this.inherited(arguments);
			if(this.url){
				// load from a url
				dojo.xhrGet({
					url: this.url,
					// then poopulate:
					load: dojo.hitch(this,"_populate"),
					error: dojo.hitch(this,"_loadError")
				});
			}else{
				// or just populate from our internal content
				this._populate(this.containerNode.innerHTML);
			}
		},
	
		_populate: function(data){
			// put the content in a common node
			this.containerNode.innerHTML =
				"<pre><code class='" + this.lang + "'>" +
					data.replace(/\</g,"&lt;") +
				"</code></pre>";
			// highlight it
			dojo.query("pre > code",this.containerNode).forEach(dojox.highlight.init);
			// FIXME: in ie7, the innerHTML in a real <pre> isn't split by \n's ?
			// split the content into lines
			var lines = this.containerNode.innerHTML.split("\n");
			dojo.forEach(lines,function(line,i){
				// setup all the lines of the content as <li>'s
				var li = dojo.doc.createElement('li');
				// add some style sugar:
				dojo.addClass(li, (i % 2 !== 0 ? "even" : "odd"));
				line = "<pre><code>" + line + "&nbsp;</code></pre>";
				line = line.replace(/\t/g," &nbsp; ");
				li.innerHTML = line;
				this.codeList.appendChild(li);
			},this);
			// save our data
			this._lines = dojo.query("li",this.codeList);
			this._updateView();
		},
	
		// FIXME: user _setRangeAttr pattern? so you can code.set('range', [1, 100]);
		setRange: function(/* Array */range){
			// summary: update the view to a new passed range
			if(dojo.isArray(range)){
				this.range = range;
				this._updateView();
			}
		},
	
		_updateView: function(){
			// summary: set the list to the current range
			if(this.range){
				var r = this.range;
				this._lines
					// hide them all
					.style({ display:"none" })
					.filter(function(n,i){
						// remove nodes out of range
						return (i + 1 >= r[0] && i + 1 <= r[1]);
					})
					// set them visible again
					.style({ display:"" })
				;
				// set the "start" attribute on the OL so numbering works
				dojo.attr(this.codeList,"start",r[0]);
			}
		},
	
		_loadError: function(error){
			// summary: a generic error handler for the url=""
			console.warn("loading: ", this.url, " FAILED", error);
		}

	});

}); 