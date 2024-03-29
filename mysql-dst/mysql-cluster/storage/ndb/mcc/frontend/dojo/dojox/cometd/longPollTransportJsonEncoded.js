//>>built
define(["dijit","dojo","dojox","dojo/require!dojox/cometd/_base"],function(_1,_2,_3){
_2.provide("dojox.cometd.longPollTransportJsonEncoded");
_2.require("dojox.cometd._base");
_3.cometd.longPollTransportJsonEncoded=new function(){
this._connectionType="long-polling";
this._cometd=null;
this.check=function(_4,_5,_6){
return ((!_6)&&(_2.indexOf(_4,"long-polling")>=0));
};
this.tunnelInit=function(){
var _7={channel:"/meta/connect",clientId:this._cometd.clientId,connectionType:this._connectionType,id:""+this._cometd.messageId++};
_7=this._cometd._extendOut(_7);
this.openTunnelWith([_7]);
};
this.tunnelCollapse=function(){
if(!this._cometd._initialized){
return;
}
if(this._cometd._advice&&this._cometd._advice["reconnect"]=="none"){
return;
}
if(this._cometd._status=="connected"){
setTimeout(_2.hitch(this,function(){
this._connect();
}),this._cometd._interval());
}else{
setTimeout(_2.hitch(this._cometd,function(){
this.init(this.url,this._props);
}),this._cometd._interval());
}
};
this._connect=function(){
if(!this._cometd._initialized){
return;
}
if(this._cometd._polling){
return;
}
if((this._cometd._advice)&&(this._cometd._advice["reconnect"]=="handshake")){
this._cometd._status="unconnected";
this._initialized=false;
this._cometd.init(this._cometd.url,this._cometd._props);
}else{
if(this._cometd._status=="connected"){
var _8={channel:"/meta/connect",connectionType:this._connectionType,clientId:this._cometd.clientId,id:""+this._cometd.messageId++};
if(this._cometd.connectTimeout>=this._cometd.expectedNetworkDelay){
_8.advice={timeout:(this._cometd.connectTimeout-this._cometd.expectedNetworkDelay)};
}
_8=this._cometd._extendOut(_8);
this.openTunnelWith([_8]);
}
}
};
this.deliver=function(_9){
};
this.openTunnelWith=function(_a,_b){
this._cometd._polling=true;
var _c={url:(_b||this._cometd.url),postData:_2.toJson(_a),contentType:"text/json;charset=UTF-8",handleAs:this._cometd.handleAs,load:_2.hitch(this,function(_d){
this._cometd._polling=false;
this._cometd.deliver(_d);
this._cometd._backon();
this.tunnelCollapse();
}),error:_2.hitch(this,function(_e){
this._cometd._polling=false;
var _f={failure:true,error:_e,advice:this._cometd._advice};
this._cometd._publishMeta("connect",false,_f);
this._cometd._backoff();
this.tunnelCollapse();
})};
var _10=this._cometd._connectTimeout();
if(_10>0){
_c.timeout=_10;
}
this._poll=_2.rawXhrPost(_c);
};
this.sendMessages=function(_11){
for(var i=0;i<_11.length;i++){
_11[i].clientId=this._cometd.clientId;
_11[i].id=""+this._cometd.messageId++;
_11[i]=this._cometd._extendOut(_11[i]);
}
return _2.rawXhrPost({url:this._cometd.url||_2.config["cometdRoot"],handleAs:this._cometd.handleAs,load:_2.hitch(this._cometd,"deliver"),postData:_2.toJson(_11),contentType:"text/json;charset=UTF-8",error:_2.hitch(this,function(err){
this._cometd._publishMeta("publish",false,{messages:_11});
}),timeout:this._cometd.expectedNetworkDelay});
};
this.startup=function(_12){
if(this._cometd._status=="connected"){
return;
}
this.tunnelInit();
};
this.disconnect=function(){
var _13={channel:"/meta/disconnect",clientId:this._cometd.clientId,id:""+this._cometd.messageId++};
_13=this._cometd._extendOut(_13);
_2.rawXhrPost({url:this._cometd.url||_2.config["cometdRoot"],handleAs:this._cometd.handleAs,postData:_2.toJson([_13]),contentType:"text/json;charset=UTF-8"});
};
this.cancelConnect=function(){
if(this._poll){
this._poll.cancel();
this._cometd._polling=false;
this._cometd._publishMeta("connect",false,{cancel:true});
this._cometd._backoff();
this.disconnect();
this.tunnelCollapse();
}
};
};
_3.cometd.longPollTransport=_3.cometd.longPollTransportJsonEncoded;
_3.cometd.connectionTypes.register("long-polling",_3.cometd.longPollTransport.check,_3.cometd.longPollTransportJsonEncoded);
_3.cometd.connectionTypes.register("long-polling-json-encoded",_3.cometd.longPollTransport.check,_3.cometd.longPollTransportJsonEncoded);
});
 