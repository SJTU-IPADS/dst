//>>built
define(["dijit","dojo","dojox","dojo/require!dojo/dnd/Avatar,dojo/dnd/common"],function(_1,_2,_3){
_2.provide("dojox.layout.dnd.Avatar");
_2.require("dojo.dnd.Avatar");
_2.require("dojo.dnd.common");
_2.declare("dojox.layout.dnd.Avatar",_2.dnd.Avatar,{constructor:function(_4,_5){
this.opacity=_5||0.9;
},construct:function(){
var _6=this.manager.source,_7=_6.creator?_6._normalizedCreator(_6.getItem(this.manager.nodes[0].id).data,"avatar").node:this.manager.nodes[0].cloneNode(true);
_2.addClass(_7,"dojoDndAvatar");
_7.id=_2.dnd.getUniqueId();
_7.style.position="absolute";
_7.style.zIndex=1999;
_7.style.margin="0px";
_7.style.width=_2.marginBox(_6.node).w+"px";
_2.style(_7,"opacity",this.opacity);
this.node=_7;
},update:function(){
_2.toggleClass(this.node,"dojoDndAvatarCanDrop",this.manager.canDropFlag);
},_generateText:function(){
}});
});
 