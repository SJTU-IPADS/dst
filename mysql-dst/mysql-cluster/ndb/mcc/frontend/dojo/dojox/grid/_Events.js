//>>built
define("dojox/grid/_Events",["dojo/keys","dojo/dom-class","dojo/_base/declare","dojo/_base/event","dojo/_base/sniff"],function(_1,_2,_3,_4,_5){
return _3("dojox.grid._Events",null,{cellOverClass:"dojoxGridCellOver",onKeyEvent:function(e){
this.dispatchKeyEvent(e);
},onContentEvent:function(e){
this.dispatchContentEvent(e);
},onHeaderEvent:function(e){
this.dispatchHeaderEvent(e);
},onStyleRow:function(_6){
var i=_6;
i.customClasses+=(i.odd?" dojoxGridRowOdd":"")+(i.selected?" dojoxGridRowSelected":"")+(i.over?" dojoxGridRowOver":"");
this.focus.styleRow(_6);
this.edit.styleRow(_6);
},onKeyDown:function(e){
if(e.altKey||e.metaKey){
return;
}
var _7;
switch(e.keyCode){
case _1.ESCAPE:
this.edit.cancel();
break;
case _1.ENTER:
if(!this.edit.isEditing()){
_7=this.focus.getHeaderIndex();
if(_7>=0){
this.setSortIndex(_7);
break;
}else{
this.selection.clickSelect(this.focus.rowIndex,dojo.isCopyKey(e),e.shiftKey);
}
_4.stop(e);
}
if(!e.shiftKey){
var _8=this.edit.isEditing();
this.edit.apply();
if(!_8){
this.edit.setEditCell(this.focus.cell,this.focus.rowIndex);
}
}
if(!this.edit.isEditing()){
var _9=this.focus.focusView||this.views.views[0];
_9.content.decorateEvent(e);
this.onRowClick(e);
_4.stop(e);
}
break;
case _1.SPACE:
if(!this.edit.isEditing()){
_7=this.focus.getHeaderIndex();
if(_7>=0){
this.setSortIndex(_7);
break;
}else{
this.selection.clickSelect(this.focus.rowIndex,dojo.isCopyKey(e),e.shiftKey);
}
_4.stop(e);
}
break;
case _1.TAB:
this.focus[e.shiftKey?"previousKey":"nextKey"](e);
break;
case _1.LEFT_ARROW:
case _1.RIGHT_ARROW:
if(!this.edit.isEditing()){
var _a=e.keyCode;
_4.stop(e);
_7=this.focus.getHeaderIndex();
if(_7>=0&&(e.shiftKey&&e.ctrlKey)){
this.focus.colSizeAdjust(e,_7,(_a==_1.LEFT_ARROW?-1:1)*5);
}else{
var _b=(_a==_1.LEFT_ARROW)?1:-1;
if(this.isLeftToRight()){
_b*=-1;
}
this.focus.move(0,_b);
}
}
break;
case _1.UP_ARROW:
if(!this.edit.isEditing()&&this.focus.rowIndex!==0){
_4.stop(e);
this.focus.move(-1,0);
}
break;
case _1.DOWN_ARROW:
if(!this.edit.isEditing()&&this.focus.rowIndex+1!=this.rowCount){
_4.stop(e);
this.focus.move(1,0);
}
break;
case _1.PAGE_UP:
if(!this.edit.isEditing()&&this.focus.rowIndex!==0){
_4.stop(e);
if(this.focus.rowIndex!=this.scroller.firstVisibleRow+1){
this.focus.move(this.scroller.firstVisibleRow-this.focus.rowIndex,0);
}else{
this.setScrollTop(this.scroller.findScrollTop(this.focus.rowIndex-1));
this.focus.move(this.scroller.firstVisibleRow-this.scroller.lastVisibleRow+1,0);
}
}
break;
case _1.PAGE_DOWN:
if(!this.edit.isEditing()&&this.focus.rowIndex+1!=this.rowCount){
_4.stop(e);
if(this.focus.rowIndex!=this.scroller.lastVisibleRow-1){
this.focus.move(this.scroller.lastVisibleRow-this.focus.rowIndex-1,0);
}else{
this.setScrollTop(this.scroller.findScrollTop(this.focus.rowIndex+1));
this.focus.move(this.scroller.lastVisibleRow-this.scroller.firstVisibleRow-1,0);
}
}
break;
default:
break;
}
},onMouseOver:function(e){
e.rowIndex==-1?this.onHeaderCellMouseOver(e):this.onCellMouseOver(e);
},onMouseOut:function(e){
e.rowIndex==-1?this.onHeaderCellMouseOut(e):this.onCellMouseOut(e);
},onMouseDown:function(e){
e.rowIndex==-1?this.onHeaderCellMouseDown(e):this.onCellMouseDown(e);
},onMouseOverRow:function(e){
if(!this.rows.isOver(e.rowIndex)){
this.rows.setOverRow(e.rowIndex);
e.rowIndex==-1?this.onHeaderMouseOver(e):this.onRowMouseOver(e);
}
},onMouseOutRow:function(e){
if(this.rows.isOver(-1)){
this.onHeaderMouseOut(e);
}else{
if(!this.rows.isOver(-2)){
this.rows.setOverRow(-2);
this.onRowMouseOut(e);
}
}
},onMouseDownRow:function(e){
if(e.rowIndex!=-1){
this.onRowMouseDown(e);
}
},onCellMouseOver:function(e){
if(e.cellNode){
_2.add(e.cellNode,this.cellOverClass);
}
},onCellMouseOut:function(e){
if(e.cellNode){
_2.remove(e.cellNode,this.cellOverClass);
}
},onCellMouseDown:function(e){
},onCellClick:function(e){
this._click[0]=this._click[1];
this._click[1]=e;
if(!this.edit.isEditCell(e.rowIndex,e.cellIndex)){
this.focus.setFocusCell(e.cell,e.rowIndex);
}
if(this._click.length>1&&this._click[0]==null){
this._click.shift();
}
this.onRowClick(e);
},onCellDblClick:function(e){
var _c;
if(this._click.length>1&&_5("ie")){
_c=this._click[1];
}else{
if(this._click.length>1&&this._click[0].rowIndex!=this._click[1].rowIndex){
_c=this._click[0];
}else{
_c=e;
}
}
this.focus.setFocusCell(_c.cell,_c.rowIndex);
this.onRowClick(_c);
this.edit.setEditCell(_c.cell,_c.rowIndex);
this.onRowDblClick(e);
},onCellContextMenu:function(e){
this.onRowContextMenu(e);
},onCellFocus:function(_d,_e){
this.edit.cellFocus(_d,_e);
},onRowClick:function(e){
this.edit.rowClick(e);
this.selection.clickSelectEvent(e);
},onRowDblClick:function(e){
},onRowMouseOver:function(e){
},onRowMouseOut:function(e){
},onRowMouseDown:function(e){
},onRowContextMenu:function(e){
_4.stop(e);
},onHeaderMouseOver:function(e){
},onHeaderMouseOut:function(e){
},onHeaderCellMouseOver:function(e){
if(e.cellNode){
_2.add(e.cellNode,this.cellOverClass);
}
},onHeaderCellMouseOut:function(e){
if(e.cellNode){
_2.remove(e.cellNode,this.cellOverClass);
}
},onHeaderCellMouseDown:function(e){
},onHeaderClick:function(e){
},onHeaderCellClick:function(e){
this.setSortIndex(e.cell.index);
this.onHeaderClick(e);
},onHeaderDblClick:function(e){
},onHeaderCellDblClick:function(e){
this.onHeaderDblClick(e);
},onHeaderCellContextMenu:function(e){
this.onHeaderContextMenu(e);
},onHeaderContextMenu:function(e){
if(!this.headerMenu){
_4.stop(e);
}
},onStartEdit:function(_f,_10){
},onApplyCellEdit:function(_11,_12,_13){
},onCancelEdit:function(_14){
},onApplyEdit:function(_15){
},onCanSelect:function(_16){
return true;
},onCanDeselect:function(_17){
return true;
},onSelected:function(_18){
this.updateRowStyles(_18);
},onDeselected:function(_19){
this.updateRowStyles(_19);
},onSelectionChanged:function(){
}});
});
 