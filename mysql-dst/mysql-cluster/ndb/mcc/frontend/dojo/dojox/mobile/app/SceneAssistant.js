//>>built
define(["dijit","dojo","dojox"],function(_1,_2,_3){
_2.provide("dojox.mobile.app.SceneAssistant");
_2.experimental("dojox.mobile.app.SceneAssistant");
_2.declare("dojox.mobile.app.SceneAssistant",null,{constructor:function(){
},setup:function(){
},activate:function(_4){
},deactivate:function(){
},destroy:function(){
var _5=_2.query("> [widgetId]",this.containerNode).map(_1.byNode);
_2.forEach(_5,function(_6){
_6.destroyRecursive();
});
this.disconnect();
},connect:function(_7,_8,_9){
if(!this._connects){
this._connects=[];
}
this._connects.push(_2.connect(_7,_8,_9));
},disconnect:function(){
_2.forEach(this._connects,_2.disconnect);
this._connects=[];
}});
});
 