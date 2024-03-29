//>>built
define(["dijit","dojo","dojox"],function(_1,_2,_3){
_2.provide("dojox.drawing.tools.Pencil");
_3.drawing.tools.Pencil=_3.drawing.util.oo.declare(_3.drawing.stencil.Path,function(){
this._started=false;
},{draws:true,minDist:15,onDown:function(_4){
this._started=true;
var p={x:_4.x,y:_4.y};
this.points=[p];
this.lastPoint=p;
this.revertRenderHit=this.renderHit;
this.renderHit=false;
this.closePath=false;
},onDrag:function(_5){
if(!this._started||this.minDist>this.util.distance(_5.x,_5.y,this.lastPoint.x,this.lastPoint.y)){
return;
}
var p={x:_5.x,y:_5.y};
this.points.push(p);
this.render();
this.checkClosePoint(this.points[0],_5);
this.lastPoint=p;
},onUp:function(_6){
if(!this._started){
return;
}
if(!this.points||this.points.length<2){
this._started=false;
this.points=[];
return;
}
var _7=this.getBounds();
if(_7.w<this.minimumSize&&_7.h<this.minimumSize){
this.remove(this.hit,this.shape,this.closeGuide);
this._started=false;
this.setPoints([]);
return;
}
if(this.checkClosePoint(this.points[0],_6,true)){
this.closePath=true;
}
this.renderHit=this.revertRenderHit;
this.renderedOnce=true;
this.render();
this.onRender(this);
}});
_3.drawing.tools.Pencil.setup={name:"dojox.drawing.tools.Pencil",tooltip:"Pencil Tool",iconClass:"iconLine"};
_3.drawing.register(_3.drawing.tools.Pencil.setup,"tool");
});
 