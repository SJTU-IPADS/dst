//>>built
define("dojox/gauges/_Gauge",["dojo/_base/declare","dojo/_base/lang","dojo/_base/html","dojo/_base/array","dojo/_base/event","dojo/_base/connect","dojo/dom-construct","dijit/_Widget","dojox/gfx","./Range","dojo/fx/easing"],function(_1,_2,_3,_4,_5,_6,_7,_8,_9,_a){
var _b=0;
var _c=0;
return _1("dojox.gauges._Gauge",[_8],{width:0,height:0,background:null,image:null,useRangeStyles:0,useTooltip:true,majorTicks:null,minorTicks:null,_defaultIndicator:null,defaultColors:[[0,84,170,1],[68,119,187,1],[102,153,204,1],[153,187,238,1],[153,204,255,1],[204,238,255,1],[221,238,255,1]],min:null,max:null,surface:null,hideValues:false,gaugeContent:undefined,_backgroundDefault:{color:"#E0E0E0"},_rangeData:null,_indicatorData:null,_drag:null,_img:null,_overOverlay:false,_lastHover:"",startup:function(){
if(this.image===null){
this.image={};
}
this.connect(this.gaugeContent,"onmousedown",this.handleMouseDown);
this.connect(this.gaugeContent,"onmousemove",this.handleMouseMove);
this.connect(this.gaugeContent,"onmouseover",this.handleMouseOver);
this.connect(this.gaugeContent,"onmouseout",this.handleMouseOut);
this.connect(this.gaugeContent,"touchstart",this.handleTouchStart);
this.connect(this.gaugeContent,"touchend",this.handleTouchEnd);
this.connect(this.gaugeContent,"touchmove",this.handleTouchMove);
if(!_2.isArray(this.ranges)){
this.ranges=[];
}
if(!_2.isArray(this.indicators)){
this.indicators=[];
}
var _d=[],_e=[];
var i;
if(this.hasChildren()){
var _f=this.getChildren();
for(i=0;i<_f.length;i++){
if(/.*Indicator/.test(_f[i].declaredClass)){
_e.push(_f[i]);
continue;
}
switch(_f[i].declaredClass){
case _a.prototype.declaredClass:
_d.push(_f[i]);
break;
}
}
this.ranges=this.ranges.concat(_d);
this.indicators=this.indicators.concat(_e);
}
if(!this.background){
this.background=this._backgroundDefault;
}
this.background=this.background.color||this.background;
if(!this.surface){
this.createSurface();
}
this.addRanges(this.ranges);
if(this.minorTicks&&this.minorTicks.interval){
this.setMinorTicks(this.minorTicks);
}
if(this.majorTicks&&this.majorTicks.interval){
this.setMajorTicks(this.majorTicks);
}
for(i=0;i<this.indicators.length;i++){
this.addIndicator(this.indicators[i]);
}
this.inherited(arguments);
},hasChildren:function(){
return this.getChildren().length>0;
},buildRendering:function(){
var n=this.domNode=this.srcNodeRef?this.srcNodeRef:_7.create("div");
this.gaugeContent=_7.create("div",{className:"dojoxGaugeContent"});
this.containerNode=_7.create("div");
this.mouseNode=_7.create("div");
while(n.hasChildNodes()){
this.containerNode.appendChild(n.firstChild);
}
_7.place(this.gaugeContent,n);
_7.place(this.containerNode,n);
_7.place(this.mouseNode,n);
},_setTicks:function(_10,_11,_12){
var i;
if(_10&&_2.isArray(_10._ticks)){
for(i=0;i<_10._ticks.length;i++){
this._removeScaleTick(_10._ticks[i]);
}
}
var t={length:_11.length,offset:_11.offset,noChange:true};
if(_11.color){
t.color=_11.color;
}
if(_11.font){
t.font=_11.font;
}
if(_11.labelPlacement){
t.direction=_11.labelPlacement;
}
_11._ticks=[];
for(i=this.min;i<=this.max;i+=_11.interval){
if(i==this.max&&this._isScaleCircular()){
continue;
}
t.value=i;
if(_12){
var _13=this._getNumberModule();
if(_13){
t.label=(_11.fixedPrecision&&_11.precision)?_13.format(i,{places:_11.precision}):_13.format(i);
}else{
t.label=(_11.fixedPrecision&&_11.precision)?i.toFixed(_11.precision):i.toString();
}
}
_11._ticks.push(this._addScaleTick(t,_12));
}
return _11;
},_isScaleCircular:function(){
return false;
},setMinorTicks:function(_14){
this.minorTicks=this._setTicks(this.minorTicks,_14,false);
},setMajorTicks:function(_15){
this.majorTicks=this._setTicks(this.majorTicks,_15,true);
},postCreate:function(){
if(this.hideValues){
_3.style(this.containerNode,"display","none");
}
_3.style(this.mouseNode,"width","0");
_3.style(this.mouseNode,"height","0");
_3.style(this.mouseNode,"position","absolute");
_3.style(this.mouseNode,"z-index","100");
if(this.useTooltip){
require(["dijit/Tooltip"],dojo.hitch(this,function(_16){
_16.show("test",this.mouseNode,!this.isLeftToRight());
_16.hide(this.mouseNode);
}));
}
},_getNumberModule:function(){
if(_c==0){
try{
_c=require("dojo/number");
}
catch(e){
_c=null;
}
}
return _c;
},createSurface:function(){
this.gaugeContent.style.width=this.width+"px";
this.gaugeContent.style.height=this.height+"px";
this.surface=_9.createSurface(this.gaugeContent,this.width,this.height);
this._backgroundGroup=this.surface.createGroup();
this._rangeGroup=this.surface.createGroup();
this._minorTicksGroup=this.surface.createGroup();
this._majorTicksGroup=this.surface.createGroup();
this._overlayGroup=this.surface.createGroup();
this._indicatorsGroup=this.surface.createGroup();
this._foregroundGroup=this.surface.createGroup();
this._background=this._backgroundGroup.createRect({x:0,y:0,width:this.width,height:this.height});
this._background.setFill(this.background);
if(this.image.url){
var _17=this._backgroundGroup;
if(this.image.overlay){
_17=this._overlayGroup;
}
this._img=_17.createImage({width:this.image.width||this.width,height:this.image.height||this.height,src:this.image.url});
if(this.image.x||this.image.y){
this._img.setTransform({dx:this.image.x||0,dy:this.image.y||0});
}
}
},draw:function(){
var i;
if(!this.surface){
return;
}
this.drawBackground(this._backgroundGroup);
if(this._rangeData){
for(i=0;i<this._rangeData.length;i++){
this.drawRange(this._rangeGroup,this._rangeData[i]);
}
}
if(this._minorTicksData){
for(i=0;i<this._minorTicksData.length;i++){
this._minorTicksData[i].draw(this._minorTicksGroup);
}
}
if(this._majorTicksData){
for(i=0;i<this._majorTicksData.length;i++){
this._majorTicksData[i].draw(this._majorTicksGroup);
}
}
if(this._indicatorData){
for(i=0;i<this._indicatorData.length;i++){
this._indicatorData[i].draw(this._indicatorsGroup);
}
}
this.drawForeground(this._foregroundGroup);
},drawBackground:function(_18){
},drawForeground:function(_19){
},setBackground:function(_1a){
if(!_1a){
_1a=this._backgroundDefault;
}
this.background=_1a.color||_1a;
this._background.setFill(this.background);
},addRange:function(_1b){
this.addRanges([_1b]);
},addRanges:function(_1c){
if(!this._rangeData){
this._rangeData=[];
}
var _1d;
for(var i=0;i<_1c.length;i++){
_1d=_1c[i];
if((this.min===null)||(_1d.low<this.min)){
this.min=_1d.low;
}
if((this.max===null)||(_1d.high>this.max)){
this.max=_1d.high;
}
if(!_1d.color){
var _1e=this._rangeData.length%this.defaultColors.length;
if(_9.svg&&this.useRangeStyles>0){
_1e=(this._rangeData.length%this.useRangeStyles)+1;
_1d.color={style:"dojoxGaugeRange"+_1e};
}else{
_1e=this._rangeData.length%this.defaultColors.length;
_1d.color=this.defaultColors[_1e];
}
}
this._rangeData[this._rangeData.length]=_1d;
}
this.draw();
},_addScaleTick:function(_1f,_20){
if(!_1f.declaredClass){
_1f=new this._defaultIndicator(_1f);
}
_1f._gauge=this;
if(_20){
if(!this._majorTicksData){
this._majorTicksData=[];
}
this._majorTicksData[this._majorTicksData.length]=_1f;
_1f.draw(this._majorTicksGroup);
}else{
if(!this._minorTicksData){
this._minorTicksData=[];
}
this._minorTicksData[this._minorTicksData.length]=_1f;
_1f.draw(this._minorTicksGroup);
}
return _1f;
},_removeScaleTick:function(_21){
var i;
if(this._majorTicksData){
for(i=0;i<this._majorTicksData.length;i++){
if(this._majorTicksData[i]===_21){
this._majorTicksData.splice(i,1);
_21.remove();
return;
}
}
}
if(this._minorTicksData){
for(i=0;i<this._minorTicksData.length;i++){
if(this._minorTicksData[i]===_21){
this._minorTicksData.splice(i,1);
_21.remove();
return;
}
}
}
},addIndicator:function(_22){
if(!_22.declaredClass){
_22=new this._defaultIndicator(_22);
}
_22._gauge=this;
if(!_22.hideValue){
this.containerNode.appendChild(_22.domNode);
}
if(!this._indicatorData){
this._indicatorData=[];
}
this._indicatorData[this._indicatorData.length]=_22;
_22.draw(this._indicatorsGroup);
return _22;
},removeIndicator:function(_23){
for(var i=0;i<this._indicatorData.length;i++){
if(this._indicatorData[i]===_23){
this._indicatorData.splice(i,1);
_23.remove();
break;
}
}
},moveIndicatorToFront:function(_24){
if(_24.shape){
_24.shape.moveToFront();
}
},drawText:function(_25,txt,x,y,_26,_27,_28){
var t=_25.createText({x:x,y:y,text:txt,align:_26});
t.setFill(_27?_27:"black");
if(_28){
t.setFont(_28);
}
return t;
},removeText:function(t){
if(t.parent){
t.parent.remove(t);
}
},updateTooltip:function(txt,e){
if(this.useTooltip){
require(["dijit/Tooltip"],dojo.hitch(this,function(_29){
if(this._lastHover!=txt){
if(txt!==""){
_29.hide(this.mouseNode);
_29.show(txt,this.mouseNode,!this.isLeftToRight());
}else{
_29.hide(this.mouseNode);
}
this._lastHover=txt;
}
}));
}
},handleMouseOver:function(e){
if(this.image&&this.image.overlay){
if(e.target==this._img.getEventSource()){
var _2a;
this._overOverlay=true;
var r=this.getRangeUnderMouse(e);
if(r&&r.hover){
_2a=r.hover;
}
if(this.useTooltip&&!this._drag){
if(_2a){
this.updateTooltip(_2a,e);
}else{
this.updateTooltip("",e);
}
}
}
}
},handleMouseOut:function(e){
this._overOverlay=false;
this._hideTooltip();
},handleMouseMove:function(e){
if(this.useTooltip){
if(e){
_3.style(this.mouseNode,"left",e.pageX+1+"px");
_3.style(this.mouseNode,"top",e.pageY+1+"px");
}
if(this._overOverlay){
var r=this.getRangeUnderMouse(e);
if(r&&r.hover){
this.updateTooltip(r.hover,e);
}else{
this.updateTooltip("",e);
}
}
}
},handleMouseDown:function(e){
var _2b=this._getInteractiveIndicator();
if(_2b){
this._handleMouseDownIndicator(_2b,e);
}
},_handleDragInteractionMouseMove:function(e){
if(this._drag){
this._dragIndicator(this,e);
_5.stop(e);
}
},_handleDragInteractionMouseUp:function(e){
this._drag=null;
for(var i=0;i<this._mouseListeners.length;i++){
_6.disconnect(this._mouseListeners[i]);
}
this._mouseListeners=[];
_5.stop(e);
},_handleMouseDownIndicator:function(_2c,e){
if(!_2c.noChange){
if(!this._mouseListeners){
this._mouseListeners=[];
}
this._drag=_2c;
this._mouseListeners.push(_6.connect(document,"onmouseup",this,this._handleDragInteractionMouseUp));
this._mouseListeners.push(_6.connect(document,"onmousemove",this,this._handleDragInteractionMouseMove));
this._mouseListeners.push(_6.connect(document,"ondragstart",this,_5.stop));
this._mouseListeners.push(_6.connect(document,"onselectstart",this,_5.stop));
this._dragIndicator(this,e);
_5.stop(e);
}
},_handleMouseOverIndicator:function(_2d,e){
if(this.useTooltip&&!this._drag){
if(_2d.hover){
require(["dijit/Tooltip"],dojo.hitch(this,function(_2e){
_3.style(this.mouseNode,"left",e.pageX+1+"px");
_3.style(this.mouseNode,"top",e.pageY+1+"px");
_2e.show(_2d.hover,this.mouseNode,!this.isLeftToRight());
}));
}else{
this.updateTooltip("",e);
}
}
if(_2d.onDragMove&&!_2d.noChange){
this.gaugeContent.style.cursor="pointer";
}
},_handleMouseOutIndicator:function(_2f,e){
this._hideTooltip();
this.gaugeContent.style.cursor="pointer";
},_hideTooltip:function(){
if(this.useTooltip&&this.mouseNode){
require(["dijit/Tooltip"],dojo.hitch(this,function(_30){
_30.hide(this.mouseNode);
}));
}
},_handleMouseOutRange:function(_31,e){
this._hideTooltip();
},_handleMouseOverRange:function(_32,e){
if(this.useTooltip&&!this._drag){
if(_32.hover){
_3.style(this.mouseNode,"left",e.pageX+1+"px");
_3.style(this.mouseNode,"top",e.pageY+1+"px");
require(["dijit/Tooltip"],dojo.hitch(this,function(_33){
_33.show(_32.hover,this.mouseNode,!this.isLeftToRight());
}));
}else{
this.updateTooltip("",e);
}
}
},handleTouchStartIndicator:function(_34,e){
if(!_34.noChange){
this._drag=_34;
_5.stop(e);
}
},handleTouchStart:function(e){
this._drag=this._getInteractiveIndicator();
this.handleTouchMove(e);
},handleTouchEnd:function(e){
if(this._drag){
this._drag=null;
_5.stop(e);
}
},handleTouchMove:function(e){
if(this._drag&&!this._drag.noChange){
var _35=e.touches;
var _36=_35[0];
this._dragIndicatorAt(this,_36.pageX,_36.pageY);
_5.stop(e);
}
},_getInteractiveIndicator:function(){
for(var i=0;i<this._indicatorData.length;i++){
var _37=this._indicatorData[i];
if(_37.interactionMode=="gauge"&&!_37.noChange){
return _37;
}
}
return null;
}});
});
 