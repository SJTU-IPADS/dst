//>>built
define("dojox/widget/Toaster",["dojo/_base/declare","dojo/_base/lang","dojo/_base/connect","dojo/_base/fx","dojo/dom-style","dojo/dom-class","dojo/dom-geometry","dijit/registry","dijit/_WidgetBase","dijit/_TemplatedMixin","dijit/BackgroundIframe","dojo/fx","dojo/has","dojo/_base/window","dojo/window"],function(_1,_2,_3,_4,_5,_6,_7,_8,_9,_a,_b,_c,_d,_e,_f){
_2.getObject("dojox.widget",true);
var _10=function(w){
return w.substring(0,1).toUpperCase()+w.substring(1);
};
return _1("dojox.widget.Toaster",[_9,_a],{templateString:"<div class=\"dijitToasterClip\" dojoAttachPoint=\"clipNode\"><div class=\"dijitToasterContainer\" dojoAttachPoint=\"containerNode\" dojoAttachEvent=\"onclick:onSelect\"><div class=\"dijitToasterContent\" dojoAttachPoint=\"contentNode\"></div></div></div>",messageTopic:"",messageTypes:{MESSAGE:"message",WARNING:"warning",ERROR:"error",FATAL:"fatal"},defaultType:"message",positionDirection:"br-up",positionDirectionTypes:["br-up","br-left","bl-up","bl-right","tr-down","tr-left","tl-down","tl-right"],duration:2000,slideDuration:500,separator:"<hr></hr>",postCreate:function(){
this.inherited(arguments);
this.hide();
_e.body().appendChild(this.domNode);
if(this.messageTopic){
_3.subscribe(this.messageTopic,this,"_handleMessage");
}
},_handleMessage:function(_11){
if(_2.isString(_11)){
this.setContent(_11);
}else{
this.setContent(_11.message,_11.type,_11.duration);
}
},setContent:function(_12,_13,_14){
_14=_14||this.duration;
if(this.slideAnim){
if(this.slideAnim.status()!="playing"){
this.slideAnim.stop();
}
if(this.slideAnim.status()=="playing"||(this.fadeAnim&&this.fadeAnim.status()=="playing")){
setTimeout(_2.hitch(this,function(){
this.setContent(_12,_13,_14);
}),50);
return;
}
}
for(var _15 in this.messageTypes){
_6.remove(this.containerNode,"dijitToaster"+_10(this.messageTypes[_15]));
}
_5.set(this.containerNode,"opacity",1);
this._setContent(_12);
_6.add(this.containerNode,"dijitToaster"+_10(_13||this.defaultType));
this.show();
var _16=_7.getMarginBox(this.containerNode);
this._cancelHideTimer();
if(this.isVisible){
this._placeClip();
if(!this._stickyMessage){
this._setHideTimer(_14);
}
}else{
var _17=this.containerNode.style;
var pd=this.positionDirection;
if(pd.indexOf("-up")>=0){
_17.left=0+"px";
_17.top=_16.h+10+"px";
}else{
if(pd.indexOf("-left")>=0){
_17.left=_16.w+10+"px";
_17.top=0+"px";
}else{
if(pd.indexOf("-right")>=0){
_17.left=0-_16.w-10+"px";
_17.top=0+"px";
}else{
if(pd.indexOf("-down")>=0){
_17.left=0+"px";
_17.top=0-_16.h-10+"px";
}else{
throw new Error(this.id+".positionDirection is invalid: "+pd);
}
}
}
}
this.slideAnim=_c.slideTo({node:this.containerNode,top:0,left:0,duration:this.slideDuration});
this.connect(this.slideAnim,"onEnd",function(_18,_19){
this.fadeAnim=_4.fadeOut({node:this.containerNode,duration:1000});
this.connect(this.fadeAnim,"onEnd",function(evt){
this.isVisible=false;
this.hide();
});
this._setHideTimer(_14);
this.connect(this,"onSelect",function(evt){
this._cancelHideTimer();
this._stickyMessage=false;
this.fadeAnim.play();
});
this.isVisible=true;
});
this.slideAnim.play();
}
},_setContent:function(_1a){
if(_2.isFunction(_1a)){
_1a(this);
return;
}
if(_1a&&this.isVisible){
_1a=this.contentNode.innerHTML+this.separator+_1a;
}
this.contentNode.innerHTML=_1a;
},_cancelHideTimer:function(){
if(this._hideTimer){
clearTimeout(this._hideTimer);
this._hideTimer=null;
}
},_setHideTimer:function(_1b){
this._cancelHideTimer();
if(_1b>0){
this._cancelHideTimer();
this._hideTimer=setTimeout(_2.hitch(this,function(evt){
if(this.bgIframe&&this.bgIframe.iframe){
this.bgIframe.iframe.style.display="none";
}
this._hideTimer=null;
this._stickyMessage=false;
this.fadeAnim.play();
}),_1b);
}else{
this._stickyMessage=true;
}
},_placeClip:function(){
var _1c=_f.getBox();
var _1d=_7.getMarginBox(this.containerNode);
var _1e=this.clipNode.style;
_1e.height=_1d.h+"px";
_1e.width=_1d.w+"px";
var pd=this.positionDirection;
if(pd.match(/^t/)){
_1e.top=_1c.t+"px";
}else{
if(pd.match(/^b/)){
_1e.top=(_1c.h-_1d.h-2+_1c.t)+"px";
}
}
if(pd.match(/^[tb]r-/)){
_1e.left=(_1c.w-_1d.w-1-_1c.l)+"px";
}else{
if(pd.match(/^[tb]l-/)){
_1e.left=0+"px";
}
}
_1e.clip="rect(0px, "+_1d.w+"px, "+_1d.h+"px, 0px)";
if(_d("ie")){
if(!this.bgIframe){
this.clipNode.id=_8.getUniqueId("dojox_widget_Toaster_clipNode");
this.bgIframe=new _b(this.clipNode);
}
var _1f=this.bgIframe.iframe;
if(_1f){
_1f.style.display="block";
}
}
},onSelect:function(e){
},show:function(){
_5.set(this.domNode,"display","block");
this._placeClip();
if(!this._scrollConnected){
this._scrollConnected=_3.connect(_f,"onscroll",this,this._placeClip);
}
},hide:function(){
_5.set(this.domNode,"display","none");
if(this._scrollConnected){
_3.disconnect(this._scrollConnected);
this._scrollConnected=false;
}
_5.set(this.containerNode,"opacity",1);
}});
});
 