//>>built
define(["dijit","dojo","dojox"],function(_1,_2,_3){
_2.provide("dojox.drawing.manager.keys");
(function(){
var _4=false;
var _5=true;
var _6="abcdefghijklmnopqrstuvwxyz";
_3.drawing.manager.keys={arrowIncrement:1,arrowShiftIncrement:10,shift:false,ctrl:false,alt:false,cmmd:false,meta:false,onDelete:function(_7){
},onEsc:function(_8){
},onEnter:function(_9){
},onArrow:function(_a){
},onKeyDown:function(_b){
},onKeyUp:function(_c){
},listeners:[],register:function(_d){
var _e=_3.drawing.util.common.uid("listener");
this.listeners.push({handle:_e,scope:_d.scope||window,callback:_d.callback,keyCode:_d.keyCode});
},_getLetter:function(_f){
if(!_f.meta&&_f.keyCode>=65&&_f.keyCode<=90){
return _6.charAt(_f.keyCode-65);
}
return null;
},_mixin:function(evt){
evt.meta=this.meta;
evt.shift=this.shift;
evt.alt=this.alt;
evt.cmmd=this.cmmd;
evt.letter=this._getLetter(evt);
return evt;
},editMode:function(_10){
_4=_10;
},enable:function(_11){
_5=_11;
},scanForFields:function(){
if(this._fieldCons){
_2.forEach(this._fieldCons,_2.disconnect,_2);
}
this._fieldCons=[];
_2.query("input").forEach(function(n){
var a=_2.connect(n,"focus",this,function(evt){
this.enable(false);
});
var b=_2.connect(n,"blur",this,function(evt){
this.enable(true);
});
this._fieldCons.push(a);
this._fieldCons.push(b);
},this);
},init:function(){
setTimeout(_2.hitch(this,"scanForFields"),500);
_2.connect(document,"blur",this,function(evt){
this.meta=this.shift=this.ctrl=this.cmmd=this.alt=false;
});
_2.connect(document,"keydown",this,function(evt){
if(!_5){
return;
}
if(evt.keyCode==16){
this.shift=true;
}
if(evt.keyCode==17){
this.ctrl=true;
}
if(evt.keyCode==18){
this.alt=true;
}
if(evt.keyCode==224){
this.cmmd=true;
}
this.meta=this.shift||this.ctrl||this.cmmd||this.alt;
if(!_4){
this.onKeyDown(this._mixin(evt));
if(evt.keyCode==8||evt.keyCode==46){
_2.stopEvent(evt);
}
}
});
_2.connect(document,"keyup",this,function(evt){
if(!_5){
return;
}
var _12=false;
if(evt.keyCode==16){
this.shift=false;
}
if(evt.keyCode==17){
this.ctrl=false;
}
if(evt.keyCode==18){
this.alt=false;
}
if(evt.keyCode==224){
this.cmmd=false;
}
this.meta=this.shift||this.ctrl||this.cmmd||this.alt;
!_4&&this.onKeyUp(this._mixin(evt));
if(evt.keyCode==13){
console.warn("KEY ENTER");
this.onEnter(evt);
_12=true;
}
if(evt.keyCode==27){
this.onEsc(evt);
_12=true;
}
if(evt.keyCode==8||evt.keyCode==46){
this.onDelete(evt);
_12=true;
}
if(_12&&!_4){
_2.stopEvent(evt);
}
});
_2.connect(document,"keypress",this,function(evt){
if(!_5){
return;
}
var inc=this.shift?this.arrowIncrement*this.arrowShiftIncrement:this.arrowIncrement;
var x=0,y=0;
if(evt.keyCode==32&&!_4){
_2.stopEvent(evt);
}
if(evt.keyCode==37){
x=-inc;
}
if(evt.keyCode==38){
y=-inc;
}
if(evt.keyCode==39){
x=inc;
}
if(evt.keyCode==40){
y=inc;
}
if(x||y){
evt.x=x;
evt.y=y;
evt.shift=this.shift;
if(!_4){
this.onArrow(evt);
_2.stopEvent(evt);
}
}
});
}};
_2.addOnLoad(_3.drawing.manager.keys,"init");
})();
});
 