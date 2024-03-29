//>>built
define("dojox/gfx/_base",["dojo/_base/lang","dojo/_base/html","dojo/_base/Color","dojo/_base/sniff","dojo/_base/window","dojo/_base/array","dojo/dom","dojo/dom-construct","dojo/dom-geometry"],function(_1,_2,_3,_4,_5,_6,_7,_8,_9){
var g=_1.getObject("dojox.gfx",true),b=g._base={};
g._hasClass=function(_a,_b){
var _c=_a.getAttribute("className");
return _c&&(" "+_c+" ").indexOf(" "+_b+" ")>=0;
};
g._addClass=function(_d,_e){
var _f=_d.getAttribute("className")||"";
if(!_f||(" "+_f+" ").indexOf(" "+_e+" ")<0){
_d.setAttribute("className",_f+(_f?" ":"")+_e);
}
};
g._removeClass=function(_10,_11){
var cls=_10.getAttribute("className");
if(cls){
_10.setAttribute("className",cls.replace(new RegExp("(^|\\s+)"+_11+"(\\s+|$)"),"$1$2"));
}
};
b._getFontMeasurements=function(){
var _12={"1em":0,"1ex":0,"100%":0,"12pt":0,"16px":0,"xx-small":0,"x-small":0,"small":0,"medium":0,"large":0,"x-large":0,"xx-large":0};
var p;
if(_4("ie")){
_5.doc.documentElement.style.fontSize="100%";
}
var div=_8.create("div",{style:{position:"absolute",left:"0",top:"-100px",width:"30px",height:"1000em",borderWidth:"0",margin:"0",padding:"0",outline:"none",lineHeight:"1",overflow:"hidden"}},_5.body());
for(p in _12){
div.style.fontSize=p;
_12[p]=Math.round(div.offsetHeight*12/16)*16/12/1000;
}
_5.body().removeChild(div);
return _12;
};
var _13=null;
b._getCachedFontMeasurements=function(_14){
if(_14||!_13){
_13=b._getFontMeasurements();
}
return _13;
};
var _15=null,_16={};
b._getTextBox=function(_17,_18,_19){
var m,s,al=arguments.length;
var i;
if(!_15){
_15=_8.create("div",{style:{position:"absolute",top:"-10000px",left:"0"}},_5.body());
}
m=_15;
m.className="";
s=m.style;
s.borderWidth="0";
s.margin="0";
s.padding="0";
s.outline="0";
if(al>1&&_18){
for(i in _18){
if(i in _16){
continue;
}
s[i]=_18[i];
}
}
if(al>2&&_19){
m.className=_19;
}
m.innerHTML=_17;
if(m["getBoundingClientRect"]){
var bcr=m.getBoundingClientRect();
return {l:bcr.left,t:bcr.top,w:bcr.width||(bcr.right-bcr.left),h:bcr.height||(bcr.bottom-bcr.top)};
}else{
return _9.getMarginBox(m);
}
};
var _1a=0;
b._getUniqueId=function(){
var id;
do{
id=dojo._scopeName+"xUnique"+(++_1a);
}while(_7.byId(id));
return id;
};
_1.mixin(g,{defaultPath:{type:"path",path:""},defaultPolyline:{type:"polyline",points:[]},defaultRect:{type:"rect",x:0,y:0,width:100,height:100,r:0},defaultEllipse:{type:"ellipse",cx:0,cy:0,rx:200,ry:100},defaultCircle:{type:"circle",cx:0,cy:0,r:100},defaultLine:{type:"line",x1:0,y1:0,x2:100,y2:100},defaultImage:{type:"image",x:0,y:0,width:0,height:0,src:""},defaultText:{type:"text",x:0,y:0,text:"",align:"start",decoration:"none",rotated:false,kerning:true},defaultTextPath:{type:"textpath",text:"",align:"start",decoration:"none",rotated:false,kerning:true},defaultStroke:{type:"stroke",color:"black",style:"solid",width:1,cap:"butt",join:4},defaultLinearGradient:{type:"linear",x1:0,y1:0,x2:100,y2:100,colors:[{offset:0,color:"black"},{offset:1,color:"white"}]},defaultRadialGradient:{type:"radial",cx:0,cy:0,r:100,colors:[{offset:0,color:"black"},{offset:1,color:"white"}]},defaultPattern:{type:"pattern",x:0,y:0,width:0,height:0,src:""},defaultFont:{type:"font",style:"normal",variant:"normal",weight:"normal",size:"10pt",family:"serif"},getDefault:(function(){
var _1b={};
return function(_1c){
var t=_1b[_1c];
if(t){
return new t();
}
t=_1b[_1c]=new Function();
t.prototype=g["default"+_1c];
return new t();
};
})(),normalizeColor:function(_1d){
return (_1d instanceof _3)?_1d:new _3(_1d);
},normalizeParameters:function(_1e,_1f){
var x;
if(_1f){
var _20={};
for(x in _1e){
if(x in _1f&&!(x in _20)){
_1e[x]=_1f[x];
}
}
}
return _1e;
},makeParameters:function(_21,_22){
var i=null;
if(!_22){
return _1.delegate(_21);
}
var _23={};
for(i in _21){
if(!(i in _23)){
_23[i]=_1.clone((i in _22)?_22[i]:_21[i]);
}
}
return _23;
},formatNumber:function(x,_24){
var val=x.toString();
if(val.indexOf("e")>=0){
val=x.toFixed(4);
}else{
var _25=val.indexOf(".");
if(_25>=0&&val.length-_25>5){
val=x.toFixed(4);
}
}
if(x<0){
return val;
}
return _24?" "+val:val;
},makeFontString:function(_26){
return _26.style+" "+_26.variant+" "+_26.weight+" "+_26.size+" "+_26.family;
},splitFontString:function(str){
var _27=g.getDefault("Font");
var t=str.split(/\s+/);
do{
if(t.length<5){
break;
}
_27.style=t[0];
_27.variant=t[1];
_27.weight=t[2];
var i=t[3].indexOf("/");
_27.size=i<0?t[3]:t[3].substring(0,i);
var j=4;
if(i<0){
if(t[4]=="/"){
j=6;
}else{
if(t[4].charAt(0)=="/"){
j=5;
}
}
}
if(j<t.length){
_27.family=t.slice(j).join(" ");
}
}while(false);
return _27;
},cm_in_pt:72/2.54,mm_in_pt:7.2/2.54,px_in_pt:function(){
return g._base._getCachedFontMeasurements()["12pt"]/12;
},pt2px:function(len){
return len*g.px_in_pt();
},px2pt:function(len){
return len/g.px_in_pt();
},normalizedLength:function(len){
if(len.length===0){
return 0;
}
if(len.length>2){
var _28=g.px_in_pt();
var val=parseFloat(len);
switch(len.slice(-2)){
case "px":
return val;
case "pt":
return val*_28;
case "in":
return val*72*_28;
case "pc":
return val*12*_28;
case "mm":
return val*g.mm_in_pt*_28;
case "cm":
return val*g.cm_in_pt*_28;
}
}
return parseFloat(len);
},pathVmlRegExp:/([A-Za-z]+)|(\d+(\.\d+)?)|(\.\d+)|(-\d+(\.\d+)?)|(-\.\d+)/g,pathSvgRegExp:/([A-Za-z])|(\d+(\.\d+)?)|(\.\d+)|(-\d+(\.\d+)?)|(-\.\d+)/g,equalSources:function(a,b){
return a&&b&&a===b;
},switchTo:function(_29){
var ns=typeof _29=="string"?g[_29]:_29;
if(ns){
_6.forEach(["Group","Rect","Ellipse","Circle","Line","Polyline","Image","Text","Path","TextPath","Surface","createSurface","fixTarget"],function(_2a){
g[_2a]=ns[_2a];
});
}
}});
return g;
});
 