//>>built
define("dojox/mobile/RoundRectList",["dojo/_base/array","dojo/_base/declare","dojo/_base/window","dijit/_Contained","dijit/_Container","dijit/_WidgetBase"],function(_1,_2,_3,_4,_5,_6){
return _2("dojox.mobile.RoundRectList",[_6,_5,_4],{transition:"slide",iconBase:"",iconPos:"",select:"",stateful:false,buildRendering:function(){
this.domNode=this.containerNode=this.srcNodeRef||_3.doc.createElement("UL");
this.domNode.className="mblRoundRectList";
},resize:function(){
_1.forEach(this.getChildren(),function(_7){
if(_7.resize){
_7.resize();
}
});
},onCheckStateChanged:function(_8,_9){
},_setStatefulAttr:function(_a){
this.stateful=_a;
_1.forEach(this.getChildren(),function(_b){
_b.setArrow&&_b.setArrow();
});
},deselectItem:function(_c){
_c.deselect();
},deselectAll:function(){
_1.forEach(this.getChildren(),function(_d){
_d.deselect&&_d.deselect();
});
},selectItem:function(_e){
_e.select();
}});
});
 