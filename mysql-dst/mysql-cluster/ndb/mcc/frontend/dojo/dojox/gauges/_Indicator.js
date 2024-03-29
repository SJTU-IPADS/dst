//>>built
define("dojox/gauges/_Indicator",["dojo/_base/lang","dojo/_base/declare","dojo/_base/fx","dojo/_base/html","dojo/_base/connect","dijit/_Widget","dojo/dom-construct","dojo/dom-class"],function(_1,_2,fx,_3,_4,_5,_6,_7){
return _2("dojox.gauges._Indicator",[_5],{value:0,type:"",color:"black",strokeColor:"",label:"",font:{family:"sans-serif",size:"12px"},length:0,width:0,offset:0,hover:"",front:false,easing:fx._defaultEasing,duration:1000,hideValue:false,noChange:false,interactionMode:"indicator",_gauge:null,title:"",startup:function(){
if(this.onDragMove){
this.onDragMove=_1.hitch(this.onDragMove);
}
if(this.strokeColor===""){
this.strokeColor=undefined;
}
},postCreate:function(){
if(this.title===""){
_3.style(this.domNode,"display","none");
}
if(_1.isString(this.easing)){
this.easing=_1.getObject(this.easing);
}
},buildRendering:function(){
var n=this.domNode=this.srcNodeRef?this.srcNodeRef:_6.create("div");
_7.add(n,"dojoxGaugeIndicatorDiv");
var _8=_6.create("label");
if(this.title){
_8.innerHTML=this.title+":";
}
_6.place(_8,n);
this.valueNode=_6.create("input",{className:"dojoxGaugeIndicatorInput",size:5,value:this.value});
_6.place(this.valueNode,n);
_4.connect(this.valueNode,"onchange",this,this._update);
},_update:function(){
this._updateValue(true);
},_updateValue:function(_9){
var _a=this.valueNode.value;
if(_a===""){
this.value=null;
}else{
this.value=Number(_a);
this.hover=this.title+": "+_a;
}
if(this._gauge){
this.draw(this._gauge._indicatorsGroup,_9||_9==undefined?false:true);
this.valueNode.value=this.value;
if((this.title=="Target"||this.front)&&this._gauge.moveIndicator){
this._gauge.moveIndicatorToFront(this);
}
this.valueChanged();
}
},valueChanged:function(){
},update:function(_b,_c){
if(!this.noChange){
this.valueNode.value=_b;
this._updateValue(_c);
}
},handleMouseOver:function(e){
this._gauge._handleMouseOverIndicator(this,e);
},handleMouseOut:function(e){
this._gauge._handleMouseOutIndicator(this,e);
this._gauge.gaugeContent.style.cursor="";
},handleMouseDown:function(e){
this._gauge._handleMouseDownIndicator(this,e);
},handleTouchStart:function(e){
this._gauge.handleTouchStartIndicator(this,e);
},onDragMove:function(){
this.value=Math.floor(this.value);
this.valueNode.value=this.value;
this.hover=this.title+": "+this.value;
},draw:function(_d){
},remove:function(){
if(this.shape){
this.shape.parent.remove(this.shape);
}
this.shape=null;
if(this.text){
this.text.parent.remove(this.text);
}
this.text=null;
}});
});
 