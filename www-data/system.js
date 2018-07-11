// V10.07.2018
// wlanprinter

var befehllist=[{b:'LEDON',n:"LED On"},{b:'LEDOFF',n:"LED Off"}];


var getpostData =function(url, auswertfunc,POSTdata,noheader,rh){
		var loader,i;
		try {loader=new XMLHttpRequest();}
		catch(e){
				try{loader=new ActiveXObject("Microsoft.XMLHTTP");}
				catch(e){
					try{loader=new ActiveXObject("Msxml2.XMLHTTP");}
					catch(e){loader=null;}
				}
			}	
		if(!loader)alert('XMLHttp nicht möglich.');
		var jdata=undefined;
		if(POSTdata!=undefined)jdata=POSTdata;//encodeURI
		
		loader.onreadystatechange=function(){
			if(loader.readyState==4)auswertfunc(loader);
			};
		loader.ontimeout=function(e){console.log("TIMEOUT");}
		loader.onerror=function(e){console.log("ERR",e,loader.readyState);}
		
		if(jdata!=undefined){
				loader.open("POST",url,true);
				if(rh!=undefined){
						for(i=0;i<rh.length;i++){
							loader.setRequestHeader(rh[i].typ,rh[i].val);
						}
				}
				if(noheader!==true){
					//loader.responseType="text";
					loader.setRequestHeader("Content-Type","application/x-www-form-urlencoded");
					//loader.setRequestHeader("Content-Type","text/plain");
					loader.setRequestHeader('Cache-Control','no-cache');
					loader.setRequestHeader("Pragma","no-cache");
					loader.setRequestHeader("Cache-Control","no-cache");
					jdata=encodeURI(POSTdata);
				}
				loader.send(jdata);
			}
			else{
				loader.open('GET',url,true);
				loader.setRequestHeader('Content-Type', 'text/plain');
				loader.send(null);
			}
	}
var cE=function(ziel,e,id,cn){
	var newNode=document.createElement(e);
	if(id!=undefined)newNode.id=id;
	if(cn!=undefined)newNode.className=cn;
	if(ziel)ziel.appendChild(newNode);
	return newNode;
	}
var gE=function(id){return document.getElementById(id);}
var addClass=function(htmlNode,Classe){	
	var newClass;
	if(htmlNode!=undefined){
		newClass=htmlNode.className;
		if(newClass==undefined || newClass=="")newClass=Classe;
		else
		if(!istClass(htmlNode,Classe))newClass+=' '+Classe;	
		htmlNode.className=newClass;
	}			
}
var subClass=function(htmlNode,Classe){
		var aClass,i;
		if(htmlNode!=undefined && htmlNode.className!=undefined){
			aClass=htmlNode.className.split(" ");	
			var newClass="";
			for(i=0;i<aClass.length;i++){
				if(aClass[i]!=Classe){
					if(newClass!="")newClass+=" ";
					newClass+=aClass[i];
					}
			}
			htmlNode.className=newClass;
		}
}
var istClass=function(htmlNode,Classe){
	if(htmlNode.className){
		var i,aClass=htmlNode.className.split(' ');
		for(i=0;i<aClass.length;i++){
				if(aClass[i]==Classe)return true;
		}	
	}		
	return false;
}
var filterJSON=function(s){
	var re=s;
	if(re.indexOf("'")>-1)re=re.split("'").join('"');
	try {re=JSON.parse(re);} 
	catch(e){
		console.log("JSON.parse ERROR:",s,":");
		re={"error":"parseerror"};
		}
	return re;
}
var cButt=function(z,txt,cl,data,click){
	var a=cE(z,"a","b_"+data);
	a.className=cl;
	a.innerHTML=txt;
	a.href="#";
	a.data=data;
	a.addEventListener('click', click);
};
	
var setstat=function(data){
	var n,i,rel=false,led=false,setrel=false,setled=false;
	if(data.portstatus!=undefined){
		rel=data.portstatus.relais;
		led=data.portstatus.led;
		setrel=true;setled=true;		
	}
	if(data.befehl!=undefined){
		if(data.befehl=="OK"){
			for(i=0;i<data.Arguments.length;i++){
				//if(data.Arguments[i].setport==="ON")		{rel=true;setrel=true;}
				//if(data.Arguments[i].setport==="OFF")	{setrel=true;}
				if(data.Arguments[i].setport==="LEDON")	{led=true;setled=true;}
				if(data.Arguments[i].setport==="LEDOFF")	{setled=true;}
			}
		}
	}
	//n=gE("stat_Rls");
	//if(setrel)if(rel===true)subClass(n,"inaktiv");else addClass(n,"inaktiv");
	n=gE("stat_LED");
	if(setled)if(led===true)subClass(n,"inaktiv");else addClass(n,"inaktiv");
}

var ESP8266=function(){//lokal 
	var ESP8266URL="./action?setport=";	
	
	var ini=function(){
		var i,z,b;
		z=gE("actions");
		if(z){
			//b=cE(z,"span","stat_Rls","pout cRelay inaktiv");b.title="Relay";
			b=cE(z,"span","stat_LED","pout cLED inaktiv");b.title="LED";
			b=cE(z,"br");
			for(i=0;i<befehllist.length;i++){
				b=befehllist[i];
				cButt(z,b.n,"butt",b.b,buttclick);
			}
		}
	}
	
	var buttclick=function(e){
		getpostData(ESP8266URL+this.data,fresult);//Daten laden/senden
		e.preventDefault();
	}	
	var fresult=function(data){
		var j=filterJSON(data.responseText);
		console.log(j);//befehl:"ok"
		setstat(j);
	}	
	ini();
}



var SysFunc=function(){
	var datei="./timer.txt";
	var dateisysinfo="./data.json";
	var ziel,timerdata=[];
	var savetimeout=undefined;
	var timerlistreload=undefined;
	var lokdat=undefined;
	
	var wochentag=["Montag","Dienstag","Mittwoch","Donnerstag","Freitag","Samstag","Sonntag"];
	
		
	var anzinputs=0;
	var cI=function(ziel,typ,value,title){//create input
		var label;
		var input=cE(ziel,"input");
		input.type=typ;
		if(typ=="checkbox"){
			input.checked=value;
			input.id="cb"+anzinputs;
			label=cE(ziel,"label");
			label.htmlFor=input.id;
			input.dataLabel=label;
		}	
		else
			input.value=value;
		if(title!=undefined)input.title=title;	
		anzinputs++;
		return input;
	}
	var addBefehle=function(node,dat){
		var i,o;
		for(i=0;i<befehllist.length;i++){
			o=cE(node,"option");
			o.value=befehllist[i].b;
			o.innerHTML=befehllist[i].n;
		}
	}
	
	
	var changetimekorr=function(e){
		var val=this.value*60*60;//in sekunden
		//console.log(">>>",val);
		getpostData(dateisysinfo+'?settimekorr='+val,
			function(d){
				//console.log('reload data',d);
				setTimeout(function(){
					getpostData(dateisysinfo,fresultsysinfo);
					}
				,1000);//1 sec warten, bis intern Zeit gesetzt wurde
			}
		);
		//settimekorr, led=on, led=off
	}
	
	var fresultsysinfo=function(data){
		var ziel=gE('sysinfo'),
			jdat=filterJSON(data.responseText),
			div,node,p,a,s;
		if(ziel){
			ziel.innerHTML="";
						
			div=cE(ziel,"div",undefined,"utctimset");
			div.innerHTML="UTC Zeitunterschied:";
			var val=Math.floor(jdat.datum.timekorr);
			node=cI(div,"number",val,"Zeitunterschied");
			node.addEventListener('change',changetimekorr);
			node.maxlength=2;
			node.size=2;
			if(val==-1 || val==1)
				node=document.createTextNode(" Stunde");
			else
				node=document.createTextNode(" Stunden");
			div.appendChild(node);

			
			lokdat=cE(ziel,"article");
			getlokaldata(jdat);
			
			node=document.getElementsByTagName('h1')[0];
			if(node)
				node.innerHTML=jdat.progversion;
			
			//fstotalBytes,fsusedBytes,fsused,progversion,aktionen
			
		}
	}
	var retlokaldata=function(data){
		jdat=filterJSON(data.responseText);
		getlokaldata(jdat);
	}
	var iftimr;
	var getlokaldata=function(jdat){
		var node;
		if(lokdat!=undefined){
			if(iftimr!=undefined)clearTimeout(iftimr);
			lokdat.innerHTML="";			
			node=cE(lokdat,"p");
			
			if(jdat.error!=undefined){
				console.log("Fehler",typeof jdat,jdat);
				iftimr=setTimeout(function(){
					getpostData(dateisysinfo,retlokaldata);
				},1000*20);//20sec
				return;
			}
			
			var t=jdat.lokalzeit.split(":");
			node.innerHTML="lokaltime: "+t[0]+':'+t[1];
			
			node=cE(lokdat,"p");
			s="";
			if(jdat.datum.day<10)s+="0";
			s+=jdat.datum.day+".";
			if(jdat.datum.month<10)s+="0";
			s+=jdat.datum.month+".";
			node.innerHTML="Datum: "+s+jdat.datum.year+" "+wochentag[jdat.datum.tag];
						
			node=cE(lokdat,"p");
			node.innerHTML="Sommerzeit: "+jdat.datum.summertime;
			
			node=cE(lokdat,"p");
			node.innerHTML="MAC: <span style=\"text-transform: uppercase;\">"+jdat.macadresse+"</span>";
			
			
			setstat(jdat);
			
			iftimr=setTimeout(function(){
				getpostData(dateisysinfo,retlokaldata);
			},1000*10);//10sec
		}
	}
	
	var ini=function(){
		var z2=gE('sysinfo');
		if(z2)getpostData(dateisysinfo,fresultsysinfo);//+'&time='+tim.getTime()
	}
	
	ini();
}




window.addEventListener('load', function (event) {
	var r=new ESP8266();
	var sysfunc=new SysFunc();
});