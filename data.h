
//---------------------------------chartobitmuster------------------------------
const byte zeichentab8[]={//1byte code, 8byte matrix
  32, 0,0,0,0,0,0,0,0, //space
  34, 0,0,0,0,0,0,0,0, //"
  36, 0,0,0,0,0,0,0,0, //$
  39, 0,0,0,0,0,0,0,0, //'
  
  44, 0,0,0,0,0,0,0,0, //,
  46, 0,0,0,0,0,0,0,0, //.
  
  48, 0,0,0,0,0,0,0,0, //0
  49, 0,0,0,0,0,0,0,0, //1
  50, 0,0,0,0,0,0,0,0, //2
  51, 0,0,0,0,0,0,0,0, //3
  52, 0,0,0,0,0,0,0,0, //4
  53, 0,0,0,0,0,0,0,0, //5
  54, 0,0,0,0,0,0,0,0, //6
  55, 0,0,0,0,0,0,0,0, //7
  56, 0,0,0,0,0,0,0,0, //8
  57, 0,0,0,0,0,0,0,0, //9
  
  65, 0,0,0,0,0,0,0,0, //A 
  66, 0,0,0,0,0,0,0,0,  //

  94, 0,0,0,0,0,0,0,0,  //^
  97, 0,0,0,0,0,0,0,0,  //a


  0, 0,0,0,0,0,0,0,0
};


const byte zeichentab16[]={ //2byte code, 8byte matrix
  194,179,  0,0,0,0,0,0,0,0, //³
  194,176,  0,0,0,0,0,0,0,0, //°

  195,132,  0,0,0,0,0,0,0,0, //Ä
  195,150,  0,0,0,0,0,0,0,0, //Ö
  195,156,  0,0,0,0,0,0,0,0, //Ü
  
  195,164,  0,0,0,0,0,0,0,0, //ä
  195,182,  0,0,0,0,0,0,0,0, //ö
  195,188,  0,0,0,0,0,0,0,0, //ü

  195,0, 0,0,0,0,0,0,0,0
};

const byte zeichentab24[]={//3byte code, 8byte matrix
  226,130,172, 0,0,0,0,0,0,0,0, //€
 
  226,0,0, 0,0,0,0,0,0,0,0
};


//---------------------------------WWW------------------------------
//ESP8266

//HTML \r\n \t
const String indexHTM="<!DOCTYPE html><html><head>\r\n"
"<meta charset=\"utf-8\"/>\r\n"
"<meta content=\"width=device-width,initial-scale=0.65,maximum-scale=2,user-scalable=yes\" name=\"viewport\" />\r\n"
"<link rel=\"shortcut icon\" href=\"favicon.ico\">\r\n"
"<link rel=\"STYLESHEET\" type=\"text/css\" href=\"style.css\">\r\n"
"<script type=\"text/javascript\" src=\"system.js\"></script>\r\n"
"</head><body>\r\n"
"<h1>$h1gtag</h1>\r\n"
"<div id=\"actions\"></div>\r\n"
"<div id=\"sysinfo\"></div>\r\n"
"<div id=\"filelist\">\r\n"
"$filelist"
"</div>\r\n"
"<form class=\"upload\" method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">"
"<input type=\"file\" name=\"upload\" required>\r\n"
"<input type=\"submit\" value=\"Upload\" class=\"button\">\n</form>\r\n"
"</body></html>\r\n"
;

/*
const String indexHTM="<!DOCTYPE html><html><head>\r\n"
"<meta charset=\"utf-8\"/>\r\n"
"<link rel=\"shortcut icon\" href=\"favicon.ico\">\r\n"
"<link rel=\"STYLESHEET\" type=\"text/css\" href=\"style.css\">\r\n"
"<script type=\"text/javascript\" src=\"system.js\"></script>\r\n"
"</head><body>\r\n"
"<h1>$h1gtag</h1>\r\n"
"<p class=\"buttons\">\r\n"
"<a href=\"#\" data-OF='{\"action\":\"ON\"}'>ON</a>"
"<a href=\"#\" data-OF='{\"action\":\"OFF\"}'>OFF</a>"
"<a href=\"#\" data-OF='{\"action\":\"LEDON\"}'>Led:on</a>"
"<a href=\"#\" data-OF='{\"action\":\"LEDOFF\"}'>Led:off</a>"
"</p>\r\n"
"<div id=\"timersetting\"></div>\r\n"
"<div id=\"sysinfo\"></div>\r\n"
"<div id=\"filelist\">\r\n"
"$filelist"
"</div>\r\n"
"<form class=\"upload\" method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">"
"<input type=\"file\" name=\"upload\" required>\r\n"
"<input type=\"submit\" value=\"Upload\" class=\"button\">\n</form>\r\n"
"</body></html>\r\n"
;

*/
